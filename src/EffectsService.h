#pragma once

class EffectsService {

public:
  enum Effect {
    WHITE = 0,
    RAINBOW = 1,
    RAINBOW_PULSE = 2,
    CANDLE = 3,
    WHITE_ALL = 4,
    WIFI_STRENGTH = 5
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
  NeoGamma<NeoGammaTableMethod> colorGamma;

  NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> *strip1;
  NeoPixelBrightnessBus<NeoGrbwFeature, NeoEsp32I2sN800KbpsMethod> *strip2;
  TaskHandle_t taskHandle;

  Effect effect;

  void start();
  static void handleLoop(void *pvParameters);

  void whiteEffect();
  void whiteAllEffect();
  void rainbowEffect();
  void rainbowPulseEffect();
  void candleEffect();
  void wifiStrengthEffect();
};
