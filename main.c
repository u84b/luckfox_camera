#include "device/camera.h"
#include "gpio/gpio_manager.h"
#include "json/lib/cJSON.h"
#include <linux/videodev2.h>
#include <time.h>
#include <signal.h>
/*
TODO LIST:
    MAIN PRIORITY: 
    - Create a separate module for working with JSON 
      configuration, divide the responsibilities
    OTHER STUFF:
    - signal handling
    - debug mode
*/

// it's horrible to see so many code in main but it's temporary situation.
// I'll handle it. I'll be back.


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
    
    int written = snprintf(filename, 64, "/userdata/img/frame_%02d.%02d_%02d_%02d_%02d.raw", 
        info->tm_mday, info->tm_mon+1, info->tm_hour, info->tm_min, info->tm_sec);
    
    if (written < 0){
        fprintf(stderr, "snprintf error : %s", strerror(errno));
        return -1;
    }
    return 0;
}
// temporary function
// will change its location in future updates
char* read_file(const char *filename) {
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}
// temporary function
// will change its location in future updates
static cJSON *parse_file(const char *filename)
{
    cJSON *parsed = NULL;
    char *content = read_file(filename);

    parsed = cJSON_Parse(content);

    if (content != NULL)
    {
        free(content);
    }

    return parsed;
}

// prototype function in main.c
// will change its location in future updates
int gpio_configuration(int gpio_pin_num){
    int result = -1;

    if (gpio_export(gpio_pin_num) < 0) {
        printf("gpio%d export failed\n", gpio_pin_num);
    }
    if (gpio_direction(gpio_pin_num, "in") < 0) {
        printf("gpio%d setting direction failed\n", gpio_pin_num);
    }
    result = 0;
    return result;
}

// prototype function in main.c
// will change its location in future updates
// and I also didn't want so many procedures to be called in main
// @TODO: analyze error handling in more detail and make it more efficient
// @TODO: Ideally, the function should be moved to camera.h, with some corrections
int init_camera_from_config(camera * const cam0, camera_format * const format, cJSON *json_tree, 
    cJSON *camera_params, camera_buffer_config * buf_cfg, 
    const char * config_path, const char * device){
    int width = 0;
    int height = 0;
    int memory_type = 0;
    int result = -1;

    json_tree = parse_file(config_path);

    if (json_tree == NULL) {
        printf("Parsing file %s failed\n", config_path);
    }

    camera_params = cJSON_GetObjectItem(json_tree, "camera");

    device = cJSON_GetObjectItem(json_tree, "device_path")->valuestring;
    width = cJSON_GetObjectItem(json_tree, "width")->valueint;
    height = cJSON_GetObjectItem(json_tree, "height")->valueint;
    memory_type = cJSON_GetObjectItem(json_tree, "memory_type")->valueint;

    if (camera_init(cam0) < 0){
        fprintf(stderr, "camera init failed: %s\n", strerror(errno));
        goto end;
    } // initializing general V4L2 structures
    
    puts("Camera init started...");
    
    if (camera_open_video_interface(cam0, device) < 0) {
        fprintf(stderr, "failed opening camera interface\n");
        goto end;
    } // opening /dev/video11 file
    
    puts("Opened camera interface...");

    if (camera_check_capabilities(cam0) < 0) {
        fprintf(stderr, "v4l2 capabilities failed\n");
    }; // checking capabilities of V4L2
    puts("Checked camera capabilities\n");
    camera_set_type(cam0, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); // setting type for future tasks related to buffer and format configuration
    
    format_set_frame_size(format, width, height); // setting frame size for camera capture
    format_set_pixel_format(format, V4L2_PIX_FMT_NV12);
    format_set_field(format, V4L2_FIELD_NONE);

    if (camera_set_format(cam0, *format) < 0){
        fprintf(stderr, "failed setting camera format\n");
        goto end;
    } // copying format structure to camera
    puts("Successfully set camera format...\n");
    buffer_config_set_count(buf_cfg, BUFFER_COUNT);
    buffer_config_set_memory(buf_cfg, memory_type);
    if (camera_set_buffer_config(cam0, *buf_cfg) < 0){
        fprintf(stderr, "failed setting buffer configuration\n");
        goto end;
    } // setting buffer configuration for correct work with memory
    puts("Successfully set camera buffer configuration\n");
// preparations:
    if (camera_map_buffers(cam0) < 0) {
        fprintf(stderr, "failed mapping of camera buffers\n");
    } // mmap usage here
    
    if (camera_queue_buffers(cam0) < 0) {
        fprintf(stderr, "failed queue camera buffers");
    }
    result = 0;
end:
    return result;
}
// TEMP: I will polish some details in next commit
int main(){  
    signal(SIGTERM, handle_termination);
    signal(SIGINT, handle_termination);

    camera cam0;
    camera_format format;
    cJSON *tree = {0};
    cJSON *camera_params = {0};
    char output[64];
    camera_buffer_config buf_cfg;
    const char *config_path = "/userdata/config.json";
    const char *device = NULL;
    int gpio_button = 54;
    int fd_gpio = -1;
    int is_opened = -1;

    memset(&format, 0, sizeof(format));
    memset(&buf_cfg, 0, sizeof(buf_cfg));
    memset(&cam0, 0, sizeof(cam0));
    
    /** 
    I guess it's irrelevant to shutdown 
    entire program just because of gpio troubles
    **/
    if (gpio_configuration(gpio_button) < 0) {
        printf("gpio%d configuration failed\n", gpio_button);
    }
// settings:
    if (init_camera_from_config(&cam0, &format, 
        tree, camera_params, &buf_cfg, config_path,device) < 0) {

    }
// capture:
    camera_stream_on(&cam0);
    
    is_opened = gpio_monitor_pin_value(&fd_gpio, gpio_button, O_RDONLY); // reading GPIO value (changing by pressing button)
    
    if (is_opened == 0) {
        while (keep_running) {
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
        //gpio_close(&fd_gpio, gpio_button);
    }
// TODO: advanced cleanup system
    camera_stream_off(&cam0);
    camera_cleanup_buffers(&cam0);
    camera_off(&cam0);
    gpio_close(&fd_gpio, gpio_button);
    gpio_unexport(gpio_button);

    return 0;
}