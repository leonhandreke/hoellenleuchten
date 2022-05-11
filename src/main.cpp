#include <sstream>
#include <string>

#include <Arduino.h>
#include <ESPmDNS.h>


#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include <ArtnetWifi.h>

// No MDNS advertising of the OTA update mechanism
#include <ArduinoOTA.h>

const uint16_t PixelCount = 20;
const uint16_t PixelPin = 26;

NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod> strip(PixelCount, PixelPin);
NeoGamma<NeoGammaTableMethod> colorGamma;


const uint16_t ArtnetUniverse = 0;
ArtnetWifi artnet;


std::string getName() {
  std::stringstream mdnsName;
  mdnsName << "hoellenleuchten-" << std::hex << ESP.getEfuseMac();
  return mdnsName.str();
}

void startWifi() {
  //WiFi.mode(WIFI_AP);
  //WiFi.softAP(getName().c_str(), NULL);
  WiFi.mode(WIFI_STA);
  WiFi.begin("virus89.exe-24ghz", "Mangoldsalat2019");
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi failed");
  }
  Serial.println("WiFi connected");
}

void startMdns() {
  Serial.println(getName().c_str());
  MDNS.begin(getName().c_str());
}

void startOtaUploadService() {
  ArduinoOTA
    .onStart([]() {
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

  ArduinoOTA.setPassword("admin");
  ArduinoOTA.begin();
}


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  // NOTE(Leon Handreke): Copy-pasted from
  // https://github.com/rstephan/ArtnetWifi/blob/master/examples/ArtnetWifiNeoPixel/ArtnetWifiNeoPixel.ino
  // but dumbed down to only read a single universe. If we want to do more than 128 LEDs,
  // the somewhat complicated universe receiving logic should be pulled out into a separate
  // class.
  for (int i = 0; i < length / 3; i++) {
    if (i < strip.PixelCount())
    {
      strip.SetPixelColor(i, RgbwColor(data[i * 3], data[i * 3 + 1], data[i * 3 + 2], 0));
    }
  }

  strip.Show();
}

void setup()
{
  Serial.begin(9600);

  startWifi();
  startMdns();
  startOtaUploadService();

  strip.Begin();
  strip.SetBrightness(50);

  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);
}

// Copy-pasta from NeoPixel example
void whiteOverRainbow(int whiteSpeed, int whiteLength) {

  if(whiteLength >= strip.PixelCount()) whiteLength = strip.PixelCount() - 1;

  int      head          = whiteLength - 1;
  int      tail          = 0;
  int      loops         = 3;
  int      loopNum       = 0;
  uint32_t lastTime      = millis();
  uint32_t firstPixelHue = 0;

  for(;;) { // Repeat forever (or until a 'break' or 'return')
    for(int i=0; i<strip.PixelCount(); i++) {  // For each pixel in strip...
      if(((i >= tail) && (i <= head)) ||      //  If between head & tail...
         ((tail > head) && ((i >= tail) || (i <= head)))) {
        strip.SetPixelColor(i, RgbwColor(0, 0, 0, 255)); // Set white
      } else {                                             // else set rainbow
        float pixelHue = firstPixelHue + (i * 1.0 / strip.PixelCount());
        strip.SetPixelColor(i, colorGamma.Correct(RgbColor(HsbColor(pixelHue, 1.0, 1.0))));
      }
    }

    strip.Show(); // Update strip with new contents
    // There's no delay here, it just runs full-tilt until the timer and
    // counter combination below runs out.

    firstPixelHue += 40; // Advance just a little along the color wheel

    if((millis() - lastTime) > whiteSpeed) { // Time to update head/tail?
      if(++head >= strip.PixelCount()) {      // Advance head, wrap around
        head = 0;
        if(++loopNum >= loops) return;
      }
      if(++tail >= strip.PixelCount()) {      // Advance tail, wrap around
        tail = 0;
      }
      lastTime = millis();                   // Save time of last movement
    }
  }
}


void rainbow() {
  int rainbowLoops = 5;

  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for(uint32_t firstPixelHue = 0; firstPixelHue < 65536;
    firstPixelHue += 256) {

    for(int i=0; i<strip.PixelCount(); i++) { // For each pixel in strip...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip.PixelCount());

      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      //strip.SetPixelColor(i, colorGamma.Correct(RgbColor(
      //        HsbColor(pixelHue / float(65536L), 1.0, 1.0))));
      strip.SetPixelColor(i,
              HsbColor(pixelHue / float(65536L), 1.0, 1.0));
    }

    strip.Show();
    delay(5);


  }
  Serial.println("fade2white done");
}


void loop()
{
  ArduinoOTA.handle();
  //whiteOverRainbow(75, 5);
  //rainbowFade2White(3, 3, 1);
  rainbow();

  //artnet.read();
}

