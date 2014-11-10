#!/usr/bin/env python

##################################################
#                                                #
#  BUMP! Script                                  #
#                                                #
#  Leverages codefi.re's web api to              #
#  automatically bump your images!               #
#                                                #
#  Usage:                                        #
#     bump.py <image_file> [api_key]             #
#                                                #
#  You can obtain an api key by creating a free  #
#  account at http://apps.codefi.re/bump         #
#                                                #
#  Alternatively, you may set your environment   #
#  variable: CF_BUMP_API_KEY to the key.         #
#  (protip: export it in your .bashrc)           #
#                                                #
##################################################

import binascii
import hashlib
import json
import os
import urllib
import re
import sys

try:
  # For python3
  import urllib.request
except ImportError:
  # For python2
  import imp
  import urllib2
  urllib = imp.new_module("urllib")
  urllib.request = urllib2

if sys.argv[1] == "-h" or sys.argv[1] == "--help" or sys.argv[1] == "help":
  print ""
  print "Usage: bump.py <image_file> [api_key]"
  print "  image_file  - <required> path to an image file to bump"
  print "  api_key     - [optional] api key, if not specified CF_BUMP_API_KEY environment variable will be used"
  print ""
  print "Create an account at: http://apps.codefi.re/bump to get your api key"
  print ""
  sys.exit(0)

file_name = sys.argv[1]
api_key = ""

if len(sys.argv) >= 3:
  api_key = sys.argv[2]
else:
  api_key = os.environ.get("CF_BUMP_API_KEY")

if (api_key is None) or (len(api_key) == 0):
  print "No API key specified!"
  print "Go to http://apps.codefi.re/bump sign up and generate an api key."
  print "Then set the environment variable CF_BUMP_API_KEY to that key"
  print "  or pass it to this script as the second parameter."
  sys.exit(1)

bump_url = "http://apps.codefi.re/bump/api/v1/bump"

file = open(file_name, "a+b")

file.seek(36, 0)
data = binascii.b2a_hex(file.read(4))
page_size = int(((data[6] + data[7]) + (data[4] + data[5]) + (data[2] + data[3]) + (data[0] + data[1])), 16)

sig_size = 1024
sig_magic = "41a9e467744d1d1ba429f2ecea655279ee9333001f7cdaa665eb501dd9d661fe"
file_size = os.path.getsize(file_name)
num_pages = file_size / page_size

file.seek(0,0)
file.seek(-sig_size, 2)
data = binascii.b2a_hex(file.read(sig_size))

if data.startswith(sig_magic):
  print "%s" % file_name + " is already bumped!"
  file.close()
  sys.exit(1)

global i
i = num_pages - 1
file.seek(0, 0)

while i >= 0:
  file.seek(page_size * i, 0)
  data = file.read(page_size)
  data = data.split('\x00')[0]

  if not data:
    file.truncate(page_size * i)
    i = i - 1
  else:
    break

sha1 = hashlib.sha1()
file.seek(0, 0)
sha1.update(file.read())

request_url = bump_url + "/" + sha1.hexdigest() + "/" + api_key

response = urllib.request.urlopen(request_url)
json_data = response.read().decode("utf-8")
data = json.loads(json_data)

if data["status"] != "success":
  print "Bump request failed, please try again"
  file.close()
  sys.exit(1)

file.write(binascii.a2b_hex(data["bump"]))
file.close()

print "%s is now bumped! Happy flashing!" % file_name
