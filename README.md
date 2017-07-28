# [WIP] dovecot-mruby-plugin [![Build Status](https://travis-ci.org/matsumotory/dovecot-mruby-plugin.svg?branch=master)](https://travis-ci.org/matsumotory/dovecot-mruby-plugin)

dovecot with mruby is a programmable IMAP/POP server scripting with mruby. Experimet Dovecot Plugin Develop Environment for mruby. [pmilter](https://github.com/matsumotory/pmilter) is a programmable Milter server scripting with mruby.

## Quick Build

```
make setup
make
```

## Test

```
make setup
make test
```

## Play Ground

```
make run
```

```
$ telnet 127.0.0.1 6070
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
* OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE IDLE AUTH=PLAIN] Dovecot ready.
1 login test testPassword
1 OK [CAPABILITY IMAP4rev1 LITERAL+ SASL-IR LOGIN-REFERRALS ID ENABLE IDLE SORT SORT=DISPLAY THREAD=REFERENCES THREAD=REFS THREAD=ORDEREDSUBJECT MULTIAPPEND URL-PARTIAL CATENATE UNSELECT CHILDREN NAMESPACE UIDPLUS LIST-EXTENDED I18NLEVEL=1 CONDSTORE QRESYNC ESEARCH ESORT SEARCHRES WITHIN CONTEXT=SEARCH LIST-STATUS BINARY MOVE MRUBY] Logged in
1 MRUBY 3*9
1 27 (0.001 + 0.000 secs).
```

build and test system: special thanks: https://github.com/posteo/scrambler-plugin
