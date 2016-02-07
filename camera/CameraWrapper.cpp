/*
 * Copyright (C) 2012-2015, The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
* @file CameraWrapper.cpp
*
* This file wraps a vendor camera module.
*
*/

//#define LOG_NDEBUG 0

#define LOG_TAG "CameraWrapper"
#include <cutils/log.h>

#include <utils/threads.h>
#include <utils/String8.h>
#include <hardware/hardware.h>
#include "hardware/camera.h"
#include <camera/Camera.h>
#include <camera/CameraParameters.h>

static android::Mutex gCameraWrapperLock;
static camera_module_t *gVendorModule = 0;

static int camera_device_open(const hw_module_t *module, const char *name,
        hw_device_t **device);
static int camera_device_close(hw_device_t* device);
static int camera_get_number_of_cameras(void);
static int camera_get_camera_info(int camera_id, struct camera_info *info);

static struct hw_module_methods_t camera_module_methods = {
    .open = camera_device_open
};

camera_module_t HAL_MODULE_INFO_SYM = {
    .common = {
         .tag = HARDWARE_MODULE_TAG,
         .module_api_version = CAMERA_MODULE_API_VERSION_1_0,
         .hal_api_version = HARDWARE_HAL_API_VERSION,
         .id = CAMERA_HARDWARE_MODULE_ID,
         .name = "G3 Camera Wrapper",
         .author = "The CyanogenMod Project",
         .methods = &camera_module_methods,
         .dso = NULL, /* remove compilation warnings */
         .reserved = {0}, /* remove compilation warnings */
    },
    .get_number_of_cameras = camera_get_number_of_cameras,
    .get_camera_info = camera_get_camera_info,
    .set_callbacks = NULL, /* remove compilation warnings */
    .get_vendor_tag_ops = NULL, /* remove compilation warnings */
    .open_legacy = NULL, /* remove compilation warnings */
    .set_torch_mode = NULL, /* remove compilation warnings */
    .init = NULL, /* remove compilation warnings */
    .reserved = {0}, /* remove compilation warnings */
};

typedef struct wrapper_camera_device {
    camera_device_t base;
    int id;
    camera_device_t *vendor;
} wrapper_camera_device_t;

#define VENDOR_CALL(device, func, ...) ({ \
    wrapper_camera_device_t *__wrapper_dev = (wrapper_camera_device_t*) device; \
    __wrapper_dev->vendor->ops->func(__wrapper_dev->vendor, ##__VA_ARGS__); \
})

static bool flipZsl = false;
static bool zslState = false;
static bool previewRunning = false;
static bool activeFocusMove = false;
static camera_notify_callback sNotifCb;

#define CAMERA_ID(device) (((wrapper_camera_device_t *)(device))->id)

static void notify_intercept(int32_t msg, int32_t b, int32_t c, void *cookie) {
    if (msg == CAMERA_MSG_FOCUS) {
        ALOGV("GOT FOCUS MESSAGE: %d",b);
    } else if (msg == CAMERA_MSG_FOCUS_MOVE && b == 1) {
        ALOGV("GOT FOCUS MOVE START");
        activeFocusMove = true;
    } else if (msg == CAMERA_MSG_FOCUS_MOVE && b == 0) {
        ALOGV("GOT FOCUS MOVE STOP");
        activeFocusMove = false;
    }
    sNotifCb(msg, b, c, cookie);
}

static int check_vendor_module()
{
    int rv = 0;
    ALOGV("%s", __FUNCTION__);

    if (gVendorModule)
        return 0;

    rv = hw_get_module_by_class("camera", "vendor",
            (const hw_module_t**)&gVendorModule);
    if (rv)
        ALOGE("failed to open vendor camera module");
    return rv;
}

static bool is4k(android::CameraParameters &params) {
    int video_width, video_height;
    params.getVideoSize(&video_width, &video_height);

    return video_width*video_height > 1920*1080;
}

const static char * iso_values[] = {"auto,ISO50,ISO100,ISO150,ISO200,ISO250,ISO300,ISO350,ISO400,ISO450,ISO500,ISO600,ISO700,ISO800,ISO1000,ISO1500,ISO2000,ISO2700,auto","auto"};

static char *camera_fixup_getparams(int id, const char *settings)
{
    bool videoMode = false;
    const char* isoMode;
    char *manipBuf;

    android::CameraParameters params;
    params.unflatten(android::String8(settings));

#ifdef LOG_NDEBUG
    ALOGV("%s: original parameters:", __FUNCTION__);
    params.dump();
#endif

    const char *videoSizesStr = params.get(android::CameraParameters::KEY_SUPPORTED_VIDEO_SIZES);
    char tmpsz[strlen(videoSizesStr) + 10 + 1];
    sprintf(tmpsz, "3840x2160,%s", videoSizesStr);
    params.set(android::CameraParameters::KEY_SUPPORTED_VIDEO_SIZES, tmpsz);

    if (params.get(android::CameraParameters::KEY_RECORDING_HINT)) {
        videoMode = (!strcmp(params.get(android::CameraParameters::KEY_RECORDING_HINT), "true"));
    }

    params.set(android::CameraParameters::KEY_SUPPORTED_ISO_MODES, iso_values[id]);

    /* lge-iso to iso */
    if(params.get(android::CameraParameters::KEY_LGE_ISO_MODE)) {
        isoMode = params.get(android::CameraParameters::KEY_LGE_ISO_MODE);
        ALOGV("%s: ISO mode: %s", __FUNCTION__, isoMode);

        if(strcmp(isoMode, "auto") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "auto");
        } else if(strcmp(isoMode, "50") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO50");
        } else if(strcmp(isoMode, "100") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO100");
        } else if(strcmp(isoMode, "150") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO150");
        } else if(strcmp(isoMode, "200") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO200");
        } else if(strcmp(isoMode, "250") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO250");
        } else if(strcmp(isoMode, "300") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO300");
        } else if(strcmp(isoMode, "350") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO350");
        } else if(strcmp(isoMode, "400") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO400");
        } else if(strcmp(isoMode, "450") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO450");
        } else if(strcmp(isoMode, "500") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO500");
        } else if(strcmp(isoMode, "600") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO600");
        } else if(strcmp(isoMode, "700") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO700");
        } else if(strcmp(isoMode, "800") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO800");
        } else if(strcmp(isoMode, "1000") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO1000");
        } else if(strcmp(isoMode, "1500") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO1500");
        } else if(strcmp(isoMode, "2000") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO2000");
        } else if(strcmp(isoMode, "2700") == 0) {
            params.set(android::CameraParameters::KEY_ISO_MODE, "ISO2700");
        }
    }

    /* Set supported scene modes */
    if (!videoMode) {
        manipBuf = strdup(params.get(android::CameraParameters::KEY_SUPPORTED_SCENE_MODES));
        if (manipBuf != NULL && strstr(manipBuf,"hdr") == NULL) {
            strncat(manipBuf,",hdr",4);
            params.set(android::CameraParameters::KEY_SUPPORTED_SCENE_MODES,
                manipBuf);
        }
        free(manipBuf);
    }

    if (id == 0 && is4k(params)) {
        params.set("preview-format", "yuv420sp");
        params.set("video-snapshot-supported", "false");
    }

    /* LIE! The camera will set 3 snaps when doing HDR, and only return one. This hangs apps
     * that wait for the rest to come in. Make sure we never return multiple snaps unless
     * doing ZSL */
    if (!videoMode && (!params.get("zsl") || strncmp(params.get("zsl"),"on", 2))) {
        params.set("num-snaps-per-shutter", "1");
    }

#ifdef LOG_NDEBUG
    ALOGV("%s: fixed parameters:", __FUNCTION__);
    params.dump();
#endif

    android::String8 strParams = params.flatten();
    char *ret = strdup(strParams.string());

    return ret;
}

static char *camera_fixup_setparams(int id, const char *settings)
{
    bool videoMode = false;
    const char* isoMode;

    android::CameraParameters params;
    params.unflatten(android::String8(settings));

#ifdef LOG_NDEBUG
    ALOGV("%s: original parameters:", __FUNCTION__);
    params.dump();
#endif

    params.set(android::CameraParameters::KEY_LGE_CAMERA, (id == 0 && is4k(params)) ? "1" : "0");

    if (params.get(android::CameraParameters::KEY_RECORDING_HINT)) {
        videoMode = (!strcmp(params.get(android::CameraParameters::KEY_RECORDING_HINT), "true"));
    }

    if (!videoMode && !strncmp(params.get(android::CameraParameters::KEY_SCENE_MODE),"hdr",3)) {
        params.set("hdr-mode", "1");
    } else {
        params.set("hdr-mode", "0");
    }

    /* iso to lge-iso */
    if(params.get(android::CameraParameters::KEY_ISO_MODE)) {
        isoMode = params.get(android::CameraParameters::KEY_ISO_MODE);
        ALOGV("%s: ISO mode: %s", __FUNCTION__, isoMode);

        params.set(android::CameraParameters::KEY_ISO_MODE, "auto");

        if(strcmp(isoMode, "auto") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "auto");
        } else if(strcmp(isoMode, "ISO50") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "50");
        } else if(strcmp(isoMode, "ISO100") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "100");
        } else if(strcmp(isoMode, "ISO150") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "150");
        } else if(strcmp(isoMode, "ISO200") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "200");
        } else if(strcmp(isoMode, "ISO250") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "250");
        } else if(strcmp(isoMode, "ISO300") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "300");
        } else if(strcmp(isoMode, "ISO350") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "350");
        } else if(strcmp(isoMode, "ISO400") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "400");
        } else if(strcmp(isoMode, "ISO450") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "450");
        } else if(strcmp(isoMode, "ISO500") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "500");
        } else if(strcmp(isoMode, "ISO600") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "600");
        } else if(strcmp(isoMode, "ISO700") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "700");
        } else if(strcmp(isoMode, "ISO800") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "800");
        } else if(strcmp(isoMode, "ISO1000") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "1000");
        } else if(strcmp(isoMode, "ISO1500") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "1500");
        } else if(strcmp(isoMode, "ISO2000") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "2000");
        } else if(strcmp(isoMode, "ISO2700") == 0) {
            params.set(android::CameraParameters::KEY_LGE_ISO_MODE, "2700");
        }
    }

    if (!strcmp(params.get("zsl"), "on")) {
        if (previewRunning && !zslState) { flipZsl = true; }
        zslState = true;
        params.set("camera-mode", "1");
    } else {
        if (previewRunning && zslState) { flipZsl = true; }
        zslState = false;
        params.set("camera-mode", "0");
    }

#ifdef LOG_NDEBUG
    ALOGV("%s: fixed parameters:", __FUNCTION__);
    params.dump();
#endif

    android::String8 strParams = params.flatten();
    char *ret = strdup(strParams.string());

    return ret;
}

/*******************************************************************
 * implementation of camera_device_ops functions
 *******************************************************************/

static int camera_set_preview_window(struct camera_device *device,
        struct preview_stream_ops *window)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, set_preview_window, window);
}

static void camera_set_callbacks(struct camera_device *device,
        camera_notify_callback notify_cb,
        camera_data_callback data_cb,
        camera_data_timestamp_callback data_cb_timestamp,
        camera_request_memory get_memory,
        void *user)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return;

    sNotifCb = notify_cb;
    VENDOR_CALL(device, set_callbacks, notify_intercept, data_cb, data_cb_timestamp,
            get_memory, user);
}

static void camera_enable_msg_type(struct camera_device *device,
        int32_t msg_type)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return;

    VENDOR_CALL(device, enable_msg_type, msg_type);
}

static void camera_disable_msg_type(struct camera_device *device,
        int32_t msg_type)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return;

    VENDOR_CALL(device, disable_msg_type, msg_type);
}

static int camera_msg_type_enabled(struct camera_device *device,
        int32_t msg_type)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return 0;

    return VENDOR_CALL(device, msg_type_enabled, msg_type);
}

static int camera_start_preview(struct camera_device *device)
{
    int rc = 0;
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    rc = VENDOR_CALL(device, start_preview);
    previewRunning = (rc == android::NO_ERROR);
    return rc;
}

static void camera_stop_preview(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return;

    previewRunning = false;
    VENDOR_CALL(device, stop_preview);
}

static int camera_preview_enabled(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, preview_enabled);
}

static int camera_store_meta_data_in_buffers(struct camera_device *device,
        int enable)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, store_meta_data_in_buffers, enable);
}

static char *camera_get_parameters(struct camera_device *device);
static int camera_set_parameters(struct camera_device *device, const char *params);
static int camera_start_recording(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return EINVAL;

    android::CameraParameters parameters;
    parameters.unflatten(android::String8(camera_get_parameters(device)));
    if (CAMERA_ID(device) == 0 && is4k(parameters)) {
        parameters.set("preview-format", "nv12-venus");
        parameters.set("picture-size", "4160x2340");
    }
    camera_set_parameters(device, strdup(parameters.flatten().string()));

    android::CameraParameters parameters2;
    parameters2.unflatten(android::String8(VENDOR_CALL(device, get_parameters)));
    parameters2.dump();

    return VENDOR_CALL(device, start_recording);
}

static void camera_stop_recording(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return;

    VENDOR_CALL(device, stop_recording);

    /* Restart preview after stop recording to flush buffers and not crash */
    VENDOR_CALL(device, stop_preview);
    VENDOR_CALL(device, start_preview);
}

static int camera_recording_enabled(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, recording_enabled);
}

static void camera_release_recording_frame(struct camera_device *device,
        const void *opaque)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return;

    VENDOR_CALL(device, release_recording_frame, opaque);
}

static int camera_auto_focus(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    if (activeFocusMove) {
       ALOGV("FORCED FOCUS MOVE STOP");
       VENDOR_CALL(device, cancel_auto_focus);
       activeFocusMove = false;
    }

    return VENDOR_CALL(device, auto_focus);
}

static int camera_cancel_auto_focus(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, cancel_auto_focus);
}

static int camera_take_picture(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, take_picture);
}

static int camera_cancel_picture(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, cancel_picture);
}

static int camera_set_parameters(struct camera_device *device,
        const char *params)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    char *tmp = NULL;
    tmp = camera_fixup_setparams(CAMERA_ID(device), params);

#ifdef LOG_NDEBUG
    __android_log_write(ANDROID_LOG_VERBOSE, LOG_TAG, tmp);
#endif

    if (flipZsl) {
        camera_stop_preview(device);
    }
    int ret = VENDOR_CALL(device, set_parameters, tmp);
    if (flipZsl) {
        camera_start_preview(device);
        flipZsl = false;
    }
    return ret;
}

static char *camera_get_parameters(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return NULL;

    char *params = VENDOR_CALL(device, get_parameters);

#ifdef LOG_NDEBUG
    __android_log_write(ANDROID_LOG_VERBOSE, LOG_TAG, params);
#endif

    char *tmp = camera_fixup_getparams(CAMERA_ID(device), params);
    VENDOR_CALL(device, put_parameters, params);
    params = tmp;

#ifdef LOG_NDEBUG
    __android_log_write(ANDROID_LOG_VERBOSE, LOG_TAG, params);
#endif

    return params;
}

static void camera_put_parameters(struct camera_device *device, char *params)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (params)
        free(params);
}

static int camera_send_command(struct camera_device *device,
        int32_t cmd, int32_t arg1, int32_t arg2)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, send_command, cmd, arg1, arg2);
}

static void camera_release(struct camera_device *device)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return;

    VENDOR_CALL(device, release);
}

static int camera_dump(struct camera_device *device, int fd)
{
    ALOGV("%s->%08X->%08X", __FUNCTION__, (uintptr_t)device,
            (uintptr_t)(((wrapper_camera_device_t*)device)->vendor));

    if (!device)
        return -EINVAL;

    return VENDOR_CALL(device, dump, fd);
}

extern "C" void heaptracker_free_leaked_memory(void);

static int camera_device_close(hw_device_t *device)
{
    int ret = 0;
    wrapper_camera_device_t *wrapper_dev = NULL;

    ALOGV("%s", __FUNCTION__);

    android::Mutex::Autolock lock(gCameraWrapperLock);

    if (!device) {
        ret = -EINVAL;
        goto done;
    }

    wrapper_dev = (wrapper_camera_device_t*) device;

    wrapper_dev->vendor->common.close((hw_device_t*)wrapper_dev->vendor);
    if (wrapper_dev->base.ops)
        free(wrapper_dev->base.ops);
    free(wrapper_dev);
done:
#ifdef HEAPTRACKER
    heaptracker_free_leaked_memory();
#endif
    return ret;
}

/*******************************************************************
 * implementation of camera_module functions
 *******************************************************************/

/* open device handle to one of the cameras
 *
 * assume camera service will keep singleton of each camera
 * so this function will always only be called once per camera instance
 */

static int camera_device_open(const hw_module_t *module, const char *name,
        hw_device_t **device)
{
    int rv = 0;
    int num_cameras = 0;
    int cameraid;
    wrapper_camera_device_t *camera_device = NULL;
    camera_device_ops_t *camera_ops = NULL;

    android::Mutex::Autolock lock(gCameraWrapperLock);

    ALOGV("%s", __FUNCTION__);

    if (name != NULL) {
        if (check_vendor_module())
            return -EINVAL;

        cameraid = atoi(name);
        num_cameras = gVendorModule->get_number_of_cameras();

        if (cameraid > num_cameras) {
            ALOGE("camera service provided cameraid out of bounds, "
                    "cameraid = %d, num supported = %d",
                    cameraid, num_cameras);
            rv = -EINVAL;
            goto fail;
        }

        camera_device = (wrapper_camera_device_t*)malloc(sizeof(*camera_device));
        if (!camera_device) {
            ALOGE("camera_device allocation fail");
            rv = -ENOMEM;
            goto fail;
        }
        memset(camera_device, 0, sizeof(*camera_device));
        camera_device->id = cameraid;

        rv = gVendorModule->common.methods->open(
                (const hw_module_t*)gVendorModule, name,
                (hw_device_t**)&(camera_device->vendor));
        if (rv) {
            ALOGE("vendor camera open fail");
            goto fail;
        }
        ALOGV("%s: got vendor camera device 0x%08X",
                __FUNCTION__, (uintptr_t)(camera_device->vendor));

        camera_ops = (camera_device_ops_t*)malloc(sizeof(*camera_ops));
        if (!camera_ops) {
            ALOGE("camera_ops allocation fail");
            rv = -ENOMEM;
            goto fail;
        }

        memset(camera_ops, 0, sizeof(*camera_ops));

        camera_device->base.common.tag = HARDWARE_DEVICE_TAG;
        camera_device->base.common.version = CAMERA_DEVICE_API_VERSION_1_0;
        camera_device->base.common.module = (hw_module_t *)(module);
        camera_device->base.common.close = camera_device_close;
        camera_device->base.ops = camera_ops;

        camera_ops->set_preview_window = camera_set_preview_window;
        camera_ops->set_callbacks = camera_set_callbacks;
        camera_ops->enable_msg_type = camera_enable_msg_type;
        camera_ops->disable_msg_type = camera_disable_msg_type;
        camera_ops->msg_type_enabled = camera_msg_type_enabled;
        camera_ops->start_preview = camera_start_preview;
        camera_ops->stop_preview = camera_stop_preview;
        camera_ops->preview_enabled = camera_preview_enabled;
        camera_ops->store_meta_data_in_buffers = camera_store_meta_data_in_buffers;
        camera_ops->start_recording = camera_start_recording;
        camera_ops->stop_recording = camera_stop_recording;
        camera_ops->recording_enabled = camera_recording_enabled;
        camera_ops->release_recording_frame = camera_release_recording_frame;
        camera_ops->auto_focus = camera_auto_focus;
        camera_ops->cancel_auto_focus = camera_cancel_auto_focus;
        camera_ops->take_picture = camera_take_picture;
        camera_ops->cancel_picture = camera_cancel_picture;
        camera_ops->set_parameters = camera_set_parameters;
        camera_ops->get_parameters = camera_get_parameters;
        camera_ops->put_parameters = camera_put_parameters;
        camera_ops->send_command = camera_send_command;
        camera_ops->release = camera_release;
        camera_ops->dump = camera_dump;

        *device = &camera_device->base.common;
    }

    return rv;

fail:
    if (camera_device) {
        free(camera_device);
        camera_device = NULL;
    }
    if (camera_ops) {
        free(camera_ops);
        camera_ops = NULL;
    }
    *device = NULL;
    return rv;
}

static int camera_get_number_of_cameras(void)
{
    ALOGV("%s", __FUNCTION__);
    if (check_vendor_module())
        return 0;
    return gVendorModule->get_number_of_cameras();
}

static int camera_get_camera_info(int camera_id, struct camera_info *info)
{
    ALOGV("%s", __FUNCTION__);
    if (check_vendor_module())
        return 0;
    return gVendorModule->get_camera_info(camera_id, info);
}
