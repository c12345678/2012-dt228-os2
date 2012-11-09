#!/usr/bin/env ruby

require 'fileutils'

def authenticity_token(url, cookie)
  meta_tags = %x{curl #{url}/submissions/new -L --cookie-jar #{cookie} -s | grep csrf}.split
  begin
    meta_tags[5].scan(/content="(.*)"/).first.first
  rescue => e
    nil
  end
end

def submit(id, url, file, cookie_jar, token)
  %x{curl #{url}/submissions -L -c #{cookie_jar} -s -F "submission[student_id]=#{id}" -F "submission[upload]=@#{file};type=application/zip" -F authenticity_token=#{token} }
end

if ARGV.length != 3
  $stderr.puts "Usage: #{$0} <url> <id> <pin>"
  exit 1
end

# Check that we are where we should be
dirs = Dir.getwd.split('/').pop(2)
if dirs[0] != "assignment1" && dirs[1] != "submit"
  $stderr.puts "You have not changed to the 'submit' directory of the 'assignment1' directory"
  exit 1
end

cookie_jar = 'cookie.txt'
id = ARGV[1].downcase
pin = ARGV[2]

parts = ARGV[0].split("//")
url = parts[0] + "//" + id + ":" + pin + "@" + parts[1]
token = authenticity_token url, cookie_jar

if token.nil?
  $stderr.puts "#{$0}: Error getting auth token. Check networking is enabled and your ID and PIN are correct"
  exit 1
end

file = id + ".tar.gz"
begin
  FileUtils.remove_dir "assignment1", true
  FileUtils.rm file
rescue => e
end

FileUtils.mkdir "assignment1"
FileUtils.cd('..') do
  begin
    (["Makefile"] + Dir.glob("*.[ch]")).each do |file|
      FileUtils.cp file, "submit/assignment1"
    end
  rescue => e
    $stderr.puts "#{$0}: No project files found! Check you are in the right directory"
    exit 1
  end
end


%x{tar zcf #{file} assignment1}
unless Dir.glob(file).first == file
  $stderr.puts "#{$0}: Error creating submission bundle. Check that you have tar(1) and gzip(1) installed"
end

$stderr.puts submit(id, url, file, cookie_jar, token)
exit 0
