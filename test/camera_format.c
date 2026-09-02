#include "../device/camera.h"



int main() {

    camera cam;
    camera_format fmt;
    camera_buffer_config cfg;
    const char *main_video_path = "/dev/video11";

    memset(&cam, 0, sizeof(cam));
    memset(&fmt, 0, sizeof(fmt));
    memset(&cfg, 0, sizeof(cfg));
    
    camera_init(&cam);
    camera_open_video_interface(&cam, main_video_path);
    
    camera_set_type(&cam, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); // setting type for future tasks related to buffer and format configuration
    format_set_frame_size(&fmt, 640, 480); // setting frame size for camera capture
    format_set_pixel_format(&fmt, V4L2_PIX_FMT_NV12);
    format_set_field(&fmt, V4L2_FIELD_NONE);

    if (camera_set_format(&cam, fmt) < 0){
        fprintf(stderr, "failed setting camera format\n");
        return -1;
    } // copying format structure to camera

    camera_off(&cam);
    return 0;
}