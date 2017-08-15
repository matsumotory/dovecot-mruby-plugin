# 
# DOVECOT_MRUBY_INIT_PATH=./dovecot/configuration/mruby_handler/command_register.rb \
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

Dovecot::IMAP.command_register("CAP") do |args|
  Dovecot::IMAP.send_line "execute CAPABILITY commands"
  Dovecot::IMAP.capability
end
