#pragma once

#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

// Reference global variables managed inside main.cpp
extern volatile int station_idx;
extern int active_playing_idx;
extern audio_tools::EncodedAudioStream decoder; 
extern audio_tools::VolumeStream volume; 
extern TaskHandle_t pumpTaskHandle; // Use task handles for real suspension control

typedef struct RadioStation {
    const char* name;
    const char* url;
    audio_tools::URLStream* streamObj;
    audio_tools::RingBufferStream* slidingBuffer; 
    audio_tools::StreamCopy* copierObj;
    int myIdx;

    void init(int idx, size_t bufferSize, audio_tools::EncodedAudioStream* targetDecoder) {
        myIdx = idx;
        streamObj = new audio_tools::URLStream();
        slidingBuffer = new audio_tools::RingBufferStream(bufferSize);
        copierObj = new audio_tools::StreamCopy(*targetDecoder, *slidingBuffer, 1024);
    }

    void pump() {
        if (streamObj == nullptr || slidingBuffer == nullptr) return;
        int availableBytes = streamObj->available();
        if (availableBytes > 0) {
            uint8_t chunkBuffer[256]; 
            size_t bytesToRead = (availableBytes > 256) ? 256 : availableBytes;
            
            int bytesRead = streamObj->readBytes(chunkBuffer, bytesToRead);
            if (bytesRead > 0) {
                slidingBuffer->write(chunkBuffer, bytesRead);
            }
        }
    }

    void play() {
        if (slidingBuffer == nullptr || copierObj == nullptr) return;

        // BULLETPROOF FREE-RTOS TASK SUSPENSION SWITCH ENGINE
        if (active_playing_idx != myIdx) {
            
            // 1. Physically freeze Core 0's pump thread at the RTOS scheduler level
            if (pumpTaskHandle != NULL) {
                vTaskSuspend(pumpTaskHandle);
            }
            
            active_playing_idx = myIdx;
            
            // 2. Mute audio output instantly to mask initialization changes
            float targetVolume = volume.volume();
            volume.setVolume(0.0);
            
            // 3. Reset buffer pointer offsets back to zero alignment
            slidingBuffer->begin(); 
            
            // 4. Safely reset decoder structures while Core 0 is completely frozen
            decoder.end();
            decoder.begin();
            
            Serial.printf("Switched clean decoding target to: %s\n", name);
            
            // 5. Resume the network pump task safely
            if (pumpTaskHandle != NULL) {
                vTaskResume(pumpTaskHandle);
            }
            
            // 6. Give Core 0 a small window to inject a clean frame stream before restoring sound
            delay(50); 
            volume.setVolume(targetVolume);
        }
        
        copierObj->copy(); 
    }

    RadioStation* operator->() { return this; }
} RadioStation;
