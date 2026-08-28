#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "Station.h" 

using namespace audio_tools;

#include <Arduino.h>
#include <WiFi.h>
#include "esp_heap_caps.h"

// ============================================================================
// THE PSRAM FORCE HACK: Redirecting Network Core Heap Demands to Octal PSRAM
// ============================================================================
extern "C" {
    // Standard memory hooks
    void *__real_malloc(size_t size);
    void *__wrap_malloc(size_t size) {
        if (size > 48) {
            void *ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            if (ptr) return ptr;
        }
        return heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    }

    void *__real_calloc(size_t n, size_t size);
    void *__wrap_calloc(size_t n, size_t size) {
        size_t total = n * size;
        if (total > 48) {
            void *ptr = heap_caps_malloc(total, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            if (ptr) { memset(ptr, 0, total); return ptr; }
        }
        void *ptr = heap_caps_malloc(total, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (ptr) memset(ptr, 0, total);
        return ptr;
    }

    void *mbedtls_calloc(size_t n, size_t size) { return __wrap_calloc(n, size); }
    void mbedtls_free(void *ptr) { heap_caps_free(ptr); }

    // FREERTOS TASK HIJACK HOOK
    // Intercepts Phil Schatzmann's internal task generation calls
    BaseType_t __real_xTaskCreate(TaskFunction_t pvTaskCode, const char * const pcName, const uint32_t usStackDepth, void * const pvParameters, UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask);
    BaseType_t __wrap_xTaskCreate(TaskFunction_t pvTaskCode, const char * const pcName, const uint32_t usStackDepth, void * const pvParameters, UBaseType_t uxPriority, TaskHandle_t * const pxCreatedTask) {
        
        // Check if this is an audio library stream buffering task
        if (pcName != NULL && (strstr(pcName, "Audio") != NULL || strstr(pcName, "Buffer") != NULL || strstr(pcName, "URL") != NULL)) {
            // Force Task Stack and TCB completely into external Octal PSRAM
            return xTaskCreatePinnedToCoreWithCaps(
                pvTaskCode, pcName, usStackDepth, pvParameters, 
                uxPriority, pxCreatedTask, 
                0, // Assign background processing tasks safely to Core 0
                MALLOC_CAP_SPIRAM
            );
        }
        
        // Fall back to standard creation for core system interrupts/WiFi peripherals
        return __real_xTaskCreate(pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask);
    }
}


// Macro to print current free internal SRAM and external PSRAM
#define PRINT_RAM(label) do { \
    Serial.printf("=== RAM: %s ===\n", label); \
    Serial.printf("  Internal SRAM Free : %u Bytes\n", ESP.getFreeHeap()); \
    Serial.printf("  External PSRAM Free: %u Bytes\n\n", ESP.getFreePsram()); \
} while(0)


// Unified array defining your radio station lineup
RadioStation stations[] = {
    {"KINK BASE",       "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK.mp3",         nullptr, nullptr},
    {"KINK DNA",        "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK_DNA.mp3",         nullptr, nullptr},
    {"KINK DISTORTION", "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK_DISTORTION.mp3",  nullptr, nullptr},
    {"KINK 90S",        "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK_90S.mp3",         nullptr, nullptr}
};
const int STATIONS_COUNT = sizeof(stations) / sizeof(stations[0]);

// 2. ONLY ONE buffer task wrapper shared among all streams to save SRAM
URLStreamBuffered shared_buffer(4096); 

I2SStream i2s;
VolumeStream volume(i2s);
MP3DecoderHelix mp3;
EncodedAudioStream decoder(&volume, &mp3);
StreamCopy copier(decoder, shared_buffer, 1024);

volatile int station_idx = 0;
volatile uint32_t lastButton = 0;

void IRAM_ATTR menuISR() {
    if (millis() - lastButton > 200) {
        station_idx = (station_idx + 1) % STATIONS_COUNT;
        lastButton = millis();
    }
}


void setup() {
    Serial.begin(115200);
    AudioLogger::instance().begin(Serial, AudioLogger::Warning); 
    pinMode(MENU_PIN, INPUT_PULLUP);
    attachInterrupt(MENU_PIN, menuISR, FALLING);

    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.pin_bck = PIN_I2S_SCK; cfg.pin_ws = PIN_I2S_FS; cfg.pin_data = PIN_I2S_SD;
    i2s.begin(cfg);
    volume.setVolume(0.20);
    decoder.begin();

    PRINT_RAM("BEFORE URL BEGIN");
    for (int i = 0; i < STATIONS_COUNT; i++) {
        stations[i].streamObj = new URLStreamBuffered(WIFI_SSID, WIFI_PASS, 4096);
        stations[i].copierObj = new StreamCopy(decoder, *stations[i].streamObj, 1024);
        
        stations[i].streamObj->begin(stations[i].url, "audio/mpeg");
        Serial.printf("Initialized [%s] -> %s\n", stations[i].name, stations[i].url);
    }
    PRINT_RAM("AFTER URL BEGIN");
}

void loop() {
    stations[station_idx]->play();
}
