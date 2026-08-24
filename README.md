# LuckFox Camera
## Brief description
Application for LuckFox SC3336 camera.\
The program is triggered by pressing a button, takes a frame, and saves the `.raw` file in /userdata/ 

## How to compile it?
### Instruction for Linux users: 
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
**Then I recommend you to use `build.sh` script from directory to build project.**