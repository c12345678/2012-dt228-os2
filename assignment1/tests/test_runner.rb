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

def path_for(cmd)
  %x{which "#{cmd}"}.chomp
end


TIMEOUT=2

@tests = [
  {tc: 'Run a command with no arguments', depends: :EXTERNAL, cmd: path_for('uname'), expected: %x{uname}.chomp, explanation: "Unexpected output from running the a command"},
  {tc: 'Check if fork() is implemented', depends: :EXTERNAL, cmd: path_for('ps'), expected: 'lsh', explanation: "Command not executed in fork()"},
  {tc: 'Run a command with a single argument', depends: :EXTERNAL, cmd: path_for('uname') + ' -n', expected: %x{uname -n}.chomp, explanation: "No support for passing command line arguments (1 arg)"},
  {tc: 'Run a command with three arguments', depends: :EXTERNAL, cmd: path_for('expr') + ' 1 + 1', expected: '^2', explanation: "No support for passing command line arguments (3 args)"},
  {tc: 'Run a commmand with multiple arguments', depends: :EXTERNAL, cmd: path_for('expr') + ' 5 * 6 + 7 / 2 - 8 * 3', expected: '^9', explanation: "No support for passing command line arguments (11 args)"},
  {tc: 'Check if PATH lookup is implemented for a command that exists', depends: :EXTERNAL, cmd: path_for('uname'), expected: %x{uname}.chomp, explanation: "Command in PATH but not executed "},
  {tc: 'Check how PATH lookup is implemented for a command that does not exist', depends: :EXTERNAL, cmd: 'xyz', expected: 'No such file or directory', explanation: "Command not in PATH but expected error not thrown"},
  {tc: 'Check if relative path commands will run', depends: :EXTERNAL, cmd: relative_path_to(path_for('uname')), expected: %x{uname}.chomp, explanation: "Relative path command not executed "},
  {tc: 'Check if environment variables are implemented', depends: :USERVARS, cmd: 'PS1="% "', expected: '% ', explanation: "Failed to set a new command prompt into PS1"}
]

def valid_tests
  return @valid_tests if @valid_tests
  features=%x(make --silent listfeatures 2>/dev/null).split(/-DLSH_ENABLE_/).map(&:strip).map(&:to_sym)[1..-1]
  @valid_tests = @tests.select { |t| features.include?(t[:depends]) }
end

@passes=0
@fails=0
@run=0
@prompt=%x(make --silent showprompt 2>/dev/null).chomp
@prompt="lsh>> " if @prompt == ""

#$expect_verbose=1
begin
  valid_tests.each_with_index do |test, i|
    @run = i + 1
    PTY.spawn(ARGV[0]) do |lsh_out, lsh_in, pid|
      lsh_out.expect(/#{@prompt}/, TIMEOUT) do |r|
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

$stderr.puts "\n#{@passes} out of #{valid_tests.size} tests passed of #{@run} test cases run. "
if @run < valid_tests.size
  $stderr.puts "#{valid_tests.size - @run} test cases could not run. Fix failing unit-test-#{@run - 1} to proceed\n"
end
