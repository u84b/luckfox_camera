#!/bin/bash

set -e

SOURCE="$1"

if [ -z "$SOURCE" ]; then
    echo "Usage: $0 <source.cpp>"
    exit 1
fi

NAME=$(basename "$SOURCE" .cpp)
BUILD_DIR="build/$NAME"

cmake -S . -B "$BUILD_DIR" -DSOURCE="$SOURCE"
cmake --build "$BUILD_DIR"

mv "$BUILD_DIR/converter" "$BUILD_DIR/$NAME"