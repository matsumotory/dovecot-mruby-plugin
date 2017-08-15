# dovecot-mruby-plugin [![Build Status](https://travis-ci.org/matsumotory/dovecot-mruby-plugin.svg?branch=master)](https://travis-ci.org/matsumotory/dovecot-mruby-plugin)

dovecot included dovecot-mruby-plugin is a programmable IMAP/POP server scripting with mruby like [ngx_mruby](https://github.com/matsumotory/ngx_mruby) for nginx, [mod_mruby](https://github.com/matsumotory/mod_mruby) for apache httpd and [trusterd](https://github.com/matsumotory/trusterd) for HTTP/2 mruby Web server. Also [pmilter](https://github.com/matsumotory/pmilter) is a programmable Milter server scripting with mruby for SMTP server. 

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

## IMAP command register using mruby

- /path/to/command_register.rb

```ruby
%w(

matsumotory
test

).each do |cmd|
  Dovecot::IMAP.command_register(cmd) do |args|
    Dovecot::IMAP.send_line "Hi, #{Dovecot::IMAP.username}"
    if cmd == Dovecot::IMAP.username
      Dovecot::IMAP.send_line "You are me."
    else
      Dovecot::IMAP.send_line "I am #{cmd}.  Not you with #{args}"
    end
  end
end
```

### start dovecot with `DOVECOT_MRUBY_INIT_PATH` env

- dovecot.conf

```
import_environment = DOVECOT_MRUBY_INIT_PATH
```

- start dovecot

```bash
DOVECOT_MRUBY_INIT_PATH=/path/to/command_register.rb ./dovecot/target/sbin/dovecot \
	-c ./dovecot/configuration/dovecot.conf
```

- telnet example

```
$ telnet 127.0.0.1 6070
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
* OK [CAPABILITY ...] Dovecot ready.

1 login test testPassword
1 OK [CAPABILITY ...] Logged in

1 matsumotory hoge 1
* matsumotory Hi, test
* matsumotory I am matsumotory.  Not you with ["hoge", "1"]
1 OK matsumotory completed (0.001 + 0.000 secs).

1 test 
* test Hi, test
* test You are me.
1 OK test completed (0.001 + 0.000 secs).

1 matsumotory 
* matsumotory Hi, test
* matsumotory I am matsumotory.  Not you with []
1 OK matsumotory completed (0.001 + 0.000 secs).

1 logout
* BYE Logging out
1 OK Logout completed (0.001 + 0.000 secs).

Connection closed by foreign host.
```

## Pre or Post command hook using mruby

- `conf.d/95-mruby.conf`

callback Ruby scripts on pre and post each IMAP commands.

```
protocol imap {
  mail_plugins = $mail_plugins imap_mruby
}

plugin {
  #
  # mruby_pre_${command name} = '${Ruby code}'
  # mruby_post_${command name} = '${Ruby code}'
  # mruby_pre_${command name}_path = /path/to/pre_code.rb
  # mruby_post_${command name}_path = /path/to/post_code.rb
  #
  # support the following commands
  #
  # select examine create delete rename subscribe unsubscribe list rlist lsub rlsub
  # status check close append expunge search fetch store copy uid noop capability idle
  # namespace getquota setquota getquotaroot

  mruby_pre_capability = '"pre #{Dovecot::IMAP.command_name}"'
  mruby_post_capability = '"post #{Dovecot::IMAP.command_name}"'

  # mruby_pre_capability_path = /tmp/a.rb
  # mruby_post_capability_path = /tmp/a.rb
}
```

### dovcot start and watching log

- telnet

```
$ telnet 127.0.0.1 6070
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
* OK [CAPABILITY ...] Dovecot ready.

1 login test testPassword
1 OK [CAPABILITY ...] Logged in

1 capability
* CAPABILITY IMAP4rev1 ...
1 OK Capability completed (0.002 + 0.000 + 0.001 secs).
```

- logging

```
Aug 15 10:14:39 imap-login: Info: Login: user=<test>, method=PLAIN, rip=127.0.0.1, lip=127.0.0.1, mpid=29724, secured, session=<JZILDMhW1L5/AAAB>
Aug 15 10:14:39 imap(test): Debug: Loading modules from directory: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot
Aug 15 10:14:39 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib10_quota_plugin.so
Aug 15 10:14:39 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib15_notify_plugin.so
Aug 15 10:14:39 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib20_fts_plugin.so
Aug 15 10:14:39 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib20_mail_log_plugin.so
Aug 15 10:14:39 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib20_zlib_plugin.so
Aug 15 10:14:39 imap(test): Debug: Module loaded: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/target/lib/dovecot/lib95_imap_mruby_plugin.so
Aug 15 10:14:39 imap(test): Debug: Added userdb setting: plugin/scrambler_plain_password=<hidden>
Aug 15 10:14:39 imap(test): Info: code-path: /home/ubuntu/DEV/dovecot-mruby-plugin/dovecot/configuration/mruby_handler/command_register.rb
Aug 15 10:14:39 imap(test): Info: run mruby file at mruby_init, return value: ["matsumotory", "test"]
Aug 15 10:14:39 imap(test): Debug: Effective uid=1000, gid=1000, home=/home/ubuntu/src/dovecot-2.2.31/build/modules/dovecot/home/test
Aug 15 10:14:39 imap(test): Debug: quota: No quota setting - plugin disabled
Aug 15 10:14:39 imap(test): Debug: Namespace inbox: type=private, prefix=, sep=/, inbox=yes, hidden=no, list=yes, subscriptions=yes location=mdbox:~/mail
Aug 15 10:14:39 imap(test): Debug: fs: root=/home/ubuntu/src/dovecot-2.2.31/build/modules/dovecot/home/test/mail, index=, indexpvt=, control=, inbox=, alt=
Aug 15 10:14:39 imap(test): Debug: fts: No fts setting - plugin disabled
Aug 15 10:14:41 imap(test): Info: mruby_pre_capability inline-code: "pre #{Dovecot::IMAP.command_name} #{Dovecot::IMAP.username}"
Aug 15 10:14:41 imap(test): Info: run mruby at mruby_pre_capability, return value: "pre capability test"
Aug 15 10:14:41 imap(test): Info: mruby mruby_pre_capability_path hook declined
Aug 15 10:14:41 imap(test): Info: mruby_post_capability inline-code: "post #{Dovecot::IMAP.command_name} #{Dovecot::IMAP.username}"
Aug 15 10:14:41 imap(test): Info: run mruby at mruby_post_capability, return value: "post capability test"
Aug 15 10:14:41 imap(test): Info: mruby mruby_post_capability_path hook declined
Aug 15 10:14:43 imap(test): Info: Logged out in=22 out=778
```

## User Case

Control cpu usage of each command with any user.

### limit cpu 30% using mruby-cgroup

- pre.rb

```ruby
rate = Cgroup::CPU.new "test"

# limit cpu 30% usage
rate.cfs_quota_us = 30000

rate.create

if Dovecot::IMAP.username == "test" && Dovecot::IMAP.command_name == "LIST"
  rate.attach
end
```

- post.rb

```ruby
rate = Cgroup::CPU.new "test"

if Dovecot::IMAP.username == "test" && Dovecot::IMAP.command_name == "LIST"
  rate.detach
end
```

- 95-mruby.conf

```
protocol imap {
  mail_plugins = $mail_plugins imap_mruby
}

plugin {
  mruby_pre_list_path = /path/to/pre.rb
  mruby_post_list_path = /path/to/post.rb
}
```

- connect dovecot and run LIST with CPU 100% to 30% limit

```
29906 ubuntu    20   0   87640   9508   7532 R  29.9  0.1   0:04.46 imap 
```

build and test system: special thanks: https://github.com/posteo/scrambler-plugin
