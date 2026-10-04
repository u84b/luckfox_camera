#include "app.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define APP_C_PADDING 4
#define APP_C_STREAM_PADDING 6

volatile static sig_atomic_t keep_running = 1;

void handle_termination(int signum){
    if (signum == SIGTERM || signum == SIGINT){ // it was obvious but I haven't changed it till now
        keep_running = 0;
    }
}

int create_timestamp_name(char * const filename){
    struct tm *info = NULL;
    time_t raw_time = 0;

    time(&raw_time);
    
    info = localtime(&raw_time);
    
    int written = snprintf(filename, 64, "/oem/img/frame_%02d.%02d_%02d_%02d_%02d.raw", 
        info->tm_mday, info->tm_mon+1, info->tm_hour, info->tm_min, info->tm_sec);
    
    if (written < 0){
        fprintf(stderr, "snprintf error : %s", strerror(errno));
        return -1;
    }
    return 0;
}

int init_application(application *app){

    signal(SIGTERM, handle_termination);
    signal(SIGINT, handle_termination);
    
    memset(&app->cam, 0, sizeof(app->cam));

    app->gpio_button = 54;
    app->fd_gpio = -1;
    app->client_fd = -1;
    app->is_opened = -1;


    return 0;
}

int load_config_from_json(const char * const config_path, camera_config * cfg){
    //puts("LOAD_CONFIG_FROM_JSON");
    if (config_path == NULL) return 1; // okay, not fatal, because we can use default configuration

    if (cfg == NULL) return -1;
    
    memset(&cfg->buf_cfg, 0, sizeof(cfg->buf_cfg)); // I forgot it last time
    memset(&cfg->device_path, 0, sizeof(cfg->device_path)); // it's neccessary to prevent segfault...
    memset(&cfg->format, 0, sizeof(cfg->format));
    
    cJSON* json_tree = {0};
    cJSON* camera_params = {0};
    char *buf = NULL;
    int result = -1;
    
    json_tree = parse_file(config_path);
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "json file parsed");
    }

    if (json_tree == NULL)
    {
        printf("Parsing file %s failed\n", config_path);
        goto end;
    }
    
    
    camera_params = cJSON_GetObjectItem(json_tree, "camera");
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "Got camera params JSON object");
    }

    buf = cJSON_GetObjectItem(camera_params, "device_path")->valuestring;
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s%s\n", APP_C_PADDING, "Device path: ", buf);
    }
    
    int written = snprintf(cfg->device_path, sizeof(cfg->device_path), "%s", buf);
    
    if (written < 0){
        fprintf(stderr, "snprintf error : %s", strerror(errno));
        goto end;
    
    }
    
    cfg->format.format.fmt.pix_mp.width = cJSON_GetObjectItem(camera_params, "width")->valueint;
    cfg->format.format.fmt.pix_mp.height = cJSON_GetObjectItem(camera_params, "height")->valueint;
    cfg->format.format.fmt.pix_mp.pixelformat = cJSON_GetObjectItem(camera_params, "pixelFormat")->valueint;
    cfg->buf_cfg.buf_config.memory = cJSON_GetObjectItem(camera_params, "memoryType")->valueint;
    cfg->buf_cfg.buf_config.count = cJSON_GetObjectItem(camera_params, "bufferCount")->valueint;

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "Camera format/buffer setting completed");
    }

    if (json_tree != NULL)
    {
        
        cJSON_Delete(json_tree);
        if (OPTIONS_MASK & DEBUG_MODE)
        {
            printf("%*s\n", APP_C_PADDING, "cJSON tree object deletion completed");
        }

    }
    result = 0;
end:
    return result;
}

int app_camera_init_from_config(camera * const c, camera_config * const config){
    if (c == NULL)
    {
        return -1;
    }

    if (config == NULL)
    {
        return -1;
    }

    if (camera_init(c) < 0){
        fprintf(stderr, "camera init failed: %s\n", strerror(errno));
        return -1;
    }

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "Camera init successfully");
    }
    
    if (camera_open_video_interface(c, config->device_path) < 0) {
        fprintf(stderr, "failed opening camera interface\n");
        return -1;
    }
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "V4L2 video node opened");
    }
    
    if (camera_check_capabilities(c) < 0) {
        fprintf(stderr, "v4l2 capabilities failed\n");
        return -1;
    } // checking capabilities of V4L2

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "V4L2 capabilities okay");
    }
    
    camera_set_type(c, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);

    if (camera_set_format(c, config->format))
    {
        fprintf(stderr, "failed setting camera format\n");
        return -1;
    }


    if (camera_set_buffer_config(c, config->buf_cfg)) {
        fprintf(stderr, "failed setting buffer configuration\n");
        return -1;
    }

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "Camera set buffer config completed");
    }
    

    return 0;
}


int app_camera_config_default(application *app){

    if (app == NULL) return -1;

    camera_format format = {0};
    camera_buffer_config buffer_config = {0};
    char *video_node = "/dev/video11";
    int result = -1;

    int written = snprintf(app->device, 32, "%s", video_node);
    
    if (written < 0){
        fprintf(stderr, "snprintf error : %s", strerror(errno));
        return -1;
    }

    if (camera_init(&app->cam) < 0){
        fprintf(stderr, "camera init failed: %s\n", strerror(errno));
        goto end;
    } // initializing general V4L2 structures

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "Camera init successfully");
    }

    if (camera_open_video_interface(&app->cam, video_node) < 0) {
        fprintf(stderr, "failed opening camera interface\n");
        goto end;
    }

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "V4L2 video node opened");
    }

    if (camera_check_capabilities(&app->cam) < 0) {
        fprintf(stderr, "v4l2 capabilities failed\n");
        goto end;
    }

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "V4L2 capabilities okay");
    }

    camera_set_type(&app->cam, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); // setting type for future tasks related to buffer and &app->format configuration
    
    format_set_frame_size(&format, 640, 480); // setting frame size for camera capture
    format_set_pixel_format(&format, V4L2_PIX_FMT_NV12);
    format_set_field(&format, V4L2_FIELD_NONE);

    if (camera_set_format(&app->cam, format) < 0)
    {
        goto end;
    }



    buffer_config_set_memory(&buffer_config, V4L2_MEMORY_MMAP);
    buffer_config_set_count(&buffer_config, BUFFER_COUNT);

    if (camera_set_buffer_config(&app->cam, buffer_config) < 0)
    {
        goto end;
    }

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "Camera set buffer config completed");
    }

    result = 0;
end:
    return result;
};


// @TODO: implement GPIO poll & IPC functions in another place to use it here
// @TODO: create fundamental functions for wireless communication
int app_prepare_camera_buffers(application * app){
    int result = -1;

    if (app == NULL) goto end;

    if (camera_map_buffers(&app->cam) < 0) {
        fprintf(stderr, "failed mapping of camera buffers\n");
        goto end;
    } // mmap usage here

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "Mapping camera buffers completed");
    }
    
    if (camera_queue_buffers(&app->cam) < 0) {
        fprintf(stderr, "failed queue camera buffers");
        goto end;
    }

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_PADDING, "Queue camera buffers completed");
    }

    result = 0;
end:
    return result;
}

int app_run_camera_stream(application * app){

    int result = -1;
    int flag = 0;
    int connected = 0;
    camera_stream_on(&app->cam);

    // it mostly happens because I didn't connect camera to board
    if (app->cam.stream_started != 1)
    {
        printf("Camera stream on failed: %m\n");
        goto end;
    }
    
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_STREAM_PADDING, "Camera stream on");
    }

    app->is_opened = gpio_monitor_pin_value(&app->fd_gpio, app->gpio_button, O_RDONLY);
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        printf("%*s\n", APP_C_STREAM_PADDING, "GPIO value monitoring started");
    }
    
    if (app->is_opened == 0) {

        struct pollfd gpio_poll; // I'll implement that part in another place soon
        memset(&gpio_poll, 0, sizeof(struct pollfd));
        gpio_poll.fd = app->fd_gpio;
        gpio_poll.events = POLLPRI;
        gpio_poll.revents = 0;

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, SOCK_PATH, sizeof(addr.sun_path) - 1);

        if (OPTIONS_MASK & DEBUG_MODE)
        {
            printf("%*s\n", APP_C_PADDING, "Socket setting completed");
        }
        
        while (keep_running)
        {
            if (app->client_fd < 0)
            {
                app->client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
                if (app->client_fd < 0)
                {
                    printf("Failed to create socket: %m\n");
                }

                if (connect(app->client_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
                {
                    printf("Connection failed: %m\n");
                    close(app->client_fd);
                } else {
                    connected = 1;
                    if (OPTIONS_MASK & DEBUG_CAMERA_STREAM)
                    {
                        printf("%*s\n", APP_C_STREAM_PADDING, "stream: successfully connected to server");
                    }
                }
            }
            

            int ready = poll(&gpio_poll, 1, 1000);
            
            if (ready < 0)
            {
                printf("An error occured: %m\n");
            }

            else if (gpio_poll.revents & POLLPRI)
            {

                flag = !gpio_read(&app->fd_gpio, app->gpio_button);

                if (flag)
                {
                    
                    if (create_timestamp_name(app->output_filename) == -1){
                        printf("failed creating timestamp name\n");
                        int written = snprintf(app->output_filename, 64, "/oem/img/new_frame.raw");
                        if (written < sizeof(app->output_filename))
                        {
                            puts("failed setting default name");
                            continue;
                        }
                        
                    }

                    if (OPTIONS_MASK & DEBUG_CAMERA_STREAM)
                    {
                        printf("%*s\n", APP_C_STREAM_PADDING, "stream: created timestamp name");

                    }
                    

                    if (camera_capture_frame(&app->cam, app->output_filename) < 0)
                    {
                        fprintf(stderr, "failed to capture frame\n"); // continue anyway...
                        return 1;
                    }

                    if (DEBUG_MODE & DEBUG_CAMERA_STREAM)
                    {
                        printf("%*s\n", APP_C_STREAM_PADDING, "stream: camera capture frame done");
                    }
                    
                    

                    if (connected)
                    {
                        if (write(app->client_fd, app->output_filename, sizeof(app->output_filename)) == -1) {
                            printf("Write failed: %m\n");
                        }
                        else {
                            printf("Message sent successfully!\n");
                        }
                    }

                    if (DEBUG_MODE & DEBUG_CAMERA_STREAM)
                    {
                        printf("%*s\n", APP_C_STREAM_PADDING, "stream: message sent to the server successfully");
                    }
                }
            }
        }
    }
    else {
        goto end;
    }

    result = 0;
end:
    app->client_fd = -1;
    camera_stream_off(&app->cam);
    return result;
}

int app_cleanup(application * app) {    
    if (app == NULL)
    {
        return -1;
    }

    camera_cleanup_buffers(&app->cam);
    camera_off(&app->cam);
    gpio_unexport(app->gpio_button);
    gpio_close(&app->fd_gpio, app->gpio_button);
    
    return 0;
}