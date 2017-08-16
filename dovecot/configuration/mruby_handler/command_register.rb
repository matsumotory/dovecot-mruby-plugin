# 
# DOVECOT_MRUBY_INIT_PATH=`pwd`/dovecot/configuration/mruby_handler/command_register.rb \
#           ./dovecot/target/sbin/dovecot -c ./dovecot/configuration/dovecot.conf
#

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

# alias command_register
Dovecot::IMAP.register_command("HOGE") do |args|
  Dovecot::IMAP.send_line "register hoge command"
end

Dovecot::IMAP.alias_command_register("CAP") do
  Dovecot::IMAP.send_line "alias CAPABILITY commands"
  Dovecot::IMAP.capability
end

# alias alias_command_register
Dovecot::IMAP.register_alias("LIST_ALIAS") do
  Dovecot::IMAP.send_line "alias LIST commands"
  Dovecot::IMAP.list
end
