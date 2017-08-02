#include "dovecot/config.h"
#include "dovecot/lib.h"
#include "dovecot/str.h"
#include "dovecot/imap-client.h"
#include "dovecot/imap-common.h"
#include "dovecot/module-context.h"

#include "imap-mruby-plugin.h"

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/string.h"

#define IMAP_MRUBY_IMAP_CONTEXT(obj) MODULE_CONTEXT(obj, imap_mruby_imap_module)

struct imap_mruby_context {
  union imap_module_context module_ctx;
  mrb_state *mrb;
};

const char *imap_mruby_plugin_version = DOVECOT_ABI_VERSION;

static struct module *imap_mruby_module;
static imap_client_created_func_t *next_hook_client_created;

static MODULE_CONTEXT_DEFINE_INIT(imap_mruby_imap_module, &imap_module_register);

static bool cmd_mruby(struct client_command_context *cmd)
{
  mrb_value v;
  const struct imap_arg *args;
  const char *code;

  struct client *client = cmd->client;
  struct imap_mruby_context *imctx = IMAP_MRUBY_IMAP_CONTEXT(client);

  mrb_state *mrb = imctx->mrb;
  mrbc_context *c = mrbc_context_new(mrb);

  if (mrb == NULL) {
    i_error("mrb_open() failed.");
    return FALSE;
  }

  if (!client_read_args(cmd, 0, 0, &args)) {
    return FALSE;
  }

  if (!imap_arg_get_atom(args, &code) || !IMAP_ARG_IS_EOL(&args[1])) {
    client_send_command_error(cmd, "Invalid arguments.");
    return TRUE;
  }

  v = mrb_load_string_cxt(mrb, code, c);
  mrbc_context_free(mrb, c);

  if (mrb->exc != 0 && (mrb_nil_p(v) || mrb_undef_p(v))) {
    v = mrb_obj_value(mrb->exc);
  }

  client_send_tagline(cmd, mrb_str_to_cstr(mrb, mrb_inspect(mrb, v)));
  return TRUE;
}

static void imap_mruby_client_created(struct client **clientp)
{
  struct client *client = *clientp;
  struct imap_mruby_context *imctx;

  if (mail_user_is_plugin_loaded(client->user, imap_mruby_module)) {
    str_append(client->capability_string, " MRUBY");
    imctx = p_new(client->pool, struct imap_mruby_context, 1);
    imctx->mrb = mrb_open();
    MODULE_CONTEXT_SET(client, imap_mruby_imap_module, imctx);
  }

  if (next_hook_client_created != NULL) {
    next_hook_client_created(clientp);
  }
}

static void mruby_command_run_getenv(struct client_command_context *cmd, const char *env)
{
  struct client *client = cmd->client;
  struct imap_mruby_context *imctx = IMAP_MRUBY_IMAP_CONTEXT(client);
  mrb_state *mrb = imctx->mrb;
  mrbc_context *c = mrbc_context_new(mrb);
  mrb_value v;
  const char *code_str;

  code_str = mail_user_plugin_getenv(cmd->client->user, env);

  if (code_str == NULL) {
    i_info("mruby %s hook declined", env);
    return;
  }

  i_info("%s inline-code: %s", env, code_str);

  v = mrb_load_string_cxt(mrb, code_str, c);
  mrbc_context_free(mrb, c);

  if (mrb->exc != 0 && (mrb_nil_p(v) || mrb_undef_p(v))) {
    v = mrb_obj_value(mrb->exc);
  }

  i_info("run mruby at %s, return value: %s", env, mrb_str_to_cstr(mrb, mrb_inspect(mrb, v)));
}

static void mruby_command_pre(struct client_command_context *cmd)
{
  if (strcasecmp(cmd->name, "CAPABILITY") == 0) {
    mruby_command_run_getenv(cmd, "mruby_pre_capability");
  }
}

static void mruby_command_post(struct client_command_context *cmd)
{
  if (strcasecmp(cmd->name, "CAPABILITY") == 0) {
    mruby_command_run_getenv(cmd, "mruby_post_capability");
  }
}

void imap_mruby_plugin_init(struct module *module)
{
  i_info("mruby_plugin_init");

  /* add MRUBY command */
  command_register("MRUBY", cmd_mruby, 0);

  //* callback function each command */
  command_hook_register(mruby_command_pre, mruby_command_post);

  imap_mruby_module = module;
  next_hook_client_created = imap_client_created_hook_set(imap_mruby_client_created);
}

void imap_mruby_plugin_deinit(void)
{
  command_unregister("MRUBY");
  command_hook_unregister(mruby_command_pre, mruby_command_post);
  imap_client_created_hook_set(next_hook_client_created);
}

const char imap_mruby_plugin_binary_dependency[] = "imap";
