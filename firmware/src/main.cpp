#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

using namespace audio_tools;

const char* URLs[] = {
    "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK_DISTORTION.mp3",
    "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK_90S.mp3"
};

URLStreamBuffered url(WIFI_SSID, WIFI_PASS);
I2SStream i2s;
VolumeStream volume(i2s);
MP3DecoderHelix mp3;
EncodedAudioStream decoder(&volume, &mp3);
StreamCopy copier(decoder, url);

TaskHandle_t audioTask;
int station = 0;
float volumeLevel = 0.10;

void audioTaskFunc(void*) {
    url.begin(URLs[station], "audio/mpeg");

    for (;;) {
        if (ulTaskNotifyTake(pdTRUE, 0)) {
            station = !station;
            url.end();
            url.begin(URLs[station], "audio/mpeg");
        }
        copier.copy();
    }
}

void buttonTaskFunc(void*) {
    bool last = HIGH;

    for (;;) {
        bool now = digitalRead(MENU_PIN);

        if (last && !now) {
            xTaskNotifyGive(audioTask);
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        last = now;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void updateVolume() {
    static uint32_t lastChange, pressedAt;
    int dir = !digitalRead(VOL_UP_PIN) ? 1 :
              !digitalRead(VOL_DOWN_PIN) ? -1 : 0;

    if (!dir) {
        pressedAt = 0;
        return;
    }

    uint32_t now = millis();

    if (!pressedAt) {
        pressedAt = now;
        lastChange = now - 100;
    }

    if (now - lastChange >= (now - pressedAt > 500 ? 100 : 0)) {
        volumeLevel = constrain(volumeLevel + dir * 0.01, 0.0f, 1.0f);
        volume.setVolume(volumeLevel);
        lastChange = now;
    }
}

void setup() {
    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.pin_bck = PIN_I2S_SCK;
    cfg.pin_ws = PIN_I2S_FS;
    cfg.pin_data = PIN_I2S_SD;

    pinMode(VOL_UP_PIN, INPUT_PULLUP);
    pinMode(VOL_DOWN_PIN, INPUT_PULLUP);
    pinMode(MENU_PIN, INPUT_PULLUP);

    i2s.begin(cfg);
    volume.setVolume(volumeLevel);
    decoder.begin();

    xTaskCreate(audioTaskFunc, "Audio", 4096, nullptr, 2, &audioTask);
    xTaskCreate(buttonTaskFunc, "Button", 2048, nullptr, 1, nullptr);
}

void loop() {
    updateVolume();
    vTaskDelay(pdMS_TO_TICKS(10));
}
