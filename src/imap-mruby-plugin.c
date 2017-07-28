#include "dovecot/config.h"
#include "dovecot/lib.h"
#include "dovecot/str.h"
#include "dovecot/imap-client.h"
#include "dovecot/imap-common.h"
#include "dovecot/module-context.h"

#include "imap-mruby-plugin.h"

const char *imap_hello_plugin_version = DOVECOT_ABI_VERSION;

static struct module *imap_hello_module;
static imap_client_created_func_t *next_hook_client_created;

static bool cmd_hello(struct client_command_context *cmd)
{
  client_send_command_error(cmd, "hello matsumotory");
  return FALSE;
}

static void imap_hello_client_created(struct client **client)
{
  if (mail_user_is_plugin_loaded((*client)->user, imap_hello_module)) {
    str_append((*client)->capability_string, " MATSUMOTORY");
  }

  if (next_hook_client_created != NULL) {
    next_hook_client_created(client);
  }
}

void imap_hello_plugin_init(struct module *module)
{
  i_info("hello_plugin_init");
  command_register("MATSUMOTORY", cmd_hello, 0);
  imap_hello_module = module;
  next_hook_client_created = imap_client_created_hook_set(imap_hello_client_created);
}

void imap_hello_plugin_deinit(void)
{
  command_unregister("MATSUMOTORY");
  imap_client_created_hook_set(next_hook_client_created);
}

const char imap_hello_plugin_binary_dependency[] = "imap";
