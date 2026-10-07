#!/usr/bin/sh

set -u

BIN_DIR="./bin"
CC="${GCC_COMPILER:-}gcc"

echo Using: ${CC}

output="$BIN_DIR/camera"
mkdir -p "$(dirname "$output")"

${CC} -O2 -Wall -Wextra -Wl,--gc-sections main.c ./app/*.c ./device/*.c ./gpio/*.c ./json/*.c ./json/lib/*.c -o "$output"
adb push "$output" /oem
