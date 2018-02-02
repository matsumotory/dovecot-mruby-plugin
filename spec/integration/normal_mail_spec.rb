require File.expand_path('../helper', File.dirname(__FILE__))
require 'socket'
require 'tempfile'

describe 'capability hello matsumotory' do

  before :all do
    @database = Database.new
    @mailer = Mailer.new
    @storage = Storage.new
    @administrator = Administrator.new 'test'

    @database.clear_users
    @database.clear_keys
    @database.insert_user 1, 'test', 'testPassword'
  end

  after :all do
    @mailer.close
  end

  context 'of a standard mail' do

    before :all do
      @mailer.deliver 'test message one', 'test'
    end

    after :all do
      @storage.clear
    end

    it 'should allow the administrator to access the mail' do
      mails = @administrator.fetch 'testPassword'
      expect(mails.length).to eq 1
      expect(mails[0]).to match /test message one/
    end

    it 'should respond the meta-data and mail' do
      mails = @mailer.receive
      expect(mails.length).to eq 1
      expect(mails[0]).to match /test message one/
    end

  end

end
