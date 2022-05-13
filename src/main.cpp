#include <sstream>
#include <string>

#include <Arduino.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

#include <WiFi.h>
#include <WiFiUdp.h>

#include "OtaUploadService.h"

const uint16_t PixelCount = 240;
const uint16_t PixelPin = 26;

NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod> strip(PixelCount, PixelPin);
NeoGamma<NeoGammaTableMethod> colorGamma;


const uint16_t ArtnetUniverse = 0;

void startWifi() {
  //WiFi.mode(WIFI_AP);
  //WiFi.softAP(getName().c_str(), NULL);
  WiFi.mode(WIFI_STA);
  //WiFi.begin("virus89.exe-24ghz", "Mangoldsalat2019");
  WiFi.begin("annbau.freifunk.net", NULL);
  //WiFi.begin("berlin.freifunk.net", NULL);
  WiFi.begin("WGina", "w1rs1ndd1ewg1naw1rs1ndpr1ma");
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi failed");
  }
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  // NOTE(Leon Handreke): Copy-pasted from
  // https://github.com/rstephan/ArtnetWifi/blob/master/examples/ArtnetWifiNeoPixel/ArtnetWifiNeoPixel.ino
  // but dumbed down to only read a single universe. If we want to do more than 128 LEDs,
  // the somewhat complicated universe receiving logic should be pulled out into a separate
  // class.
  for (int i = 0; i < length / 4; i++) {
    if (i < strip.PixelCount())
    {
      strip.SetPixelColor(i, RgbwColor(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i*4+3]));
      //Serial.println("%i %d %d %d %d", i, data[i*3], data[i*3+1], data[i*3+2
    }
  }

  strip.Show();
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Startup");

  //startWifi();

  OtaUploadService otaUploadService;
  otaUploadService.start();

  strip.Begin();
  strip.SetBrightness(50);





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
    delay(20);


  }
  //Serial.println("fade2white done");
}

void white() {
  for(int i=0; i<strip.PixelCount(); i++) { // For each pixel in strip...
    //strip.SetPixelColor(i, RgbwColor(255, 255, 255, 255));
    strip.SetPixelColor(i, RgbwColor(0, 0, 0, 0));
  }
  for(int i=0; i<120; i++) { // For each pixel in strip...
    //strip.SetPixelColor(i, RgbwColor(255, 255, 255, 255));
    strip.SetPixelColor(i, RgbwColor(255, 255, 255, 255));
  }
  strip.Show();
}


void loop()
{
  //whiteOverRainbow(75, 5);
  //rainbowFade2White(3, 3, 1);
  //rainbow();
  white();

  //artnet.read();
}

