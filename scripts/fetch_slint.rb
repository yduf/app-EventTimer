#!/usr/bin/env ruby
require "digest"
require "fileutils"
require "net/http"
require "tmpdir"
require "uri"

VERSION = "1.18.1"
ASSET = "Slint-cpp-#{VERSION}-Linux-x86_64.tar.gz"
URL = "https://github.com/slint-ui/slint/releases/download/v#{VERSION}/#{ASSET}"
SHA256 = "f0b24ea601b80afe241677d1179bf5215dceae529f2b275b678a0af9a144b255"

ROOT = File.expand_path("..", __dir__)
TARGET = File.join(ROOT, "subprojects", "slint-cpp")

def installed?(dir)
  File.executable?(File.join(dir, "bin", "slint-compiler")) &&
    File.exist?(File.join(dir, "lib", "libslint_cpp.so")) &&
    File.directory?(File.join(dir, "include", "slint"))
end

def download(url, dest, limit = 5)
  raise "too many redirects" if limit.zero?

  uri = URI.parse(url)
  Net::HTTP.start(uri.host, uri.port, use_ssl: uri.scheme == "https") do |http|
    request = Net::HTTP::Get.new(uri)
    http.request(request) do |response|
      case response
      when Net::HTTPRedirection
        return download(URI.join(url, response["location"]).to_s, dest, limit - 1)
      when Net::HTTPSuccess
        File.open(dest, "wb") do |file|
          response.read_body { |chunk| file.write(chunk) }
        end
      else
        raise "download failed: HTTP #{response.code} #{response.message}"
      end
    end
  end
end

def package_root(dir)
  return dir if installed?(dir)

  Dir.children(dir).each do |entry|
    candidate = File.join(dir, entry)
    return candidate if File.directory?(candidate) && installed?(candidate)
  end
  nil
end

if installed?(TARGET) && !ARGV.include?("--force")
  puts "Slint C++ #{VERSION} already unpacked in #{TARGET}"
  exit 0
end

Dir.mktmpdir("slint-fetch") do |tmp|
  archive = File.join(tmp, ASSET)
  puts "Downloading #{URL}"
  download(URL, archive)

  digest = Digest::SHA256.file(archive).hexdigest
  unless digest == SHA256
    abort "checksum mismatch for #{ASSET}\n  expected #{SHA256}\n  actual   #{digest}"
  end

  unpacked = File.join(tmp, "unpacked")
  FileUtils.mkdir_p(unpacked)
  abort "unpacking #{ASSET} failed" unless system("tar", "-xzf", archive, "-C", unpacked)

  source = package_root(unpacked)
  abort "unexpected layout in #{ASSET}: no bin/slint-compiler, lib/libslint_cpp.so and include/slint" unless source

  FileUtils.rm_rf(TARGET)
  FileUtils.mkdir_p(File.dirname(TARGET))
  FileUtils.mv(source, TARGET)
end

puts "Slint C++ #{VERSION} installed in #{TARGET}"
