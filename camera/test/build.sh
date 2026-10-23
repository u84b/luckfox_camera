#!/usr/bin/sh

set -u
CC="${GCC_COMPILER:-}gcc"
mkdir -p ./bin

#gcc cjson_parse.c ../json/lib/cJSON.c ../json/lib/cJSON_Utils.c -o "./bin/parsed" -lm
gcc -Oz -Wl,--gc-sections check_size.c ../json/*.c ../json/lib/*.c ../device/*.c ../app/*.c -o "./bin/size" -lm
${CC} -O2 -Wl,--gc-sections client_test.c -o ./bin/client
gcc -Oz -Wl,--gc-sections client_test.c -o ~/works/lazy/client

adb push ./bin/client /oem
# BIN_DIR="./bin"
# 

# echo Using: ${CC}

# output="$BIN_DIR/poll_gpio"
# mkdir -p "$(dirname "$output")"

# ${CC} -O2 -Wall -Wl,--gc-sections poll_gpio_test.c ../gpio/gpio_manager.c ../json/config.c ../json/lib/cJSON.c ../json/lib/cJSON_Utils.c -o "$output"
