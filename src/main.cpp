#include <sstream>
#include <string>
#include <unordered_map>

#include <Arduino.h>
#include <Preferences.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

#include <WiFi.h>
#include <ArtnetWifi.h>

#include "OtaUploadService.h"
#include "EffectsService.h"

#include "secrets.h"

Preferences preferences;
const char* lastEffectKey = "lastEffect";

const uint16_t PixelCount = 24;
NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> strip1(PixelCount, 25, NeoBusChannel_0);
NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> strip2(PixelCount, 27, NeoBusChannel_1);

const uint16_t ArtnetUniverse = 0;
ArtnetWifi artnet;

EffectsService effectsService = EffectsService(&strip1, &strip2);

std::unordered_map<std::string, std::string> hostnames = {

};

void startWifi() {
  Serial.print("ESP32 Base MAC Address: ");
  Serial.println(ESP.getEfuseMac());
  Serial.print("WiFi MAC Address: ");
  Serial.println(WiFi.macAddress());


  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.setHostname("bitburger-light");
//  WiFi.mode(WIFI_AP);
//  WiFi.softAP("esp32", NULL);

//  WiFi.begin("virus89.exe-24ghz", "Mangoldsalat2019");
//  WiFi.begin("annbau.freifunk.net", NULL);
//  WiFi.begin("berlin.freifunk.net", NULL);
//  WiFi.begin("WGina", "w1rs1ndd1ewg1naw1rs1ndpr1ma");
  WiFi.mode(WIFI_STA);
  WiFi.begin(HOELLENLEUCHTEN_WIFI_SSID, HOELLENLEUCHTEN_WIFI_PASSWORD);
  for (int i = 5; i <= 5; i++) {
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.println("WiFi connected");
      Serial.println(WiFi.localIP());
    }
    Serial.println("WiFi failed, retrying...");
    delay(1500);
  }
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {

  // If universe 0, start one of the pre-built effects
  if (universe == 0) {
    preferences.putUChar(lastEffectKey, data[0]);
    effectsService.startEffect(EffectsService::Effect(data[0]));
    return;
  } else if (universe == 1 || universe == 2) {
    // NOTE(Leon Handreke): Speed up somehow? Fast enough?
    effectsService.stop();

    NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> strip = universe == 1 ? strip1 : strip2;

    // NOTE(Leon Handreke): Copy-pasted from
    // https://github.com/rstephan/ArtnetWifi/blob/master/examples/ArtnetWifiNeoPixel/ArtnetWifiNeoPixel.ino
    // but dumbed down to only read a single universe. If we want to do more than 128 LEDs,
    // the somewhat complicated universe receiving logic should be pulled out into a separate
    // class.
    for (int i = 0; i < length / 4; i++) {
      if (i < strip.PixelCount())
      {
        uint8_t* pixelData = data + (i*4);
        strip.SetPixelColor(i, RgbwColor(
            pixelData[0],
            pixelData[1],
            pixelData[2],
            pixelData[3]));
      }
    }
    strip.Show();
  }


}

void _handleArtnetLoop(void *pvParameters) {
  while (true) {
    artnet.read();
    Serial.println("artnet.read()");
  }
}

void startArtnetService() {
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);

  xTaskCreate(
      _handleArtnetLoop,    // Function that should be called
      "Artnet Loop",  // Name of the task (for debugging)
      1000,            // Stack size (bytes)
      NULL,            // Parameter to pass
      1,               // Task priority
      NULL             // Task handle
  );
}

void setup()
{
  delay(100);
  Serial.begin(9600);
  Serial.println("Startup");

  strip1.Begin();
  strip1.SetBrightness(50);
  strip1.ClearTo(RgbwColor(255, 0, 0, 0));

  strip2.Begin();
  strip2.SetBrightness(50);
  strip2.ClearTo(RgbwColor(0, 255, 0, 0));

  startWifi();

  OtaUploadService otaUploadService;
  otaUploadService.start();

  // Start the last-used effect
  preferences.begin("hoelle", false);
  effectsService.startEffect(EffectsService::Effect(preferences.getUChar(lastEffectKey)));

  // Start the Artnet Service that will override
  startArtnetService();

}

void loop()
{
  //whiteOverRainbow(75, 5);
  //rainbowFade2White(3, 3, 1);
  //rainbow();

  //artnet.read();
}

