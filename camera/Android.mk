LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    CameraWrapper.cpp

LOCAL_SHARED_LIBRARIES := \
    libhardware liblog libcamera_client libgui libutils libhidltransport android.hidl.token@1.0-utils

LOCAL_STATIC_LIBRARIES := \
    libarect libbase

LOCAL_C_INCLUDES += \
    framework/native/include \
    system/media/camera/include

LOCAL_HEADER_LIBRARIES := libnativebase_headers

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/hw
LOCAL_MODULE := camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
