#include <Arduino.h>
#include <WiFi.h>
#include "esp_heap_caps.h"
#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "Station.h" 

using namespace audio_tools;
const size_t SLIDING_WINDOW_SIZE = 1024 * 32; 

// Memory wraps redirecting TLS, tasks, and sockets to Octal PSRAM
extern "C" {
    void *__real_malloc(size_t s); void *__wrap_malloc(size_t s) { if (s > 48) { void *p = heap_caps_malloc(s, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT); if (p) return p; } return heap_caps_malloc(s, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT); }
    void *__real_calloc(size_t n, size_t s); void *__wrap_calloc(size_t n, size_t s) { size_t t = n * s; if (t > 48) { void *p = heap_caps_malloc(t, MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT); if (p) { memset(p, 0, t); return p; } } void *p = heap_caps_malloc(t, MALLOC_CAP_INTERNAL|MALLOC_CAP_8BIT); if (p) memset(p, 0, t); return p; }
    void *mbedtls_calloc(size_t n, size_t s) { return __wrap_calloc(n, s); } void mbedtls_free(void *p) { heap_caps_free(p); }
    BaseType_t __real_xTaskCreate(TaskFunction_t c, const char *n, uint32_t d, void *p, UBaseType_t pr, TaskHandle_t *t);
    BaseType_t __wrap_xTaskCreate(TaskFunction_t c, const char *n, uint32_t d, void *p, UBaseType_t pr, TaskHandle_t *t) {
        if (n != NULL && (strstr(n, "Audio") != NULL || strstr(n, "Buffer") != NULL || strstr(n, "URL") != NULL || strstr(n, "Pump") != NULL)) {
            return xTaskCreatePinnedToCoreWithCaps(c, n, d, p, pr, t, 0, MALLOC_CAP_SPIRAM);
        }
        return __real_xTaskCreate(c, n, d, p, pr, t);
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

I2SStream i2s;
VolumeStream volume(i2s);
MP3DecoderHelix mp3;
EncodedAudioStream decoder(&volume, &mp3);

volatile int station_idx = 0;
int active_playing_idx = -1; 
volatile uint32_t lastButton = 0;
TaskHandle_t pumpTaskHandle = NULL; 


void IRAM_ATTR menuISR() {
    uint32_t now = millis();
    if (now - lastButton > 250) {
        station_idx = (station_idx + 1) % STATIONS_COUNT;
        lastButton = now;
    }
}

void networkPumpTask(void *p) {
    while (true) {
        if (WiFi.status() == WL_CONNECTED) { 
            for (int i = 0; i < STATIONS_COUNT; i++) {
                if (stations[i].streamObj != nullptr && stations[i].slidingBuffer != nullptr) {
                    stations[i].pump();
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1)); 
    }
}


void delayedLoaderTask(void *p) {
    vTaskDelay(pdMS_TO_TICKS(2000));
    Serial.println("[LOADER] Initializing secondary radio channels in background...");
    for (int i = 1; i < STATIONS_COUNT; i++) {
        stations[i].init(i, SLIDING_WINDOW_SIZE, &decoder);
        stations[i].streamObj->begin(stations[i].url, "audio/mpeg");
        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
    vTaskDelete(NULL); 
}

void setup() {
    Serial.begin(115200); delay(1000);
    AudioLogger::instance().begin(Serial, AudioLogger::Warning); 
    pinMode(MENU_PIN, INPUT_PULLUP); attachInterrupt(MENU_PIN, menuISR, FALLING);

    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.pin_bck = PIN_I2S_SCK; cfg.pin_ws = PIN_I2S_FS; cfg.pin_data = PIN_I2S_SD;
    cfg.buffer_size = 512; cfg.buffer_count = 4;
    i2s.begin(cfg); volume.setVolume(0.20); decoder.begin();

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nNetwork Ready!");

    stations[0].init(0, SLIDING_WINDOW_SIZE, &decoder);
    stations[0].streamObj->begin(stations[0].url, "audio/mpeg");
    Serial.printf("[INIT] Immediate Online Buffer for Main Station: %s\n", stations[0].name);

    xTaskCreatePinnedToCore(networkPumpTask, "NetPump", 4096, NULL, 1, &pumpTaskHandle, 0);
    xTaskCreatePinnedToCore(delayedLoaderTask, "DelayedLoad", 16384, NULL, 1, NULL, 0);
}

void loop() {
    stations[station_idx]->play();
}