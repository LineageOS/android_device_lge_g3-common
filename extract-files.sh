#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

DEVICE_COMMON=g3-common
VENDOR=lge

# Load extractutils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

ANDROID_ROOT="$MY_DIR/../../.."

HELPER="$ANDROID_ROOT/tools/extract-utils/extract_utils.sh"
if [ ! -f "$HELPER" ]; then
    echo "Unable to find helper script at $HELPER"
    exit 1
fi
. "$HELPER"

function blob_fixup() {
    case "${1}" in
        vendor/lib/mediadrm/libwvdrmengine.so)
            patchelf --replace-needed "libprotobuf-cpp-lite.so" "libprotobuf-cpp-lite-v29.so" "${2}"
            ;;
        vendor/lib/libdsi_netctrl.so)
            sed -i 's|/system/etc/data/dsi_config.xml|/vendor/etc/data/dsi_config.xml|g' "${2}"
            ;;
        vendor/bin/netmgrd)
            sed -i 's|/system/etc/data/netmgr_config.xml|/vendor/etc/data/netmgr_config.xml|g' "${2}"
            ;;
        vendor/bin/thermal-engine)
            sed -i 's|/system/etc|/vendor/etc|g' "${2}"
            ;;
    esac
}

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
setup_vendor "$DEVICE_COMMON" "$VENDOR" "$ANDROID_ROOT" true

extract "$MY_DIR"/proprietary-files.txt "$SRC"

# Initialize the helper for device
setup_vendor "$DEVICE" "$VENDOR" "$ANDROID_ROOT"

extract "$MY_DIR"/../$DEVICE/proprietary-files.txt "$SRC"

"$MY_DIR"/setup-makefiles.sh
