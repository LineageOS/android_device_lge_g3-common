#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

DEVICE_COMMON=g3-common
VENDOR=lge

# Load extractutils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

LINEAGE_ROOT="$MY_DIR"/../../..

HELPER="$LINEAGE_ROOT"/vendor/lineage/build/tools/extract_utils.sh
if [ ! -f "$HELPER" ]; then
    echo "Unable to find helper script at $HELPER"
    exit 1
fi
. "$HELPER"

if [ $# -eq 0 ]; then
    SRC=adb
else
    if [ $# -eq 1 ]; then
        SRC=$1
    else
        echo "$0: bad number of arguments"
        echo ""
        echo "usage: $0 [PATH_TO_EXPANDED_ROM]"
        echo ""
        echo "If PATH_TO_EXPANDED_ROM is not specified, blobs will be extracted from"
        echo "the device using adb pull."
        exit 1
    fi
fi

# Initialize the helper for common device
setup_vendor "$DEVICE_COMMON" "$VENDOR" "$LINEAGE_ROOT" true

extract "$MY_DIR"/proprietary-files.txt "$SRC"

# Initialize the helper for device
setup_vendor "$DEVICE" "$VENDOR" "$LINEAGE_ROOT"

extract "$MY_DIR"/../$DEVICE/proprietary-files.txt "$SRC"

LIBDSI_NETCTRL="$LINEAGE_ROOT"/vendor/"$VENDOR"/"$DEVICE"/proprietary/vendor/lib/libdsi_netctrl.so
sed -i 's|/system/etc/data/dsi_config.xml|/vendor/etc/data/dsi_config.xml|g' "$LIBDSI_NETCTRL"

IMSCMLIBRARY="$LINEAGE_ROOT"/vendor/"$VENDOR"/"$DEVICE_COMMON"/proprietary/vendor/etc/permissions/imscm.xml
sed -i 's|/system/framework|/vendor/framework|g' "$IMSCMLIBRARY"

NETMGRD="$LINEAGE_ROOT"/vendor/"$VENDOR"/"$DEVICE"/proprietary/vendor/bin/netmgrd
sed -i 's|/system/etc/data/netmgr_config.xml|/vendor/etc/data/netmgr_config.xml|g' "$NETMGRD"

THERMAL_ENGINE="$LINEAGE_ROOT"/vendor/"$VENDOR"/"$DEVICE_COMMON"/proprietary/vendor/bin/thermal-engine
sed -i 's|/system/etc|/vendor/etc|g' "$THERMAL_ENGINE"

"$MY_DIR"/setup-makefiles.sh
