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
