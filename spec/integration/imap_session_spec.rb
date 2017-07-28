require File.expand_path('../helper', File.dirname(__FILE__))
require 'socket'
require 'tempfile'

describe 'capability hello matsumotory' do

  before :all do
    @database = Database.new
    @storage = Storage.new

    @database.clear_users
    @database.clear_keys
    @database.insert_user 1, 'test', 'testPassword'

    @mailer = Mailer::IMAP::Session.new '127.0.0.1', '6070'
  end

  after :all do
    @mailer.logout
  end

  context 'check capability' do

    before :all do
      # do nothing
    end

    after :all do
      # do nothing
    end

    it 'get capability matsumotory command' do
      line = @mailer.login 'test', 'testPassword'
      line.should =~ /MRUBY/
    end
  end
end
