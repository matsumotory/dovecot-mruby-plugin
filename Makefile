PREFIX=/home/ubuntu/src/dovecot-2.2.31/build
DOVECOT_INC=$(PREFIX)/include/dovecot
CFLAGS=-std=gnu99 -fPIC -shared -Wall -I$(DOVECOT_INC) -DHAVE_CONFIG_H
MODULE_NAME=lib95_imap_hello_plugin.so
MODULE_LANAME=lib95_imap_hello_plugin.la

INSTALL_DEST=$(PREFIX)/lib/dovecot/

OBJ=imap-hello-plugin.o

all: lib95_imap_hello_plugin.so

lib95_imap_hello_plugin.so: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:
	rm -rf $(OBJ) $(MODULE_NAME)

install: $(MODULE_NAME)
	mkdir -p $(INSTALL_DEST)
	install -g bin -o root -m 0644 $(MODULE_NAME) $(MODULE_LANAME) $(INSTALL_DEST)

.PHONY: all lib95_imap_hello_plugin.so install clean
