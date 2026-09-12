#!/usr/bin/sh

set -u

BIN_DIR="./bin"
CC="${GCC_COMPILER:-}gcc"

echo Using: ${CC}

output="$BIN_DIR/app"
mkdir -p "$(dirname "$output")"

${CC} -O2 -Wall main.c ./app/app.c ./device/camera.c ./device/v4l2_utils.c ./gpio/gpio_manager.c ./json/config.c -o "$output"