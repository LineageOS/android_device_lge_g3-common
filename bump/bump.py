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
import math
import os
import urllib
import re
import struct
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

def pair_reverse(s):
    n = len(s) / 2
    fmt = '%dh' % n
    return struct.pack(fmt, *reversed(struct.unpack(fmt, s)))

bump_url = "http://apps.codefi.re/bump/api/v1/bump"
sig_size = 1024
file = open(file_name, "a+b")
actual_size = os.path.getsize(file_name)
file.seek(36, 0)
page_size = int(pair_reverse(binascii.b2a_hex(file.read(4))), 16)
num_pages = actual_size / page_size

file.seek(8, 0)
paged_kernel_size = (int(pair_reverse(binascii.b2a_hex(file.read(4))), 16) / page_size) * page_size

file.seek(16, 0)
paged_ramdisk_size = (int(pair_reverse(binascii.b2a_hex(file.read(4))), 16) / page_size) * page_size

file.seek(24, 0)
paged_second_size = (int(pair_reverse(binascii.b2a_hex(file.read(4))), 16) / page_size) * page_size
if paged_second_size <= 0:
    paged_second_size = 0

file.seek(40, 0)
paged_dt_size = (int(pair_reverse(binascii.b2a_hex(file.read(4))), 16) / page_size) * page_size
if paged_dt_size <= 0:
    paged_dt_size = 0

calculated_size = page_size + paged_kernel_size + paged_ramdisk_size + paged_second_size + paged_dt_size

if calculated_size > actual_size:
    print "%s appears to be invalid. The calculated size is greater than the actual size." % file_name
    file.close()
    sys.exit(1)
elif actual_size > calculated_size:
    difference = actual_size - calculated_size
    if difference != page_size and difference != (page_size * 2):
        if difference != sig_size and difference != (page_size + sig_size) and difference != (2 * page_size + 1024):
            print "%s appears to be padded. Attempting to remove padding..." % file_name
            print "Beware: this may invalidate your %s" % file_name
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
        else:
            print "%s appears to already be bumped. Bailing out!" % file_name
            sys.exit(1)

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
