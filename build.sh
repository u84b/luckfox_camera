#!/usr/bin/env bash

set -u

BIN_DIR="./bin"
CC="${GCC_COMPILER:-}gcc"

echo Using: ${CC}

output="$BIN_DIR/app"
mkdir -p "$(dirname "$output")"


${CC} -O2 main.c ./device/camera.c ./device/v4l2_utils.c ./gpio/gpio_manager.c -o "$output"