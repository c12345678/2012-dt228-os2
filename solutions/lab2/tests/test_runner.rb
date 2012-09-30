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
  @fails += 1
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
  {tc: 'Run a command with no arguments', cmd: '/bin/uname', expected: 'Linux', explanation: "Unexpected output from running the a command"},
  {tc: 'Check if fork() is implemented', cmd: '/bin/ps', expected: 'lsh', explanation: "Command not executed in fork()"},
  {tc: 'Run a command with a single argument', cmd: '/bin/uname -n', expected: 'kingfisher', explanation: "No support for passing command line arguments (1 arg)"},
  {tc: 'Run a command with three arguments', cmd: '/usr/bin/expr 1 + 1', expected: '^2', explanation: "No support for passing command line arguments (3 args)"},
  {tc: 'Run a commmand with multiple arguments', cmd: '/usr/bin/expr 5 * 6 + 7 / 2 - 8 * 3', expected: '^9', explanation: "No support for passing command line arguments (11 args)"},
  {tc: 'Check if PATH lookup is implemented for a command that exists', cmd: 'uname', expected: 'Linux', explanation: "Command in PATH but not executed "},
  {tc: 'Check how PATH lookup is implemented for a command that does not exist', cmd: 'xyz', expected: 'No such file or directory', explanation: "Command not in PATH but expected error not thrown"},
  {tc: 'Check if relative path commands will run', cmd: relative_path_to('bin/uname'), expected: 'Linux', explanation: "Relative path command not executed "},
  {tc: 'Check if environment variables are implemented', cmd: 'PS1="% "', expected: '% ', explanation: "Failed to set a new command prompt into PS1"}
]
@passes=0
@fails=0
@run=0
@prompt="lsh>> "

#$expect_verbose=1
begin
  tests.each_with_index do |test, i|
    @run = i + 1
    PTY.spawn(ARGV[0]) do |lsh_out, lsh_in, pid|
      lsh_out.expect(/^#{@prompt}$/, TIMEOUT) do |r|
        fail_and_exit "Doesn't seem to have run the shell" if r.nil?
        msg "unit-test-#{i}: " + test[:tc] + "\t(" + test[:cmd] + ')'
        lsh_in.print test[:cmd] + "\r"
        lsh_out.expect(/#{test[:expected]}/, TIMEOUT) do |r|
          if r.nil?
            fail test[:explanation]
          else
            @passes += 1
          end
        end
      end
    end
  end
rescue => e
  case
  when e.is_a?(Errno::ENOENT)
    fail_and_exit "Can't find #{ARGV[0]} binary. Run these tests with 'make tests' from within the lab source folder"
  end
end

$stderr.puts "\n#{@passes} out of #{tests.size} tests passed of #{@run} test cases run. "
if @run < tests.size
  $stderr.puts "#{tests.size - @run} test cases could not run. Fix failing unit-test-#{@run - 1} to proceed\n"
end
