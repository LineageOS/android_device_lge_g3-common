#!/system/bin/sh
# Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of The Linux Foundation nor
#       the names of its contributors may be used to endorse or promote
#       products derived from this software without specific prior written
#       permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

export PATH=/system/bin

case "$1" in
    "msm8974")
        setprop ro.sf.lcd_density 320
        ;;
esac

# Setup HDMI related nodes & permissions
# HDMI can be fb1 or fb2
# Loop through the sysfs nodes and determine
# the HDMI(dtv panel)
for fb_cnt in 0 1 2
do
file=/sys/class/graphics/fb$fb_cnt
dev_file=/dev/graphics/fb$fb_cnt
  if [ -d "$file" ]
  then
    value=`cat $file/msm_fb_type`
    case "$value" in
            "dtv panel")
        chown -h system.graphics $file/hpd
        chown -h system.system $file/hdcp/tp
        chown -h system.graphics $file/vendor_name
        chown -h system.graphics $file/product_description
        chmod -h 0664 $file/hpd
        chmod -h 0664 $file/hdcp/tp
        chmod -h 0664 $file/vendor_name
        chmod -h 0664 $file/product_description
        chmod -h 0664 $file/video_mode
        chmod -h 0664 $file/format_3d
        # create symbolic link
        ln -s $dev_file /dev/graphics/hdmi
        # Change owner and group for media server and surface flinger
        chown -h system.system $file/format_3d;;
    esac
  fi
done
