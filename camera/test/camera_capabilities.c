#include "../device/camera.h"
#include <signal.h>

volatile sig_atomic_t keep_running = 1;

void handle_termination(int signum) {
    if (signum == SIGTERM) keep_running = 0;
}

// checking camera capabilities & initialization

int main(){

    signal(SIGTERM, handle_termination);
    signal(SIGINT, handle_termination);

    camera cam;
    //camera_format fmt;
    const char *main_video_path = "/dev/video11";

    memset(&cam, 0, sizeof(cam));

    camera_init(&cam);
    puts("Camera init finished...");
    camera_open_video_interface(&cam, main_video_path);
    printf("Opened v4l2 interface in %s\n", main_video_path);
    camera_check_capabilities(&cam);
    puts("Checked V4L2 capabilities...\n");
    camera_off(&cam);

    return 0;

}