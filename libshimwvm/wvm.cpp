/*
 * Copyright (C) 2016 The CyanogenMod Project
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

#include <android/log.h>
#include <media/IMediaSource.h>
#include <stdlib.h>

/* MediaBufferGroup::MediaBufferGroup */
extern "C" int _ZN7android16MediaBufferGroupC1Ej(size_t);
extern "C" int _ZN7android16MediaBufferGroupC1Ev() {
    return _ZN7android16MediaBufferGroupC1Ej(0);
}

/* MediaBufferGroup::acquire_buffer */
extern "C" android::status_t _ZN7android16MediaBufferGroup14acquire_bufferEPPNS_11MediaBufferEbj(android::MediaBuffer**, bool, size_t);
extern "C" android::status_t _ZN7android16MediaBufferGroup14acquire_bufferEPPNS_11MediaBufferEb(android::MediaBuffer **out, bool nonBlocking) {
    __android_log_print(ANDROID_LOG_ERROR, "MEDIABUFFER RANGE_LENGTH: ", "%d", (**out).range_length()); 
    return _ZN7android16MediaBufferGroup14acquire_bufferEPPNS_11MediaBufferEbj(out, nonBlocking, (**out).range_length());
}

/* IMediaSource::ReadOptions::getSeekTo */
extern "C" bool _ZNK7android12IMediaSource11ReadOptions9getSeekToEPxPNS1_8SeekModeE(int64_t*, android::IMediaSource::ReadOptions::SeekMode*);
extern "C" bool _ZNK7android11MediaSource11ReadOptions9getSeekToEPxPNS1_8SeekModeE(int64_t *time_us, android::IMediaSource::ReadOptions::SeekMode *mode) {
    return _ZNK7android12IMediaSource11ReadOptions9getSeekToEPxPNS1_8SeekModeE(time_us, mode);
}

/* IMediaSource::ReadOptions::getNonBlocking */
extern "C" bool _ZNK7android12IMediaSource11ReadOptions14getNonBlockingEv();
extern "C" bool _ZNK7android11MediaSource11ReadOptions14getNonBlockingEv() {
    return _ZNK7android12IMediaSource11ReadOptions14getNonBlockingEv();
}
