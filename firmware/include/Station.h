#pragma once

#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"

typedef struct RadioStation {
    const char* name;
    const char* url;
    audio_tools::URLStreamBuffered* streamObj;
    audio_tools::StreamCopy* copierObj;

    void play() {
        if (copierObj != nullptr) {
            copierObj->copy();
        }
    }

    // ⚡ THE TRICK: Overload the arrow operator to point back to itself
    RadioStation* operator->() {
        return this;
    }
} RadioStation;
