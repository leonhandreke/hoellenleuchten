#include <sstream>
#include <string>
#include <unordered_map>

#include <Arduino.h>
#include <Preferences.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

#include <WiFi.h>
#include <ArtnetWifi.h>
#include <ArduinoOTA.h>

#include "EffectsService.h"

#include "secrets.h"

Preferences preferences;
const char* lastEffectKey = "lastEffect";

NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> *strip1;
NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> *strip2;

ArtnetWifi artnet;

EffectsService *effectsService;
NeoGamma<NeoGammaTableMethod> colorGamma;

TaskHandle_t artnetTaskHandle;

typedef struct {
  std::string name;
  uint16_t stripLength;
} DeviceData;

std::unordered_map<std::string, DeviceData> devices = {
    {"94:B5:55:27:50:F4",{"bitburger-light-1", 25}},
    {"94:B5:55:27:55:CC",{"bitburger-light-2", 25}},
    {"94:B5:55:26:43:EC",{"bitburger-light-3", 25}},
    {"94:B5:55:2D:4D:88",{"bitburger-light-4", 25}},
    {"94:B5:55:2D:37:C4",{"bitburger-light-5", 25}},
    {"94:B5:55:26:7E:9C",{"bitburger-light-6", 25}},
    {"94:B5:55:2D:39:78",{"bitburger-light-7", 25}},
    {"94:B5:55:26:56:68",{"bitburger-light-8", 25}},
    {"94:B5:55:26:60:44",{"bitburger-light-9", 25}},
    {"94:B5:55:26:48:D4",{"bitburger-light-10", 25}},
    {"94:B5:55:26:9D:DC",{"bitburger-light-11", 25}},
    {"94:B5:55:2D:4A:C4",{"bitburger-light-12", 25}},
    {"94:B5:55:26:91:1C",{"bitburger-light-13", 25}},
    {"94:B5:55:27:5E:F8",{"bitburger-light-14", 25}},
    {"94:B5:55:2D:41:A8",{"bitburger-light-15", 60}},
    {"94:B5:55:26:35:D8",{"bitburger-light-16", 25}},
    {"94:B5:55:25:39:0C",{"bitburger-light-17", 25}},
    {"94:B5:55:26:34:E4",{"bitburger-light-18", 25}},
    {"94:B5:55:2C:3F:CC",{"bitburger-light-19", 25}},
    {"94:B5:55:27:49:E4",{"bitburger-light-20", 25}},
};


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  Serial.println("Got DMX");

  // If universe 0, start one of the pre-built effects
  if (universe == 42) {
    preferences.putUChar(lastEffectKey, data[0]);
    effectsService->startEffect(EffectsService::Effect(data[0]));
    return;
  } else if (universe == 0 || universe == 1 || universe == 2) {
    // NOTE(Leon Handreke): Speed up somehow? Fast enough?
    effectsService->stop();

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
        strip.SetPixelColor(i, colorGamma.Correct(RgbwColor(
            pixelData[0],
            pixelData[1],
            pixelData[2],
            pixelData[3])));
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
      &artnetTaskHandle             // Task handle
  );
}

void stopArtnetService() {
  Serial.println("Stopping Artnet service");
  if (artnetTaskHandle != NULL) {
    vTaskDelete(artnetTaskHandle);
  }
}


void _handleOtaUploadLoop(void *pvParameters) {
  while (true) {
    ArduinoOTA.handle();
    vTaskDelay(500);
  }
}

void startOtaUploadService() {
  ArduinoOTA
      .onStart([]() {
        effectsService.stop();
        stopArtnetService();

        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
        else // U_SPIFFS
          type = "filesystem";

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
      });

  ArduinoOTA.setHostname(WiFi.getHostname());
  ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();

  xTaskCreate(
      _handleOtaUploadLoop,    // Function that should be called
      "Handle OTA Upload",  // Name of the task (for debugging)
      100000,            // Stack size (bytes)
      NULL,            // Parameter to pass
      10,               // Task priority
      NULL             // Task handle
  );
}

DeviceData myDevice() {
  auto deviceSearch = devices.find(std::string(WiFi.macAddress().c_str()));
  if (deviceSearch != devices.end()) {
    return deviceSearch->second;
  }
  return {"bitburger-light-unknown", 25};
}

void startWifi() {
  Serial.print("ESP32 Base MAC Address: ");
  Serial.println(ESP.getEfuseMac());
  Serial.print("WiFi MAC Address: ");
  Serial.println(WiFi.macAddress());

  auto hostname = myDevice().name;

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(WIFI_PS_NONE);

  // Set hostname so that they're easier to identify in the dashboard
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



void setup()
{
  Serial.begin(9600);
  // Give the other side some time to connect
  delay(100);
  Serial.println("Startup");

  delay(100);

  auto pixelCount = myDevice().stripLength;
  strip1 = new NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod>(pixelCount, 25, NeoBusChannel_1);
  strip2 = new NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod>(pixelCount, 27, NeoBusChannel_0);

  strip1->Begin();
  strip1->SetBrightness(255);
  strip1->ClearTo(RgbwColor(255, 0, 0, 0));

  strip2->Begin();
  strip2->SetBrightness(255);
  strip2->ClearTo(RgbwColor(0, 255, 0, 0));

  strip2->Show();
  strip1->Show();
  strip2->Show();
  strip1->Show();


  startWifi();
  startOtaUploadService();

  // Start the last-used effect
  preferences.begin("hoelle", false);
  // effectsService.startEffect(EffectsService::Effect(preferences.getUChar(lastEffectKey)));
  effectsService = new EffectsService(strip1, strip2);
  effectsService->startEffect(EffectsService::Effect::RAINBOW);

  // Start the Artnet Service that will override
  startArtnetService();
}

void loop()
{
  // Free up the CPU core
  //vTaskDelete(NULL);
  Serial.printf("RSSI: %d dBm Channel: %d\n", WiFi.RSSI(), WiFi.channel());
  vTaskDelay(10000);

  //whiteOverRainbow(75, 5);
  //rainbowFade2White(3, 3, 1);
  //rainbow();

  //artnet.read();
}

