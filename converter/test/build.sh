set -u

mkdir -p build

SOURCE="$1"

NAME=$(basename "$SOURCE" .cpp)
BUILD_DIR="build/$NAME"

g++ -g -O2 -Wall -Wextra -march=native -flto "$SOURCE" -o "$BUILD_DIR"