# I used another guide, because I'm still not confident with my ability of creating
# good enough CMake configuration. I didn't love it at university, so that's the point 
# to learn CMake better
cmake_minimum_required(VERSION 3.5)
project(converter)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_LINK_DEPENDS_NO_SHARED ON)

SET(CMAKE_C_COMPILER "$ENV{GCC_COMPILER}gcc")
SET(CMAKE_CXX_COMPILER "$ENV{GCC_COMPILER}g++")

set(CMAKE_SYSTEM_PROCESSOR arm)

set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/opencv-mobile-5.0.0-luckfox-pico/lib/cmake/opencv5")
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

if(NOT DEFINED SOURCE)
    set(SOURCE main.cpp)
endif()

add_executable(converter ${SOURCE})

target_compile_options(converter PUBLIC -O3 -Wall -Wextra -Wl,--gc-sections)

target_link_libraries(converter ${OpenCV_LIBS})