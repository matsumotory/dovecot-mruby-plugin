#ifndef HELLO_PLUGIN_H
#define HELLO_PLUGIN_H

extern const char imap_hello_plugin_binary_dependency[];

void imap_hello_plugin_init(struct module *module);
void imap_hello_plugin_deinit(void);

#endif
