#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "Station.h" 

using namespace audio_tools;

// Force Mbed TLS allocations directly into Octal PSRAM to save internal SRAM
extern "C" {
    void *mbedtls_calloc(size_t n, size_t size) {
        void *ptr = heap_caps_malloc(n * size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!ptr) ptr = heap_caps_malloc(n * size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (ptr) memset(ptr, 0, n * size);
        return ptr;
    }
    void mbedtls_free(void *ptr) { heap_caps_free(ptr); }
}

// Macro to print current free internal SRAM and external PSRAM
#define PRINT_RAM(label) do { \
    Serial.printf("=== RAM: %s ===\n", label); \
    Serial.printf("  Internal SRAM Free : %u Bytes\n", ESP.getFreeHeap()); \
    Serial.printf("  External PSRAM Free: %u Bytes\n\n", ESP.getFreePsram()); \
} while(0)


// Unified array defining your radio station lineup
RadioStation stations[] = {
    //{"KINK BASE",       "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK.mp3",         nullptr, nullptr},
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
        station_idx = (station_idx + 1) % 3;
        lastButton = millis();
    }
}


void setup() {
    Serial.begin(115200);
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
