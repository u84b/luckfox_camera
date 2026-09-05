#!/usr/bin/sh

set -u

mkdir -p ./bin

gcc cjson_parse.c ../json/lib/cJSON.c ../json/lib/cJSON_Utils.c -o "./bin/parsed" -lm
gcc -Oz -Wl,--gc-sections check_size.c ../json/lib/cJSON.c ../json/lib/cJSON_Utils.c ../device/camera.c ../device/v4l2_utils.c -o "./bin/size" -lm