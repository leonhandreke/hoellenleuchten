#include <sstream>
#include <string>

#include <Arduino.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include <ArtnetWifi.h>

#include "OtaUploadService.h"
#include "EffectsService.h"

const uint16_t PixelCount = 240;
const uint16_t PixelPin = 26;

NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod> strip(PixelCount, PixelPin);

const uint16_t ArtnetUniverse = 0;
ArtnetWifi artnet;

EffectsService effectsService = EffectsService(&strip);

void startWifi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("esp32", NULL);
  //WiFi.mode(WIFI_STA);
  //WiFi.begin("virus89.exe-24ghz", "Mangoldsalat2019");
  //WiFi.begin("annbau.freifunk.net", NULL);
  //WiFi.begin("berlin.freifunk.net", NULL);
  //WiFi.begin("WGina", "w1rs1ndd1ewg1naw1rs1ndpr1ma");
  //if (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //  Serial.println("WiFi failed");
  //}
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {

  // If universe 0, start one of the pre-built effects
  if (universe == 0) {
    effectsService.startEffect(EffectsService::Effect(data[0]));
    return;
  } else {
    effectsService.stop();
  }

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

  strip.Begin();
  strip.SetBrightness(50);
  // Start with just normal white light
  strip.ClearTo(RgbwColor(0, 0, 0, 255));

  startWifi();

  OtaUploadService otaUploadService;
  otaUploadService.start();

  startArtnetService();
}

void loop()
{
  //whiteOverRainbow(75, 5);
  //rainbowFade2White(3, 3, 1);
  //rainbow();

  //artnet.read();
}

