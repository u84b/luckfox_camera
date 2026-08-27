#!/usr/bin/env python3
import os

REMOTE_PATH = "/userdata/img"
LOCAL_RAW_DIR = "./raw_frames"
OUTPUT_PNG_DIR = "./converted_png"
WIDTH = 640
HEIGHT = 480

os.makedirs(LOCAL_RAW_DIR, exist_ok=True)
os.makedirs(OUTPUT_PNG_DIR, exist_ok=True)

# List files
os.system("adb shell \"ls --color=never {}/*.raw\" > files.txt".format(REMOTE_PATH))

f = open('files.txt', 'r')
filenames = f.read().split()
f.close()
#pull .raw files from /userdata/
for file in filenames:
    if (not(os.path.isfile("./raw_frames/{}".format(file)))):
        os.system("adb pull {} ./raw_frames".format(file))

print(os.getcwd())
filenames = os.listdir("./raw_frames")

for file in filenames:
    os.system("ffmpeg -y -f rawvideo -pixel_format nv12 -video_size {}x{} -i {}/{} -frames:v 1 {}/{}png > /dev/null 2>&1".format(WIDTH, HEIGHT,
     LOCAL_RAW_DIR, file, OUTPUT_PNG_DIR, file[:-3]))

print("Done")