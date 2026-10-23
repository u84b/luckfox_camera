# LuckFox Camera

A Linux camera application for the **LuckFox Pico** platform
with an SC3336 camera sensor. The project is written in C and C++ and
explores camera capture, image conversion, and communication between
processes on an embedded Linux device.

> **Project status:** actively developed. Camera capture and the
> converter are separate components. The build and deployment scripts
> target the LuckFox toolchain; the network-transfer workflow and parts
> of setup may still change.

## Overview

The project has two main components:

-   **`camera/`** --- C application responsible for GPIO/button
    handling, camera configuration, and image capture through Linux
    camera interfaces.
-   **`converter/`** --- C++ application using OpenCV to process
    captured `.raw` frames. It supports local IPC mode and has a TCP
    mode under development.

Intended data flow:

``` text
SC3336 camera
     |
     v
 camera/  (C: capture)
     |
     | frame / file notification
     v
 converter/  (C++: OpenCV)
     |
     v
 processed image
```

The components are separate so capture and image processing can be
developed and tested independently.

## Communication modes

  -----------------------------------------------------------------------
  Mode                                Purpose
  ----------------------------------- -----------------------------------
  `ipc`                               Local communication on the LuckFox
                                      device using a Unix domain socket.

  `tcp`                               TCP-based communication, intended
                                      to support communication with
                                      another computer.
  -----------------------------------------------------------------------

The converter defaults to `ipc`. Select a mode with `-m`:

``` sh
# Local IPC mode (default)
/oem/main

# TCP mode
/oem/main -m tcp
```

Use the address and port configured in the converter's networking code
when setting up TCP mode. TCP mode and the application-level transfer
protocol are experimental.

## Repository layout

``` text
luckfox_camera/
├── camera/
│   ├── app/          # Application orchestration
│   ├── device/       # Camera and V4L2-related code
│   ├── gpio/         # GPIO/button handling
│   ├── json/         # JSON configuration parsing
│   ├── note/         # Example/configuration files
│   └── build.sh      # Build and deploy camera executable
├── converter/
│   ├── files/        # File-related utilities
│   ├── imageu/       # Image-processing utilities
│   ├── server/       # IPC/TCP server logic
│   ├── libs/         # OpenCV Mobile dependency
│   ├── CMakeLists.txt
│   └── build.sh      # Build and deploy converter
├── build.sh          # Alternate camera build script
├── README.md
└── TODO.md
```

## Requirements

-   Linux development machine
-   LuckFox Pico cross-compilation toolchain
-   `adb` for deployment to the board
-   CMake and a C++ toolchain for building the converter
-   The OpenCV Mobile package expected by `converter/CMakeLists.txt`

The build scripts use `GCC_COMPILER` as a **compiler prefix** ending in
`arm-rockchip830-linux-uclibcgnueabihf-`, not as the compiler executable
itself.

## Build and deploy

### 1. Clone the repositories

``` sh
git clone https://github.com/u84b/luckfox_camera.git
git clone https://github.com/LuckfoxTECH/luckfox-pico.git
```

Set `GCC_COMPILER` to the toolchain prefix in your local SDK checkout.
Adjust the path to match your machine:

``` sh
export GCC_COMPILER="$HOME/LuckFox/luckfox-pico/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-"
```

### 2. Build the camera application

``` sh
cd luckfox_camera/camera
./build.sh
```

The script builds `bin/camera` and attempts to deploy it to `/oem` on
the connected board using ADB. Connect and authorize the board before
running the script.

### 3. Configure the camera

The camera application accepts a configuration-file path as its first
argument. Without an argument, it uses:

``` text
/userdata/config.json
```

Copy the example configuration to that location (adjust the source path
if your checkout differs):

``` sh
adb push note/config.json /userdata/config.json
```

The application has a default camera-configuration fallback if loading
JSON fails. If capture does not start, check the configuration and
ensure the camera drivers/modules are loaded.

### 4. Build the converter

The converter build script accepts the C++ source file to use as the
entry point. From the converter directory:

``` sh
cd ../converter
./build.sh main.cpp
```

CMake expects the OpenCV Mobile package at:

``` text
converter/libs/opencv-mobile-5.0.0-luckfox-pico/
```

Ensure that dependency is present before building. The script attempts
to push the resulting executable to `/oem` using ADB.

> Build and deployment scripts are evolving. If your checkout differs
> from these commands, inspect `camera/build.sh`, `converter/build.sh`,
> and `converter/CMakeLists.txt` before running them.

## Running on the board

The camera executable is deployed to `/oem/camera`. From an ADB shell:

``` sh
/oem/camera
```

To use an explicit configuration:

``` sh
/oem/camera /userdata/config.json
```

The current converter build script deploys the converter as `/oem/main`.
Start it from an ADB shell:

``` sh
/oem/main
```

To select TCP mode:

``` sh
/oem/main -m tcp
```

The converter requires the camera's kernel modules and device nodes to
be available. Follow the board-specific procedure for loading camera
modules before starting capture or conversion.

## Development notes

-   Camera capture and image conversion are separate executables.
-   The camera component uses C; the converter uses C++ and OpenCV.
-   Local communication uses a Unix domain socket; TCP mode is under
    development.
-   The project targets the LuckFox Linux environment and is not
    currently a generic, host-independent camera application.

## Current limitations

-   TCP transfer and its application-level protocol are still being
    developed.
-   Build and deployment scripts are tailored to the author's toolchain
    and ADB workflow.
-   Startup integration depends on the LuckFox firmware image and its
    camera-driver setup.

## License

No license is currently specified in this repository. Unless a license
is added, reuse and redistribution are not automatically granted.
