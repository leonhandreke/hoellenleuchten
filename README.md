## Hoellenleuchten

## Pinout

- GPIO25 and GPIO27: SK6812 LED strips

## Dependencies

This project uses the [ProjectIO platform](https://platformio.org/).

## Build & upload

	pio run --target upload

or via WiFi using the OTA mechanism:

	PLATFORMIO_UPLOAD_PORT=192.168.0.101 PLATFORMIO_UPLOAD_FLAGS="--auth=password" pio run --target upload -e esp32dev-ota

## Tricks

`pio run --target upload && pio device monitor` to immediately start a serial terminal after uploading.

## Current TODOs

* Implement MQTT
* Add table which light configurations are deployed where
* General code cleanup
