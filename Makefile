# thanks https://github.com/posteo/scrambler-plugin
DOVECOT_VERSION=2.2.31
DOVECOT_DIR=$(abspath dovecot)
DOVECOT_SOURCE_URL=https://www.dovecot.org/releases/2.2/dovecot-$(DOVECOT_VERSION).tar.gz
DOVECOT_SOURCE_FILE=$(DOVECOT_DIR)/dovecot-$(DOVECOT_VERSION).tar.gz
DOVECOT_SOURCE_DIR=$(DOVECOT_DIR)/source
DOVECOT_TARGET_DIR=$(DOVECOT_DIR)/target
DOVECOT_INCLUDE_DIR=dovecot/target/include

SOURCE_DIR=src
TARGET_LIB_SO=dovecot/target/lib/dovecot/lib95_imap_mruby_plugin.so

# for mruby
MRUBY_ROOT=$(SOURCE_DIR)/mruby
MRUBY_MAK_FILE := $(MRUBY_ROOT)/build/host/lib/libmruby.flags.mak
ifeq ($(wildcard $(MRUBY_MAK_FILE)),)
  MRUBY_CFLAGS =
  MRUBY_LDFLAGS =
  MRUBY_LIBS =
  MRUBY_LDFLAGS_BEFORE_LIBS =
else
  include $(MRUBY_MAK_FILE)
endif

C_FILES=$(shell ls $(SOURCE_DIR)/*.c)
H_FILES=$(C_FILES:.c=.h)
O_FILES=$(C_FILES:.c=.o)

CC=gcc
CFLAGS=-std=gnu99 \
	-Wall -W -Wmissing-prototypes -Wmissing-declarations -Wpointer-arith -Wchar-subscripts -Wformat=2 \
	-Wbad-function-cast -fno-builtin-strftime -Wstrict-aliasing=2 -Wl,-z,relro,-z,now \
	-fPIC -fstack-check -ftrapv -DPIC -D_FORTIFY_SOURCE=2 -DHAVE_CONFIG_H \
	-I$(DOVECOT_INCLUDE_DIR) $(MRUBY_CFLAGS)
LDFLAGS=-gs -shared -rdynamic -Wl,-soname,lib95_imap_mruby_plugin.so.1 $(MRUBY_LDFLAGS)
LIBS=$(MRUBY_LDFLAGS_BEFORE_LIBS) $(MRUBY_LIBS)

ifeq ($(DEBUG), 1)
	CFLAGS+=-DDEBUG_STREAMS -g
endif

all: $(TARGET_LIB_SO)

$(SOURCE_DIR)/%.o: %.c $(H_FILES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET_LIB_SO): $(O_FILES)
	mkdir -p $(shell dirname $(TARGET_LIB_SO))
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) 


# compile mruby
mruby:
	test -f $(DOVECOT_TARGET_DIR)/lib/libmruby.a || (cd $(MRUBY_ROOT) && \
                MRUBY_CONFIG=../build_config.rb make && cp build/host/lib/libmruby.a $(DOVECOT_TARGET_DIR)/lib/. && \
                cp -r include/* $(DOVECOT_TARGET_DIR)/include/.)

mruby-clean:
	rm -f $(DOVECOT_TARGET_DIR)/lib/libmruby.a
	cd $(MRUBY_ROOT) && make clean

dovecot-clean:
	cd $(DOVECOT_SOURCE_DIR) && make clean

dovecot-download:
	-test ! -f $(DOVECOT_SOURCE_FILE) && curl -o $(DOVECOT_SOURCE_FILE) $(DOVECOT_SOURCE_URL)

dovecot-extract: dovecot-download
	-test ! -d $(DOVECOT_SOURCE_DIR) && \
		(tar xzf $(DOVECOT_SOURCE_FILE) -C $(DOVECOT_DIR) && mv $(DOVECOT_DIR)/dovecot-$(DOVECOT_VERSION) $(DOVECOT_SOURCE_DIR))

dovecot-configure: dovecot-extract
	cd $(DOVECOT_SOURCE_DIR) && ./configure --prefix $(DOVECOT_TARGET_DIR) --with-sqlite --enable-silent-rules

dovecot-compile: dovecot-configure
	cd $(DOVECOT_SOURCE_DIR) && make

dovecot-install: dovecot-compile
	cd $(DOVECOT_SOURCE_DIR) && make install

dovecot-run:
	cd $(DOVECOT_DIR) && ./target/sbin/dovecot -c ./configuration/dovecot.conf

clean:
	rm -f $(O_FILES) $(TARGET_LIB_SO)

setup: dovecot-install
	bundle install --path vendor/bundle

test: $(TARGET_LIB_SO)
	bundle exec rake spec:integration

test-focus: $(TARGET_LIB_SO)
	bundle exec rake spec:integration:focus

log:
	tail -f dovecot/log/dovecot.log

clobber: clean dovecot-clean mruby-clean
	rm -rf $(DOVECOT_SOURCE_FILE) $(DOVECOT_SOURCE_DIR) $(DOVECOT_TARGET_DIR)

start: dovecot-run

restart: 
	killall dovecot
	sleep 1
	cd $(DOVECOT_DIR) && ./target/sbin/dovecot -c ./configuration/dovecot.conf

.PHONY: clean setup test log
