#pragma once

#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

// Reference global variables owned by main.cpp
extern volatile int station_idx;
extern int active_playing_idx;
extern audio_tools::MP3DecoderHelix mp3;

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
            uint8_t chunkBuffer[256]; // Correctly typed chunk buffer block
            size_t bytesToRead = (availableBytes > 256) ? 256 : availableBytes;
            int bytesRead = streamObj->readBytes(chunkBuffer, bytesToRead);
            if (bytesRead > 0) slidingBuffer->write(chunkBuffer, bytesRead);
        }
    }

    void play() {
        // ENCAPSULATED ANTI-FAST-FORWARD TRANSITION ENGINE
        if (active_playing_idx != myIdx) {
            active_playing_idx = myIdx;
            
            // FIX: Re-calling .begin() instantly resets the ring pointers to 0 
            // without running a heavy CPU while-loop, eliminating I2S starvation.
            if (slidingBuffer != nullptr) {
                slidingBuffer->begin(); 
            }
            
            // Clear out historical bit fragments inside the Helix codec parser
            mp3.end();
            mp3.begin();
            
            Serial.printf("Switched clean decoding target to: %s\n", name);
            
            // Give Core 0 a small 50ms window to inject fresh, aligned audio data
            delay(50); 
        }
        
        if (copierObj != nullptr) copierObj->copy(); 
    }

    RadioStation* operator->() { return this; }
} RadioStation;
