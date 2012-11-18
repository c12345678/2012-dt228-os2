#!/usr/bin/env ruby
# vim: set ts=2 sw=2 expandtab:

require 'pty'
require 'expect'
require 'fileutils'

class Runnable
  def redirectto
    begin
      File.read("outfile.txt").chomp
    rescue => e
      "Not found"
    end
  end

  def self.cleanup
    begin
      FileUtils.rm "outfile.txt"
    rescue => e
    end
  end
end

def msg(*args)
  unless @quiet
    $stderr.print *args
    $stderr.print "\n"
  end
end

def fail(*args)
  unless @quiet
    $stderr.print "\n"
    $stderr.print "Fail: "
  end
  msg *args
  @fails += 1
end

def fail_and_exit(*args)
  @quiet = false
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
@passes=0
@fails=0
@run=0
@grade = 0.0
@prompt=%x(make --silent showprompt 2>/dev/null).chomp
@prompt="lsh>> " if @prompt == ""
@grading = ARGV.include? "--grade"
@quiet = @grading


@tests = [
  {tc: 'Run a command with no arguments', depends: :EXTERNAL, cmd: path_for('uname'), expected: %x{uname}.chomp, explanation: "Unexpected output from running the a command", marks: 0 },
  {tc: 'Check if fork() is implemented', depends: :EXTERNAL, cmd: path_for('ps'), expected: 'lsh', explanation: "Command not executed in fork()", marks: 0 },
  {tc: 'Run a command with a single argument', depends: :EXTERNAL, cmd: path_for('uname') + ' -n', expected: %x{uname -n}.chomp, explanation: "No support for passing command line arguments (1 arg)", marks: 0 },
  {tc: 'Run a command with three arguments', depends: :EXTERNAL, cmd: path_for('expr') + ' 1 + 1', expected: '^2', explanation: "No support for passing command line arguments (3 args)", marks: 0 },
  {tc: 'Run a commmand with multiple arguments', depends: :EXTERNAL, cmd: path_for('expr') + ' 5 * 6 + 7 / 2 - 8 * 3', expected: '^9', explanation: "No support for passing command line arguments (11 args)", marks: 0 },
  {tc: 'Check if PATH lookup is implemented for a command that exists', depends: :EXTERNAL, cmd: path_for('uname'), expected: %x{uname}.chomp, explanation: "Command in PATH but not executed ", marks: 0 },
  {tc: 'Check how PATH lookup is implemented for a command that does not exist', depends: :EXTERNAL, cmd: 'xyz', expected: 'No such file or directory', explanation: "Command not in PATH but expected error not thrown", marks: 0 },
  {tc: 'Check if relative path commands will run', depends: :EXTERNAL, cmd: relative_path_to(path_for('uname')), expected: %x{uname}.chomp, explanation: "Relative path command not executed ", marks: 0 },
  {tc: 'Check if environment variables are implemented', depends: :USERVARS, cmd: 'PS1="% "', expected: '% ', explanation: "Failed to set a new command prompt into PS1", marks: 0 },
  {tc: 'Print environment variables with an internal command', depends: :EXTERNAL, cmd: 'env', expected: 'PS1=', explanation: "Expected to be able view environment variables with the 'env' command", marks: 3 },
  {tc: 'Check if command backgrounding is implemented', depends: :EXTERNAL, cmd: 'tests/sleep.sh &' , expected: @prompt, explanation: "Command backgrounding not implemented or not working", marks: 3 },
  {tc: 'Check if output redirection is implemented', depends: :EXTERNAL, cmd: 'echo redirection > outfile.txt', expected: 'Runnable.redirectto', explanation: "Output redirection not implemented or not working", marks: 3 },
  {tc: 'Check if input redirection is implemented', depends: :EXTERNAL, cmd: 'cat < outfile.txt', expected: 'redirection', explanation: "Input redirection not implemented or not working", marks: 3 },
  {tc: 'Check if command pipelining is implemented (no command arguments)', depends: :EXTERNAL, cmd: 'uname | wc' , expected: %x{uname | wc}.chomp, explanation: "Command pipelining not implemented or not working", marks: 1.5 },
  {tc: 'Check if command pipelining is implemented (with command arguments)', depends: :EXTERNAL, cmd: 'cat outfile.txt | wc -l' , expected: '1', explanation: "Command pipelining not implemented or not working with command arguments", marks: 1.5 },
  {tc: 'BONUS (Not graded): Check if simultaneuous input and output redirection is implemented', depends: :EXTERNAL, cmd: 'cat < outfile.txt > /dev/tty' , expected: 'redirection', explanation: "Simultaneous use of < and > not implemented or not working", marks: 0 },
  {tc: 'BONUS (Not graded): Check if multi-stage pipelining is implemented', depends: :EXTERNAL, cmd: 'uname -a | cat | wc -l' , expected: %{uname -a | cat | wc -l}.chomp, explanation: "3-stage pipeline not implemented or not working", marks: 0 },
  {tc: 'BONUS (Not graded): Check if internal commands can be pipelined', depends: :EXTERNAL, cmd: 'env|grep PATH' , expected: '/usr/bin', explanation: "3-stage pipeline not implemented or not working", marks: 0 },
  {tc: 'BONUS (Not graded): Check if all redirection features work together', depends: :EXTERNAL, cmd: 'cat < outfile.txt | cat > /dev/tty ' , expected: 'redirection', explanation: "Multifuncitonal redirection not implemented or not working", marks: 0 },
]

def valid_tests
  return @valid_tests if @valid_tests
  features=%x(make --silent listfeatures 2>/dev/null).split(/-DLSH_ENABLE_/).map(&:strip).map(&:to_sym)[1..-1]
  @valid_tests = @tests.select { |t| features.include?(t[:depends]) }
end

#$expect_verbose=1
begin
  valid_tests.each_with_index do |test, i|
    @run = i + 1
    PTY.spawn(ARGV[0]) do |lsh_out, lsh_in, pid|
      lsh_out.expect(/#{@prompt}/, TIMEOUT) do |r|
        fail_and_exit "Doesn't seem to have run the shell" if r.nil?
        msg "unit-test-#{i}: " + test[:tc] + "\t(" + test[:cmd].inspect + ')'
        if test[:cmd].is_a?(Array)
          test[:cmd].each do |c|
            lsh_in.print c + "\r"
          end
        else
          lsh_in.print test[:cmd] + "\r"
        end
        expected = if test[:expected].start_with?('Runnable')
          _, method = test[:expected].split /\./
          Runnable.new.send(method.to_sym)
        else
          test[:expected]
        end
        lsh_out.expect(/#{expected}/, TIMEOUT) do |r|
          if r.nil?
            fail test[:explanation]
          else
            @passes += 1
            @grade += test[:marks]
          end
        end
      end
    end
  end
rescue => e
  case
  when e.is_a?(Errno::ENOENT)
    fail_and_exit "Can't find #{ARGV[0]} binary. Run these tests with 'make tests' from within the source folder"
  end
end

msg "\n#{@passes} out of #{valid_tests.size} tests passed of #{@run} test cases run. "
if @run < valid_tests.size
  msg "#{valid_tests.size - @run} test cases could not run. Fix failing unit-test-#{@run - 1} to proceed"
end
if @grading
  $stderr.print "#{@grade}\n"
end

Runnable.cleanup

exit 0
