#!/usr/bin/env bash

# yq '.[] | select(contains({"groups": ["weg"]})) | .ip' devices.yaml

yq '.[].ip' devices.yaml | while read line
do
	echo $line
	PLATFORMIO_UPLOAD_PORT=$line PLATFORMIO_UPLOAD_FLAGS="--auth=admin" pio run --target upload -e nodemcu-32s-ota
done

