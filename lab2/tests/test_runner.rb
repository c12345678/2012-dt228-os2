#!/usr/bin/env ruby
# vim: set ts=2 sw=2 expandtab:

require 'pty'
require 'expect'

def msg(*args)
  $stderr.print *args
  $stderr.print "\n"
end

def fail(*args)
  $stderr.print "\n"
  $stderr.print "Fail: "
  msg *args
  @passes -= 1
end

def fail_and_exit(*args)
  fail(*args)
  exit 1
end

def current_depth
  ENV['PWD'].split('/').size - 1
end

def relative_path_to(file)
  '../' * current_depth + file
end


TIMEOUT=2

tests = [
  {cmd: '/bin/uname', expected: 'Linux', explanation: "Unexpected output from running the a command"},
  {cmd: '/bin/uname -n', expected: 'kingfisher', explanation: "No support for passing command line arguments"},
  {cmd: 'uname', expected: 'Linux', explanation: "Command in PATH but not executed "},
  {cmd: relative_path_to('bin/uname'), expected: 'Linux', explanation: "Relative path command not executed "},
  {cmd: 'PS1="$ "', expected: '$ ', explanation: "Failed to set a new command prompt into PS1"}
]
@passes=tests.size
@prompt="lsh>> "

#$expect_verbose=1
begin
  tests.each_with_index do |test, i|
    PTY.spawn(ARGV[0]) do |lsh_out, lsh_in, pid|
      lsh_out.expect(/^#{@prompt}$/, TIMEOUT) do |r|
        fail_and_exit "Doesn't seem to have run the shell" if r.nil?
        msg "Test #{i}: " + test[:cmd]
        lsh_in.print test[:cmd] + "\r"
        lsh_out.expect(/#{test[:expected]}/, TIMEOUT) do |r|
          fail test[:explanation] if r.nil?
        end
      end
    end
  end
rescue => e
  case
  when e.is_a?(Errno::ENOENT)
    fail_and_exit "Can't find #{ARGV[0]} binary. Make sure you have run 'make' and are you in the correct folder"
  end
end

$stderr.print "\n#{@passes} out of #{tests.size} tests passed.\n"
