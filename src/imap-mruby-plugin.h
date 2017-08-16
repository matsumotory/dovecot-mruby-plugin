#ifndef HELLO_PLUGIN_H
#define HELLO_PLUGIN_H

#include "dovecot/config.h"
#include "dovecot/lib.h"
#include "dovecot/str.h"
#include "dovecot/imap-client.h"
#include "dovecot/imap-common.h"
#include "dovecot/module-context.h"

#include "mruby.h"

#define IMAP_MRUBY_COMMAND_REG_ID "imap_mruby_command_register"

extern const char imap_hello_plugin_binary_dependency[];

typedef struct imap_mruby_internal_context {
  struct client *client;
  struct client_command_context *cmd;
  struct imap_mruby_context *imctx;
  bool cmd_done;
} imap_mruby_internal_context;

struct imap_mruby_context {
  union imap_module_context module_ctx;
  mrb_state *mrb;
  imap_mruby_internal_context *mruby_ctx;
};

void imap_hello_plugin_init(struct module *module);
void imap_hello_plugin_deinit(void);

bool cmd_mruby_handler(struct client_command_context *cmd);
bool cmd_mruby_alias_handler(struct client_command_context *cmd);

#endif
