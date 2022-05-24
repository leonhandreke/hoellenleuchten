#pragma once

class EffectsService {

public:
  enum Effect {
    WHITE = 0,
    RAINBOW = 1,
    RAINBOW_PULSE = 2
  };
  EffectsService(
      NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> *strip1,
      NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> *strip2
      ) {
      this->strip1 = strip1;
      this->strip2 = strip2;
  }

  void startEffect(Effect effect) {
    stop();
    this->effect = effect;
    start();
  }

  void stop();

private:
  NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> *strip1;
  NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> *strip2;
  TaskHandle_t taskHandle;

  Effect effect;

  void start();
  static void handleLoop(void *pvParameters);

  void whiteEffect();
  void rainbowEffect();
  void rainbowPulseEffect();
};
