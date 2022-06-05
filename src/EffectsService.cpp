#include <Arduino.h>
#include <WiFi.h>

#include <NeoPixelBrightnessBus.h>

#include "EffectsService.h"


// Copy-pasta from NeoPixel example
//void whiteOverRainbow(
//    NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod> strip1,
//    int whiteSpeed, int whiteLength) {
//
//  if(whiteLength >= strip1.PixelCount()) whiteLength = strip1.PixelCount() - 1;
//
//  int      head          = whiteLength - 1;
//  int      tail          = 0;
//  int      loops         = 3;
//  int      loopNum       = 0;
//  uint32_t lastTime      = millis();
//  uint32_t firstPixelHue = 0;
//
//  for(;;) { // Repeat forever (or until a 'break' or 'return')
//    for(int i=0; i<strip1.PixelCount(); i++) {  // For each pixel in strip1...
//      if(((i >= tail) && (i <= head)) ||      //  If between head & tail...
//         ((tail > head) && ((i >= tail) || (i <= head)))) {
//        strip1.SetPixelColor(i, RgbwColor(0, 0, 0, 255)); // Set white
//      } else {                                             // else set rainbow
//        float pixelHue = firstPixelHue + (i * 1.0 / strip1.PixelCount());
//        strip1.SetPixelColor(i, colorGamma.Correct(RgbColor(HsbColor(pixelHue, 1.0, 1.0))));
//      }
//    }
//
//    strip1.Show(); // Update strip1 with new contents
//    // There's no delay here, it just runs full-tilt until the timer and
//    // counter combination below runs out.
//
//    firstPixelHue += 40; // Advance just a little along the color wheel
//
//    if((millis() - lastTime) > whiteSpeed) { // Time to update head/tail?
//      if(++head >= strip1.PixelCount()) {      // Advance head, wrap around
//        head = 0;
//        if(++loopNum >= loops) return;
//      }
//      if(++tail >= strip1.PixelCount()) {      // Advance tail, wrap around
//        tail = 0;
//      }
//      lastTime = millis();                   // Save time of last movement
//    }
//  }
//}

void EffectsService::start() {
  xTaskCreate(
      this->handleLoop,
      "Effect",
      10000,
      this,
      1,
      &this->taskHandle);
}

void EffectsService::handleLoop(void *pvParameters) {
  EffectsService *_this = static_cast<EffectsService *>(pvParameters);

  // While true to make sure the task does not exit and therefore crash
  while (true) {
    switch (_this->effect) {
      case Effect::RAINBOW:
        _this->rainbowEffect();
        break;
      case Effect::WHITE:
        _this->whiteEffect();
        break;
      case Effect::RAINBOW_PULSE:
        _this->rainbowPulseEffect();
        break;
      case Effect::CANDLE:
        _this->candleEffect();
        break;
      case Effect::WHITE_ALL:
        _this->whiteAllEffect();
        break;
      case Effect::WIFI_STRENGTH:
        _this->wifiStrengthEffect();
        break;
      case Effect::RAINBOW_FASTER_SLOWER:
        _this->rainbowFasterSlowerEffect();
        break;
    }
    taskYIELD();
  }
}

void EffectsService::stop() {
  if(taskHandle != NULL) {
    vTaskDelete( taskHandle );
  }
}

void EffectsService::rainbowFasterSlowerEffect() {
  for (int i = 1; i <= 30; i++) {
    rainbow(i);
  }
  for (int i = 30; i >= 1; i--) {
    rainbow(i);
  }
}

void EffectsService::rainbowEffect() {
  while (true) {
    rainbow(20);
  }
}
void EffectsService::rainbow(int wait) {
  // Hue of first pixel runs 'rainbowLoops' complete loops through the color
  // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to rainbowLoops*65536, using steps of 256 so we
  // advance around the wheel at a decent clip.
  for (uint32_t firstPixelHue = 0; firstPixelHue < 65536;
       firstPixelHue += 256) {

    for (int i = 0; i < strip1->PixelCount(); i++) { // For each pixel in strip1...

      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip1
      // (strip1.numPixels() steps):
      uint32_t pixelHue = firstPixelHue + (i * 65536L / strip1->PixelCount());

      // strip1.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the three-argument variant, though the
      // second value (saturation) is a constant 255.
      //strip1.SetPixelColor(i, colorGamma.Correct(RgbColor(
      //        HsbColor(pixelHue / float(65536L), 1.0, 1.0))));
      auto color = colorGamma.Correct(RgbwColor(
          HsbColor(pixelHue / float(65536L), 1.0, 1.0)));
      strip1->SetPixelColor(i, color);
      strip2->SetPixelColor(i, color);
    }

//      for (int i = 0; i < 70; i++) {
//        strip1->SetPixelColor(i, RgbwColor(0, 0, 0, 0));
//        strip2->SetPixelColor(i, RgbwColor(0, 0, 0, 0));
//      }

    strip1->Show();
    strip2->Show();
    vTaskDelay(wait);
  }
}

void EffectsService::rainbowPulseEffect() {
  uint8_t pulse = 0;

  while (true) {
    // Hue of first pixel runs 'rainbowLoops' complete loops through the color
    // wheel. Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to rainbowLoops*65536, using steps of 256 so we
    // advance around the wheel at a decent clip.
    for (uint32_t firstPixelHue = 0; firstPixelHue < 65536;
         firstPixelHue += 256) {

      pulse++;
      if (pulse > 100) {
        pulse = 0;
      }

      for (int i = 0; i < strip1->PixelCount(); i++) { // For each pixel in strip1...

        // Offset pixel hue by an amount to make one full revolution of the
        // color wheel (range of 65536) along the length of the strip1
        // (strip1.numPixels() steps):
        uint32_t pixelHue = firstPixelHue + (i * 65536L / strip1->PixelCount());

        // strip1.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
        // optionally add saturation and value (brightness) (each 0 to 255).
        // Here we're using just the three-argument variant, though the
        // second value (saturation) is a constant 255.
        //strip1.SetPixelColor(i, colorGamma.Correct(RgbColor(
        //        HsbColor(pixelHue / float(65536L), 1.0, 1.0))));
        auto color = colorGamma.Correct(RgbwColor(
            HsbColor(pixelHue / float(65536L), 1.0, 1.0)));
        strip1->SetPixelColor(i, color);
        strip2->SetPixelColor(i, color);
      }

      strip1->Show();
      strip2->Show();

      if (pulse > 25) {
        vTaskDelay(3);
      } else {
        vTaskDelay(15);
      }
    }
  }
}

void EffectsService::whiteEffect() {
  while (true) {
    strip1->ClearTo(RgbwColor(0, 0, 0, 255));
    strip1->Show();
    strip2->ClearTo(RgbwColor(0, 0, 0, 255));
    strip2->Show();
    vTaskDelay(1000);
  }
}

void EffectsService::whiteAllEffect() {
  while (true) {
    auto color = RgbwColor(255, 255, 255, 255);
    strip1->ClearTo(color);
    strip1->Show();
    strip2->ClearTo(color);
    strip2->Show();
    vTaskDelay(1000);
  }
}

void EffectsService::candleEffect() {

  strip1->ClearTo(RgbwColor(0, 0, 0, 0));
  strip2->ClearTo(RgbwColor(0, 0, 0, 0));

  auto color = RgbwColor(200, 20, 10);
  uint8_t length = 8;
  uint8_t brightness = 50;
  uint8_t cycle = 0;

  while (true) {
    if (cycle % 1 == 0) {
      color = colorGamma.Correct(RgbwColor(200 + random(50), 80 + random(30), 10));
      length = 8 + random(7);
      brightness = 30 + random (30);
    }

    for (int i = 0; i < strip1->PixelCount(); i++) {
      if (i < length) {
        strip1->SetPixelColor(i, color);
        strip2->SetPixelColor(i, color);
      } else {
        strip1->SetPixelColor(i, RgbwColor(0, 0, 0, 0));
        strip2->SetPixelColor(i, RgbwColor(0, 0, 0, 0));
      }
    }

    strip1->SetBrightness(brightness);
    strip2->SetBrightness(brightness);

    strip1->Show();
    strip2->Show();

    vTaskDelay(100);
    cycle++;
  }
}

void EffectsService::wifiStrengthEffect() {
  WiFi.RSSI();

  while (true) {
    auto color = RgbwColor(0, 255, 0, 0);
    strip1->Show();
    strip2->ClearTo(color);
    strip2->Show();
    vTaskDelay(100);
  }
}