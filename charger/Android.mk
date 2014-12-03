LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE            := chargerlogo
LOCAL_SRC_FILES         := sbin/chargerlogo
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/sbin
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := errorlogo.png
LOCAL_SRC_FILES         := res/images/errorlogo.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := battery_0.png
LOCAL_SRC_FILES         := res/images/charger/battery_0.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images/charger
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := battery_1.png
LOCAL_SRC_FILES         := res/images/charger/battery_1.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images/charger
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := battery_2.png
LOCAL_SRC_FILES         := res/images/charger/battery_2.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images/charger
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := battery_3.png
LOCAL_SRC_FILES         := res/images/charger/battery_3.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images/charger
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := battery_4.png
LOCAL_SRC_FILES         := res/images/charger/battery_4.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images/charger
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := battery_5.png
LOCAL_SRC_FILES         := res/images/charger/battery_5.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images/charger
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := battery_charge.png
LOCAL_SRC_FILES         := res/images/charger/battery_charge.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images/charger
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE            := battery_fail.png
LOCAL_SRC_FILES         := res/images/charger/battery_fail.png
LOCAL_MODULE_PATH       := $(TARGET_ROOT_OUT)/res/images/charger
LOCAL_MODULE_CLASS      := ETC
include $(BUILD_PREBUILT)
