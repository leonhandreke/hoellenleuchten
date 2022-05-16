#pragma once

class EffectsService {

public:
  enum Effect {
    WHITE = 0,
    RAINBOW = 1
  };
  EffectsService(
      NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod> *strip) {
      this->strip = strip;
  }

  void startEffect(Effect effect) {
    this->effect = effect;
    start();
  }

  void stop();

private:
  NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2s1800KbpsMethod> *strip;
  TaskHandle_t taskHandle;

  Effect effect;

  void start();

  void whiteEffect();
  void rainbowEffect();

};
