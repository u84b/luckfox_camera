#include "device/camera.h"
#include "gpio/gpio_manager.h"
#include <linux/videodev2.h>
#include <time.h>
#include <signal.h>
/*
TODO LIST:
    - signal handling
    - debug mode
*/
volatile sig_atomic_t keep_running = 1;

void handle_termination(int signum){
    if (signum == SIGTERM){
        //puts("SIGTERM received...");
        keep_running = 0;
    }
}

int create_timestamp_name(char * const filename){
    struct tm *info = NULL;
    time_t raw_time = 0;

    time(&raw_time);
    
    info = localtime(&raw_time);
    
    int res = snprintf(filename, 64, "/userdata/img/frame_%02d.%02d_%02d_%02d_%02d.raw", 
        info->tm_mday, info->tm_mon+1, info->tm_hour, info->tm_min, info->tm_sec);
    
    if (res < 0){
        fprintf(stderr, "snprintf error : %s", strerror(errno));
        return -1;
    }
    return 0;
}

int main(){  
    signal(SIGTERM, handle_termination);
    signal(SIGINT, handle_termination);

    camera cam0;
    camera_format format;
    char output[64];
    camera_buffer_config buf_cfg;
    const char *device = "/dev/video11";
    int gpio_button = 54;
    int fd_gpio = -1;
    int is_opened = -1;

    memset(&format, 0, sizeof(format));
    memset(&buf_cfg, 0, sizeof(buf_cfg));
    memset(&cam0, 0, sizeof(cam0));
    
    gpio_export(gpio_button);
    gpio_direction(gpio_button, "in");


// settings:
    if (camera_init(&cam0) < 0){
        fprintf(stderr, "camera init failed: %s\n", strerror(errno));
        return -1;
    } // initializing general V4L2 structures
    
    puts("Camera init started...");
    
    if (camera_open_video_interface(&cam0, device) < 0) {
        fprintf(stderr, "failed opening camera interface\n");
        return -1;
    } // opening /dev/video11 file
    
    camera_check_capabilities(&cam0); // checking capabilities of V4L2

    camera_set_type(&cam0, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); // setting type for future tasks related to buffer and format configuration
    
    format_set_frame_size(&format, 640, 480); // setting frame size for camera capture
    format_set_pixel_format(&format, V4L2_PIX_FMT_NV12);
    format_set_field(&format, V4L2_FIELD_NONE);

    if (camera_set_format(&cam0, format) < 0){
        fprintf(stderr, "failed setting camera format\n");
        return -1;
    } // copying format structure to camera

    buffer_config_set_count(&buf_cfg, BUFFER_COUNT);
    buffer_config_set_memory(&buf_cfg, V4L2_MEMORY_MMAP);
    if (camera_set_buffer_config(&cam0, buf_cfg) < 0){
        fprintf(stderr, "failed setting buffer configuration\n");
        return -1;

    } // setting buffer configuration for correct work with memory
// preparations:
    camera_map_buffers(&cam0); // mmap usage here
    camera_queue_buffers(&cam0);

// capture:
    camera_stream_on(&cam0);
    
    is_opened = gpio_monitor_pin_value(&fd_gpio, gpio_button, O_RDONLY); // reading GPIO value (changing by pressing button)
    
    if (is_opened == 0) {
        while (1 & keep_running) {
            if (gpio_read(&fd_gpio, gpio_button) == 0){
                create_timestamp_name(output);
                if (camera_capture_frame(&cam0, output) < 0)
                    return -1;
            }
            usleep(20000);
        }
    }
    else {
        fprintf(stderr, "couldn't open gpio %d value\n", gpio_button);
        gpio_close(&fd_gpio, gpio_button);
    }

    camera_stream_off(&cam0);
    camera_cleanup_buffers(&cam0);
    camera_off(&cam0);
    gpio_close(&fd_gpio, gpio_button);
    gpio_unexport(gpio_button);

    return 0;
}