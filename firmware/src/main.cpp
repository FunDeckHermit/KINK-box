#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>

#include "AudioTools.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "AudioTools/Disk/AudioSourceURL.h"
#include "AudioTools/Communication/AudioHttp.h"


const char *urls[] = {
 "https://playerservices.streamtheworld.com/api/livestream-redirect/KINK_DISTORTION.mp3"
};

URLStream urlStream(WIFI_SSID, WIFI_PASS);
AudioSourceURL source(urlStream, urls, "audio/mp3");
I2SStream i2s;
VolumeStream volume(i2s);
MP3DecoderHelix decoder;
AudioPlayer player(source, i2s, decoder);

void setup() {
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);


  // setup output
  auto cfg = i2s.defaultConfig(TX_MODE);
  cfg.pin_bck = 14; cfg.pin_data = 16; cfg.pin_ws = 15;
  i2s.begin(cfg);

  player.setVolume(0.10);
  
  // setup player
  player.begin();
}

void loop() {
  //updateVolume(); // remove comments to activate volume control
  //updatePosition();  // remove comments to activate position control
  player.copy();
}