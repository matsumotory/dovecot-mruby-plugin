#include "imap-mruby-plugin.h"

#include "mruby.h"
#include "mruby/compile.h"
#include "mruby/string.h"
#include "mruby/variable.h"
#include "mruby/hash.h"
#include "mruby/array.h"

#include "imap-mruby-plugin-init.h"

#define MRUBY_ADD_COMMAND_PRE_HOOK(cmd_ctx, cmd_name)                                                                  \
  if (strcasecmp(cmd_ctx->name, cmd_name) == 0) {                                                                      \
    mruby_command_run_getenv(cmd_ctx, "mruby_pre_" cmd_name);                                                          \
    mruby_command_path_run_getenv(cmd_ctx, "mruby_pre_" cmd_name "_path");                                             \
    return;                                                                                                            \
  }
#define MRUBY_ADD_COMMAND_POST_HOOK(cmd_ctx, cmd_name)                                                                 \
  if (strcasecmp(cmd_ctx->name, cmd_name) == 0) {                                                                      \
    mruby_command_run_getenv(cmd_ctx, "mruby_post_" cmd_name);                                                         \
    mruby_command_path_run_getenv(cmd_ctx, "mruby_post_" cmd_name "_path");                                            \
    return;                                                                                                            \
  }

const char *imap_mruby_plugin_version = DOVECOT_ABI_VERSION;

static MODULE_CONTEXT_DEFINE_INIT(imap_mruby_imap_module, &imap_module_register);

#define IMAP_MRUBY_IMAP_CONTEXT(obj) MODULE_CONTEXT(obj, imap_mruby_imap_module)

static struct module *imap_mruby_module;
static imap_client_created_func_t *next_hook_client_created;
static mrb_state *global_mrb;

void imap_mruby_plugin_init(struct module *module);
void imap_mruby_plugin_deinit(void);

static void imap_mruby_set_state(mrb_state *mrb)
{
  global_mrb = mrb;
}

static mrb_state *imap_mruby_get_state()
{
  return global_mrb;
}

bool cmd_mruby_handler(struct client_command_context *cmd)
{
  struct client *client;
  struct imap_mruby_context *imctx;
  struct RClass *klass;
  const struct imap_arg *args;
  mrb_state *mrb;
  mrb_value v, cmd_block, cmd_hash, cmd_name, cmd_args;
  mrb_sym cmd_hash_sym;

  client = cmd->client;

  /* path to mruby internal */
  imctx = IMAP_MRUBY_IMAP_CONTEXT(client);
  imctx->mruby_ctx->client = client;
  imctx->mruby_ctx->cmd = cmd;
  imctx->mruby_ctx->imctx = imctx;
  imctx->mruby_ctx->cmd_done = FALSE;

  mrb = imctx->mrb;
  mrb->ud = imctx->mruby_ctx;

  klass = mrb_class_get_under(mrb, mrb_class_get(mrb, "Dovecot"), "IMAP");

  cmd_hash_sym = mrb_intern_lit(mrb, IMAP_MRUBY_COMMAND_REG_ID);
  cmd_hash = mrb_mod_cv_get(mrb, klass, cmd_hash_sym);
  cmd_name = mrb_funcall(mrb, mrb_str_new_cstr(mrb, cmd->name), "upcase", 0, NULL);
  cmd_block = mrb_hash_get(mrb, cmd_hash, cmd_name);

  if (!client_read_args(cmd, 0, 0, &args)) {
    client_send_command_error(cmd, "Unexpected arguments.");
    return TRUE;
  }

  cmd_args = mrb_ary_new(mrb);
  for (; !IMAP_ARG_IS_EOL(args); args++) {
    const char *str;
    if (!imap_arg_get_atom(args, &str)) {
      client_send_command_error(cmd, "Invalid arguments.");
      return TRUE;
    }
    mrb_ary_push(mrb, cmd_args, mrb_str_new_cstr(mrb, str));
  }

  if (mrb_type(cmd_block) == MRB_TT_PROC) {
    v = mrb_yield_argv(mrb, cmd_block, 1, &cmd_args);
  } else {
    i_error("cmd_block should be proc object: %s in %s with %s", mrb_str_to_cstr(mrb, mrb_inspect(mrb, cmd_block)),
            mrb_str_to_cstr(mrb, mrb_inspect(mrb, cmd_hash)), mrb_str_to_cstr(mrb, mrb_inspect(mrb, cmd_name)));
    client_send_command_error(cmd, "Invalid command.");
    mrb->exc = 0;
    return TRUE;
  }

  if (mrb->exc != 0 && (mrb_nil_p(v) || mrb_undef_p(v))) {
    v = mrb_obj_value(mrb->exc);
    i_error("mruby handler raise: %s", mrb_str_to_cstr(mrb, mrb_inspect(mrb, v)));
    client_send_tagline(cmd, t_strconcat("NO ", cmd->name, " is invalid.", NULL));
    mrb->exc = 0;
    return TRUE;
  }

  if (!imctx->mruby_ctx->cmd_done) {
    client_send_tagline(cmd, t_strconcat("OK ", cmd->name, " completed.", NULL));
  }
  return TRUE;
}

static void imap_mruby_client_created(struct client **clientp)
{
  struct client *client = *clientp;
  struct imap_mruby_context *imctx;

  if (mail_user_is_plugin_loaded(client->user, imap_mruby_module)) {
    imctx = p_new(client->pool, struct imap_mruby_context, 1);
    imctx->mrb = imap_mruby_get_state();
    imctx->mruby_ctx = p_new(client->pool, imap_mruby_internal_context, 1);
    MODULE_CONTEXT_SET(client, imap_mruby_imap_module, imctx);
  }

  if (next_hook_client_created != NULL) {
    next_hook_client_created(clientp);
  }
}

static void mruby_command_path_run_getenv(struct client_command_context *cmd, const char *env)
{
  struct client *client;
  struct imap_mruby_context *imctx;
  mrb_state *mrb;
  mrbc_context *c;
  mrb_value v;
  const char *path;
  FILE *fp;

  client = cmd->client;
  path = mail_user_plugin_getenv(client->user, env);

  if (path == NULL) {
    i_info("mruby %s hook declined", env);
    return;
  }

  i_info("%s code-path: %s", env, path);

  imctx = IMAP_MRUBY_IMAP_CONTEXT(client);
  imctx->mruby_ctx->client = client;
  imctx->mruby_ctx->cmd = cmd;
  imctx->mruby_ctx->imctx = imctx;

  mrb = imctx->mrb;
  mrb->ud = imctx->mruby_ctx;
  c = mrbc_context_new(mrb);
  mrbc_filename(mrb, c, path);

  if ((fp = fopen(path, "r")) == NULL) {
    i_info("open %s failed", path);
    return;
  }

  v = mrb_load_file_cxt(mrb, fp, c);
  mrbc_context_free(mrb, c);

  if (mrb->exc != 0 && (mrb_nil_p(v) || mrb_undef_p(v))) {
    v = mrb_obj_value(mrb->exc);
  }

  fclose(fp);
  mrb->exc = 0;
  i_info("run mruby file at %s, return value: %s", env, mrb_str_to_cstr(mrb, mrb_inspect(mrb, v)));
}

static void mruby_command_run_getenv(struct client_command_context *cmd, const char *env)
{
  struct client *client;
  struct imap_mruby_context *imctx;
  mrb_state *mrb;
  mrbc_context *c;
  mrb_value v;
  const char *code_str;

  client = cmd->client;
  code_str = mail_user_plugin_getenv(client->user, env);

  imctx = IMAP_MRUBY_IMAP_CONTEXT(client);
  mrb = imctx->mrb;
  c = mrbc_context_new(mrb);

  if (code_str == NULL) {
    i_info("mruby %s hook declined", env);
    return;
  }

  i_info("%s inline-code: %s", env, code_str);

  imctx->mruby_ctx->client = client;
  imctx->mruby_ctx->cmd = cmd;
  imctx->mruby_ctx->imctx = imctx;

  mrb->ud = imctx->mruby_ctx;
  v = mrb_load_string_cxt(mrb, code_str, c);
  mrbc_context_free(mrb, c);

  if (mrb->exc != 0 && (mrb_nil_p(v) || mrb_undef_p(v))) {
    v = mrb_obj_value(mrb->exc);
  }

  mrb->exc = 0;
  i_info("run mruby at %s, return value: %s", env, mrb_str_to_cstr(mrb, mrb_inspect(mrb, v)));
}

static void mruby_command_init_path_run(mrb_state *mrb, const char *path)
{
  mrbc_context *c;
  mrb_value v;
  FILE *fp;

  if (path == NULL) {
    i_info("mruby_init hook declined");
    return;
  }

  i_info("code-path: %s", path);

  c = mrbc_context_new(mrb);
  mrbc_filename(mrb, c, path);

  if ((fp = fopen(path, "r")) == NULL) {
    i_info("open %s failed", path);
    return;
  }

  v = mrb_load_file_cxt(mrb, fp, c);
  mrbc_context_free(mrb, c);

  if (mrb->exc != 0 && (mrb_nil_p(v) || mrb_undef_p(v))) {
    v = mrb_obj_value(mrb->exc);
  }

  fclose(fp);
  mrb->exc = 0;
  i_info("run mruby file at mruby_init, return value: %s", mrb_str_to_cstr(mrb, mrb_inspect(mrb, v)));
}

static void mruby_command_pre(struct client_command_context *cmd)
{
  //* mailbox management hook */
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "select");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "examine");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "create");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "delete");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "rename");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "subscribe");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "unsubscribe");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "list");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "rlist");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "lsub");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "rlsub");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "status");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "check");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "close");

  /* mail control hook */
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "append");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "expunge");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "search");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "fetch");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "store");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "copy");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "uid");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "noop");

  /* state hook */
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "capability");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "idle");

  /* etc hook */
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "namespace");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "getquota");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "setquota");
  MRUBY_ADD_COMMAND_PRE_HOOK(cmd, "getquotaroot");
}

static void mruby_command_post(struct client_command_context *cmd)
{
  //* mailbox management hook */
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "select");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "examine");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "create");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "delete");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "rename");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "subscribe");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "unsubscribe");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "list");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "rlist");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "lsub");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "rlsub");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "status");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "check");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "close");

  /* mail control hook */
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "append");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "expunge");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "search");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "fetch");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "store");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "copy");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "uid");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "noop");

  /* state hook */
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "capability");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "idle");

  /* etc hook */
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "namespace");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "getquota");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "setquota");
  MRUBY_ADD_COMMAND_POST_HOOK(cmd, "getquotaroot");
}

void imap_mruby_plugin_init(struct module *module)
{
  mrb_state *mrb;

  mrb = mrb_open();
  if (mrb == NULL) {
    i_error("mrb_open failed");
  } else {
    imap_mruby_class_init(mrb);
  }

  imap_mruby_set_state(mrb);

  /* init handler from ENV */
  mruby_command_init_path_run(mrb, getenv("DOVECOT_MRUBY_INIT_PATH"));

  //* callback function each command */
  command_hook_register(mruby_command_pre, mruby_command_post);

  imap_mruby_module = module;
  next_hook_client_created = imap_client_created_hook_set(imap_mruby_client_created);
}

void imap_mruby_plugin_deinit(void)
{
  mrb_state *mrb;

  command_hook_unregister(mruby_command_pre, mruby_command_post);
  imap_client_created_hook_set(next_hook_client_created);

  mrb = imap_mruby_get_state();
  if (mrb) {
    mrb_close(mrb);
  }
}

const char imap_mruby_plugin_binary_dependency[] = "imap";
