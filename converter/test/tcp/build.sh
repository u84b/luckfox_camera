set -u

mkdir -p build

CXX="${GCC_COMPILER:-}g++"

echo Using:${CXX}

# SOURCE0="$1"
# SOURCE1="$2"

# NAME=$(basename "$SOURCE0" .cpp)
# NAME1=$(basename "$SOURCE1" .cpp)
# BUILD_DIR0="tcp/build/$NAME"
# BUILD_DIR1="tcp/build/$NAME1"

g++ -g -O2 -Wall -Wextra -march=native -flto server.cpp -o server
# ${CXX} -O2 -Wall -Wextra -flto client.cpp -o client
# adb push client /oem