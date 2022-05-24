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

const uint16_t PixelCount = 25;
NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> strip1(PixelCount, 25, NeoBusChannel_1);
NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> strip2(PixelCount, 27, NeoBusChannel_0);

const uint16_t ArtnetUniverse = 0;
ArtnetWifi artnet;

EffectsService effectsService = EffectsService(&strip1, &strip2);

std::unordered_map<std::string, std::string> hostnames = {
    {"94:B5:55:27:50:F4","bitburger-light-1"},
    {"94:B5:55:27:55:CC","bitburger-light-2"},
    {"94:B5:55:26:43:EC","bitburger-light-3"},
    {"94:B5:55:2D:4D:88","bitburger-light-4"},
    {"94:B5:55:2D:37:C4","bitburger-light-5"},
    {"94:B5:55:26:7E:9C","bitburger-light-6"},
    {"94:B5:55:2D:39:78","bitburger-light-7"},
    {"94:B5:55:26:56:68","bitburger-light-8"},
    {"94:B5:55:26:60:44","bitburger-light-9"},
    {"94:B5:55:26:48:D4","bitburger-light-10"},
    {"94:B5:55:26:9D:DC","bitburger-light-11"},
    {"94:B5:55:2D:4A:C4","bitburger-light-12"},
    {"94:B5:55:26:91:1C","bitburger-light-13"},
    {"94:B5:55:27:5E:F8","bitburger-light-14"},
    {"94:B5:55:2D:41:A8","bitburger-light-15"},
    {"94:B5:55:26:35:D8","bitburger-light-16"},
    {"94:B5:55:25:39:0C","bitburger-light-17"},
    {"94:B5:55:26:34:E4","bitburger-light-18"},
    {"94:B5:55:2C:3F:CC","bitburger-light-19"},
    {"94:B5:55:27:49:E4","bitburger-light-20"},
};

void startWifi() {
  Serial.print("ESP32 Base MAC Address: ");
  Serial.println(ESP.getEfuseMac());
  Serial.print("WiFi MAC Address: ");
  Serial.println(WiFi.macAddress());


  WiFi.mode(WIFI_STA);
  // Set hostname so that they're easier to identify in the dashboard
  std::string hostname = "bitburger-light";
  auto hostnameSearch = hostnames.find(std::string(WiFi.macAddress().c_str()));
  if (hostnameSearch != hostnames.end()) {
    hostname = hostnameSearch->second;
  }
  // Apparently required to get setHostname to work due to a bug
  //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  WiFi.config(((u32_t)0x0UL),((u32_t)0x0UL),((u32_t)0x0UL));
  WiFi.setHostname(hostname.c_str());
//  WiFi.mode(WIFI_AP);
//  WiFi.softAP("esp32", NULL);

//  WiFi.begin("virus89.exe-24ghz", "Mangoldsalat2019");
//  WiFi.begin("annbau.freifunk.net", NULL);
//  WiFi.begin("berlin.freifunk.net", NULL);
//  WiFi.begin("WGina", "w1rs1ndd1ewg1naw1rs1ndpr1ma");
  WiFi.begin(HOELLENLEUCHTEN_WIFI_SSID, HOELLENLEUCHTEN_WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(HOELLENLEUCHTEN_WIFI_SSID);
  Serial.print(" as ");
  Serial.println(hostname.c_str());

  for (int i = 5; i <= 5; i++) {
    if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      Serial.print("WiFi connected: ");
      Serial.println(WiFi.localIP());
      return;
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
    // TODO(Leon Handreke): Better way to yield? or disable WDT?
    vTaskDelay(1);
  }
}

void startArtnetService() {
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);

  xTaskCreate(
      _handleArtnetLoop,    // Function that should be called
      "Artnet Loop",  // Name of the task (for debugging)
      10000,            // Stack size (bytes)
      NULL,            // Parameter to pass
      1,               // Task priority
      NULL             // Task handle
  );
}

void setup()
{
  Serial.begin(9600);
  // Give the other side some time to connect
  delay(100);
  Serial.println("Startup");

  delay(100);

  strip1.Begin();
  strip1.SetBrightness(50);
  strip1.ClearTo(RgbwColor(255, 0, 0, 0));
  strip1.Show();

  strip2.Begin();
  strip2.SetBrightness(50);
  strip2.ClearTo(RgbwColor(0, 255, 0, 0));
  strip2.Show();

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
  // Free up the CPU core
  vTaskDelete(NULL);

  //whiteOverRainbow(75, 5);
  //rainbowFade2White(3, 3, 1);
  //rainbow();

  //artnet.read();
}

