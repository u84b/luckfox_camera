#include "device/camera.h"
#include "gpio/gpio_manager.h"
#include <linux/videodev2.h>

/*
TODO LIST:
    - README.md
    - debug mode
*/

int main(){  
    camera cam0;
    camera_format format;
    camera_buffer_config buf_cfg;
    const char *output = "/userdata/frame.raw";
    const char *device = "/dev/video11";
    int gpio_button = 54;

    memset(&format, 0, sizeof(format));
    memset(&buf_cfg, 0, sizeof(buf_cfg));
    memset(&cam0, 0, sizeof(cam0));
    
    gpio_export(gpio_button);
    gpio_direction(gpio_button, "in");


// settings:
    camera_init(&cam0); // initializing general V4L2 structures
    puts("Camera init started...");
    camera_open_video_interface(&cam0, device); // opening /dev/video11 file
    
    camera_check_capabilities(&cam0); // checking capabilities of V4L2

    camera_set_type(&cam0, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); // setting type for future tasks related to buffer and format configuration
    
    format_set_frame_size(&format, 640, 480); // setting frame size for camera capture
    format_set_pixel_format(&format, V4L2_PIX_FMT_NV12);
    format_set_field(&format, V4L2_FIELD_NONE);

    camera_set_format(&cam0, format); // copying format structure to camera

    buffer_config_set_count(&buf_cfg, BUFFER_COUNT);
    buffer_config_set_memory(&buf_cfg, V4L2_MEMORY_MMAP);
    camera_set_buffer_config(&cam0, buf_cfg);
// preparations:
    camera_map_buffers(&cam0);
    camera_queue_buffers(&cam0);

// capture:
    camera_stream_on(&cam0);



    while (1) {
        int r = gpio_read(gpio_button);
        
        if (r == 0){
            printf("GPIO STATUS: %d\n", r);
            if (camera_capture_frame(&cam0, output) < 0)
                return -1;
        }
        usleep(20000);
    }

    camera_stream_off(&cam0);
    camera_cleanup_buffers(&cam0);
    camera_off(&cam0);

    return 0;
}