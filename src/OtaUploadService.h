#include <ArduinoOTA.h>


class OtaUploadService {

public:
  void start() {
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

    xTaskCreate(
        this->handleLoop,    // Function that should be called
        "Handle OTA Upload",  // Name of the task (for debugging)
        1000,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL             // Task handle
    );
  }

private:
  static void handleLoop(void *pvParameters) {
    ArduinoOTA.handle();
    delay(500);
  }
};
