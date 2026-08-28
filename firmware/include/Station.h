#pragma once

#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

// Reference global variables owned by main.cpp
extern volatile int station_idx;
extern int active_playing_idx;
extern audio_tools::MP3DecoderHelix mp3;
extern audio_tools::VolumeStream volume; // Intercept volume stream to hide switch noise

typedef struct RadioStation {
    const char* name;
    const char* url;
    audio_tools::URLStream* streamObj;
    audio_tools::RingBufferStream* slidingBuffer; 
    audio_tools::StreamCopy* copierObj;
    int myIdx;

    void init(int idx, size_t bufferSize, audio_tools::EncodedAudioStream* decoder) {
        myIdx = idx;
        streamObj = new audio_tools::URLStream();
        slidingBuffer = new audio_tools::RingBufferStream(bufferSize);
        copierObj = new audio_tools::StreamCopy(*decoder, *slidingBuffer, 1024);
    }

    void pump() {
        if (streamObj == nullptr || slidingBuffer == nullptr) return;
        int availableBytes = streamObj->available();
        if (availableBytes > 0) {
            uint8_t chunkBuffer[256]; 
            size_t bytesToRead = (availableBytes > 256) ? 256 : availableBytes;
            int bytesRead = streamObj->readBytes(chunkBuffer, bytesToRead);
            if (bytesRead > 0) slidingBuffer->write(chunkBuffer, bytesRead);
        }
    }

    void play() {
        if (slidingBuffer == nullptr || copierObj == nullptr) return;

        // SEAMLESS CUTOVER TRANSITION ENGINE
        if (active_playing_idx != myIdx) {
            active_playing_idx = myIdx;
            
            // 1. Mute audio output instantly to mask stale frame residue
            float targetVolume = volume.volume();
            volume.setVolume(0.0);
            
            // 2. Clear out the old buffer index alignment entirely
            slidingBuffer->begin(); 
            
            // 3. Purge the Helix bitstream decoding history cache
            mp3.end();
            mp3.begin();
            
            Serial.printf("Switched clean decoding target to: %s\n", name);
            
            // 4. Give Core 0 a brief 60ms window to buffer fresh, aligned frames
            delay(60); 
            
            // 5. Force copy operations to clear the active StreamCopy cache blocks
            copierObj->copy();
            copierObj->copy();
            copierObj->copy();
            
            // 6. Restore original volume level for clean audio delivery
            volume.setVolume(targetVolume);
        }
        
        copierObj->copy(); 
    }

    RadioStation* operator->() { return this; }
} RadioStation;
