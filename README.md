# [WIP] dovecot-mruby-plugin [![Build Status](https://travis-ci.org/matsumotory/dovecot-mruby-plugin.svg?branch=master)](https://travis-ci.org/matsumotory/dovecot-mruby-plugin)

dovecot with mruby is a programmable IMAP/POP server scripting with mruby. Experimet Dovecot Plugin Develop Environment for mruby. [pmilter](https://github.com/matsumotory/pmilter) is a programmable Milter server scripting with mruby.

## Quick Build and Install

```
make setup
make
cp -p dovecot/target/lib/dovecot/lib95_imap_mruby_plugin.so{,1} /path/to/plugin-dir/.
```

## Test

- dovecot download and install, then test dovecot-mruby-plugin in current directory

```
make setup
make test
```

## Configuration

- `conf.d/95-mruby.conf`

```
protocol imap {
  mail_plugins = $mail_plugins imap_mruby
}

plugin {
  mruby_pre_capability = '"pre #{Dovecot::IMAP4.command_name}"'
  mruby_post_capability = '"post #{Dovecot::IMAP4.command_name}"'
}
```

### dovcot start and watching log

- telnet

```
$ telnet 127.0.0.1 6070
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
* OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE IDLE AUTH=PLAIN] Dovecot ready.
1 login test testPassword
1 OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE IDLE SORT SORT=DISPLAY THREAD=REFERENCES THREAD=REFS THREAD=ORDEREDSUBJECT MULTIAPPEND URL-PARTIAL CATENATE UNSELECT CHILDREN NAMESPACE UIDPLUS LIST-EXTENDED I18NLEVEL=1 CONDSTORE QRESYNC ESEARCH ESORT SEARCHRES WITHIN CONTEXT=SEARCH LIST-STATUS BINARY MOVE MRUBY] Logged in
1 capability
* CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE IDLE SORT SORT=DISPLAY THREAD=REFERENCES THREAD=REFS THREAD=ORDEREDSUBJECT MULTIAPPEND URL-PARTIAL CATENATE UNSELECT CHILDREN NAMESPACE UIDPLUS LIST-EXTENDED I18NLEVEL=1 CONDSTORE QRESYNC ESEARCH ESORT SEARCHRES WITHIN CONTEXT=SEARCH LIST-STATUS BINARY MOVE MRUBY
1 OK Capability completed (0.002 + 0.000 + 0.001 secs).
```

- logging

```
Aug 03 11:47:30 imap-login: Info: Login: user=<test>, method=PLAIN, rip=127.0.0.1, lip=127.0.0.1, mpid=19833, secured, session=<GpIH8tdVoL1/AAAB>
Aug 03 11:47:30 imap(test): Debug: Loading modules from directory: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot
Aug 03 11:47:30 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib10_quota_plugin.so
Aug 03 11:47:30 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib15_notify_plugin.so
Aug 03 11:47:30 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib20_fts_plugin.so
Aug 03 11:47:30 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib20_mail_log_plugin.so
Aug 03 11:47:30 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib20_zlib_plugin.so
Aug 03 11:47:30 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib95_imap_mruby_plugin.so
Aug 03 11:47:30 imap(test): Debug: Added userdb setting: plugin/scrambler_plain_password=<hidden>
Aug 03 11:47:30 imap(test): Info: imap_mruby_plugin_init
Aug 03 11:47:30 imap(test): Debug: Effective uid=1000, gid=1000, home=/home/ubuntu/src/dovecot-2.2.31/build/modules/dovecot/home/test
Aug 03 11:47:30 imap(test): Info: imap_mruby_client_created
Aug 03 11:47:34 imap(test): Info: mruby_command_pre
Aug 03 11:47:34 imap(test): Info: mruby_command_run_getenv
Aug 03 11:47:34 imap(test): Info: mruby_pre_capability inline-code: "pre #{Dovecot::IMAP4.command_name}"
Aug 03 11:47:34 imap(test): Info: run mruby at mruby_pre_capability, return value: "pre capability"
Aug 03 11:47:34 imap(test): Info: mruby_command_post
Aug 03 11:47:34 imap(test): Info: mruby_command_run_getenv
Aug 03 11:47:34 imap(test): Info: mruby_post_capability inline-code: "post #{Dovecot::IMAP4.command_name}"
Aug 03 11:47:34 imap(test): Info: run mruby at mruby_post_capability, return value: "post capability"
```

build and test system: special thanks: https://github.com/posteo/scrambler-plugin
