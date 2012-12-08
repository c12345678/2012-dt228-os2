#!/usr/bin/env ruby
# vim: set ts=2 sw=2 expandtab:

require 'pty'
require 'expect'
require 'fileutils'

class Tests
  def initialize
    @@threads = []
  end

  def self.cleanup
    FileUtils.rm "/dev/shm/shm_dt228_os2", force: true
    FileUtils.rm "tests.log", force: true
  end

  def self.sync
  end

  def tc1
    client "1:0"
  end

  def tc2
    0.upto(15) do |n|
      client "2:#{n}"
    end
  end

  def tc3
    0.upto(40) do |n|
      client "3:#{n}"
    end
  end

  def tc4(j=4)
    0.upto(31) do |n|
      @@threads << Thread.new do
        client "#{j}:#{n}"
      end
    end
    @@threads.each do |t|
      t.join
    end
  end

  def tc5
    tc4 5
  end
end

class Assertions
  def tc1(result)
    if result == [0]
      return 1
    else
      return 0
    end
  end

  def tc2(result)
    if result == (0..15).to_a
      return 1
    else
      return 0
    end
  end

  def tc3(result)
    if result == (0..40).to_a
      return 1
    else
      return 0
    end
  end

  def tc4(result)
    if result == (0..31).to_a
      return 1
    else
      return 0
    end
  end

  def tc5(result)
    if result == (0..31).to_a
      return 1
    else
      return 0
    end
  end
end

def msg(*args)
  unless @quiet
    $stderr.print *args
    $stderr.print "\n"
  end
end

def fail(quiet = false, *args)
  unless quiet
    $stderr.print "\n"
    $stderr.print "Fail: "
  end
  msg *args
  @fails += 1
end

def fail_and_exit(*args)
  fail(true, *args)
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

def valid_tests
  @tests
end

def run(*args)
  begin
    system *args
  rescue => e
    fail_and_exit e.inspect
  end
end

def server_running?
  %x{pgrep shm_server}.chomp != ""
end

def client_running?
  %x{pgrep shm_client}.chomp != ""
end

def init
  Tests.cleanup
end

def start_server(usecs=0)
  run ARGV[0] + " #{usecs} >> #{LOGFILE} &"
  sleep 1
  fail_and_exit "Count not start server" unless server_running?
end

def restart_server(usecs=0)
  system "pkill shm_server"
  start_server usecs
end

def client(str)
  fail_and_exit "No server running!" unless server_running?
  run ARGV[1] + " '#{str}'"
end

@tests = [
  {tc: 'Check one message can be written and read', explanation: "Server (consumer) did not display or did not receive message", marks: 3 },
  {tc: 'Check 16 messages can be written and read in order', marks: 3},
  {tc: 'Check more than 16 messages can be processed in order (serialized producers)', marks: 3},
  {tc: 'Check more than 16 messages can be processed in order (concurrent producers)', marks: 3},
  {tc: 'Check more than 16 messages can be processed (slow consumer, concurrent producers)', wait: 10000, marks: 3},
]

@passes=0
@fails=0
@run=0
@grade = 0.0
@grading = ARGV.include? "--grade"
@quiet = @grading
LOGFILE="tests.log"

if not ARGV[0] or not ARGV[1]
  fail_and_exit "Usage: #{$0} <server_path> <client_path>"
end

# Run the test cases
init
valid_tests.each_with_index do |test, i|
  n = i + 1
  msg "unit-test-#{n}: " + test[:tc]
  wait = test[:wait] || 0
  restart_server wait
  begin
    Tests.new.send("tc#{n}".to_sym)
    @run += 1
  rescue => e
  end
end

# Sync with clients

# Check the results
sleep 5
results = {1 => [], 2 => [], 3 => [], 4 => [], 5 => []}
IO.popen("sort -t: -k1n -k2n #{LOGFILE}") do |log|
  lines = log.readlines
  lines.each do |line|
    key,val = line.chomp.split /:/
    results[key.to_i] << val.to_i
  end
end
results.keys.sort.each do |n|
  begin
    @passes += Assertions.new.send("tc#{n}".to_sym, results[n])
    @grade += valid_tests[n][:marks]
  rescue => e
    #msg e.inspect
  end
end

msg "\n#{@passes} out of #{valid_tests.size} tests passed of #{@run} test cases run. "
if @run < valid_tests.size
  msg "#{valid_tests.size - @run} test cases could not run. Fix failing unit-test-#{@run - 1} to proceed"
end
if @grading
  $stderr.print "#{@grade}\n"
end


exit 0
