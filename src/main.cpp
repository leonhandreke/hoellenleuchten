#include <sstream>

#include <Arduino.h>
#include <ESPmDNS.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h>

const uint16_t PixelCount = 20;
const uint16_t PixelPin = 26;

NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod> strip(PixelCount, PixelPin);
NeoGamma<NeoGammaTableMethod> colorGamma;


void startMdns() {
  std::stringstream mdnsName;
  mdnsName << "hoellenleuchten-" << std::hex << ESP.getEfuseMac();
  MDNS.begin(mdnsName.str().c_str());
}

void setup()
{
  startMdns();

  Serial.begin(9600);

  strip.Begin();
  strip.SetBrightness(50);
}

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

void loop()
{
  whiteOverRainbow(75, 5);
}

