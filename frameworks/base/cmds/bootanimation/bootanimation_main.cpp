/*
 * Copyright (C) 2007 The Android Open Source Project
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

#define LOG_TAG "BootAnimation"

#include <cutils/properties.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <utils/Log.h>
#include <utils/threads.h>

#if defined(HAVE_PTHREADS)
# include <pthread.h>
# include <sys/resource.h>
#endif

#include "BootAnimation.h"

using namespace android;

// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
#if defined(HAVE_PTHREADS)
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_DISPLAY);
#endif

    char value[PROPERTY_VALUE_MAX];
    property_get("persist.sys.nobootanimation", value, "0");
    int noBootAnimation = atoi(value);
    ALOGI_IF(noBootAnimation,  "boot animation disabled");

    property_get("persist.sys.nobootanimationwait", value, "0");
    int noBootAnimationWait = atoi(value);
    //for compatibility
    if (noBootAnimationWait == 0) {
        property_get("persist.sys.nowait_animation", value, "0");
        noBootAnimationWait = atoi(value);
    }
    ALOGI_IF(noBootAnimationWait,  "boot animation wait disabled");

    char bootsoundFile[PROPERTY_VALUE_MAX];
    float bootsoundVolume = 0.2;
    property_get("persist.sys.nobootsound", value, "0");
    int noBootSound = atoi(value);
    ALOGI_IF(noBootSound,  "boot nobootsound disabled");
    if (!noBootSound) {
        property_get("persist.sys.boosound_file", bootsoundFile, "/system/media/bootsound.mp3");
        ALOGI("bootsound_file=%s", bootsoundFile);
        property_get("persist.sys.boosound_volume", value, "0.2");
        bootsoundVolume = atof(value);
        ALOGI("bootsound_volume=%f", bootsoundVolume);
    }

    char bootmovieFile[PROPERTY_VALUE_MAX];
    property_get("persist.sys.nobootmovie", value, "0");
    int noBootMovie = atoi(value);
    ALOGI_IF(noBootMovie,  "boot nobootMovie disabled");
    if (!noBootMovie) {
        property_get("persist.sys.boomovie_file", bootmovieFile, "/data/local/bootmovie.mp4");
        ALOGI("bootsound_file=%s", bootmovieFile);
        property_get("persist.sys.boosound_volume", value, "0.2");
        bootsoundVolume = atof(value);
        ALOGI("bootsound_volume=%f", bootsoundVolume);
    }

    if (argc > 1) {
        ALOGI("bootanim_file args[1]=%s", argv[1]);
    }
    if (argc > 2) {
        ALOGI("bootsound_file args[2]=%s", argv[2]);
    }
    if (argc > 3) {
        ALOGI("boomovie_file args[3]=%s", argv[3]);
    }

    if (!noBootAnimation) {

        seteuid(1003);

        sp<ProcessState> proc(ProcessState::self());
        ProcessState::self()->startThreadPool();

        // create the boot animation object
        sp<BootAnimation> boot = new BootAnimation(
                                         noBootAnimationWait ? true : false,
                                         noBootAnimation ? NULL : (argc > 1 ? argv[1] : NULL),
                                         noBootSound ? NULL : (argc > 2 ? argv[2] : bootsoundFile),
                                         noBootMovie ? NULL : (argc > 3 ? argv[3] : bootmovieFile),
                                         bootsoundVolume);

        IPCThreadState::self()->joinThreadPool();

        seteuid(0);
        ALOGI("[BOOT] set sys.bootanim_completed");
        property_set("sys.bootanim_completed", "1");
    }
    return 0;
}
