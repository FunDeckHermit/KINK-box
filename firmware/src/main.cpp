#include "AudioTools.h"
#include "AudioTools/Communication/HTTP/URLStream.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

using namespace audio_tools;

const char* URLs[] = {
    "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK_DISTORTION.mp3",
    "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK_90S.mp3"
};

URLStreamBuffered url0(WIFI_SSID, WIFI_PASS, 4096);
URLStreamBuffered url1(WIFI_SSID, WIFI_PASS, 4096);

I2SStream i2s;
VolumeStream volume(i2s);
MP3DecoderHelix mp3;

EncodedAudioStream decoder(&volume, &mp3);

StreamCopy copier0(decoder, url0, 1024);
StreamCopy copier1(decoder, url1, 1024);

volatile bool station = 0;
volatile uint32_t lastButton = 0;

void IRAM_ATTR menuISR() {
    uint32_t now = millis();

    if (now - lastButton > 200) {
        station = !station;
        lastButton = now;
    }
}


void setup() {
    pinMode(MENU_PIN, INPUT_PULLUP);
    attachInterrupt(MENU_PIN, menuISR, FALLING);

    auto cfg = i2s.defaultConfig(TX_MODE);
    cfg.pin_bck  = PIN_I2S_SCK;
    cfg.pin_ws   = PIN_I2S_FS;
    cfg.pin_data = PIN_I2S_SD;

    i2s.begin(cfg);

    volume.setVolume(0.20);

    decoder.begin();

    url0.begin(URLs[0], "audio/mpeg");
    url1.begin(URLs[1], "audio/mpeg");
}

void loop() {
    if (station)
        copier1.copy();
    else
        copier0.copy();
}
