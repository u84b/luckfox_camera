# LuckFox Camera
## Brief description
Application for LuckFox SC3336 camera.\
Also it has converter module, that works
The program is triggered by pressing a button, takes a frame, and saves the `.raw` file in /userdata/ 

## How to compile it?
### Instruction for users (Linux only): 
For compiling project on your machine you need to use LuckFox Pico cross-compiler ``arm-rockchip830-linux-uclibcgnueabihf-gcc``.\
Choose directory on your machine where you want to place source code and then:
```bash
git clone https://github.com/u84b/luckfox_camera/
```
Choose directory on your machine where you want to place the compiler and then:
```bash
git clone https://github.com/LuckfoxTECH/luckfox-pico/
```
Then you need to add that line in your environment variables, for example:
```bash
#change USER in path to your username
export GCC_COMPILER=/home/USER/LuckFox/luckfox-pico/tools/linux/toolchain/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf-
```
**Then I recommend use `camera/build.sh` script from directory to build camera part of project.**\
Before you test camera program on LuckFox board, you better send config file to LuckFox. I prefer ``adb`` usage instead of ``ssh`` (the second one may be more difficult):
```bash
adb push camera/note/config.json /userdata
```
To build converter-server part first of all you need to use `downloads.sh` script in `converter/libs` for downloading all required opencv-mobile libraries. Then I recommend to use `converter/build.sh`, CMake is used there, it will build the executable file. Then:
```bash
adb push
```

### How to start converter as daemon?
Just place that script as `S45camera` to `/etc/init.d` directory in your LuckFox. It's initial script to load all required video nodes and starting converter-server:
```bash
#!/bin/sh
case "$1" in
    start)
        echo "Starting CSI camera modules"
        sh /oem/usr/ko/camera_insmod_ko.sh
        nohup /oem/main > /tmp/server.log 2>&1 &
        ;;
    stop)
        echo "Stopping CSI camera modules"
        killall /oem/main 2>/dev/null
        ;;
    *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
        ;;
esac
```