#include <Arduino.h>

#include <NeoPixelBrightnessBus.h>

#include "EffectsService.h"

NeoGamma<NeoGammaTableMethod> colorGamma;

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
    }
    taskYIELD();
  }
}

void EffectsService::stop() {
  if(taskHandle != NULL) {
    vTaskDelete( taskHandle );
  }
}

void EffectsService::rainbowEffect() {
  while (true) {
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
        strip1->SetPixelColor(i,
                            HsbColor(pixelHue / float(65536L), 1.0, 1.0));
        strip2->SetPixelColor(i,
                             HsbColor(pixelHue / float(65536L), 1.0, 1.0));
      }

      strip1->Show();
      strip2->Show();
      vTaskDelay(20);
    }
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
        strip1->SetPixelColor(i,
                              HsbColor(pixelHue / float(65536L), 1.0, 1.0));
        strip2->SetPixelColor(i,
                              HsbColor(pixelHue / float(65536L), 1.0, 1.0));
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
