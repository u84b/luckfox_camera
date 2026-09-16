#include "app.h"
#include <stdio.h>
#include <string.h>
#include <time.h>


// init_application (gpio_configuration, load_config_from_json -> app_camera_init_from_config 
//                         || app_camera_init_default)
// 

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

int init_application(application *app){
    
    memset(&app->cam, 0, sizeof(app->cam));
    // memset(&app->format, 0, sizeof(app->format));
    // memset(&app->buf_cfg, 0, sizeof(app->buf_cfg));

    app->gpio_button = 54;
    app->fd_gpio = -1;
    app->is_opened = -1;


    return 0;
}

int load_config_from_json(const char * const config_path, camera_config * cfg){

    if (config_path == NULL) return 1; // okay, not fatal, because we can use default configuration

    if (cfg == NULL) return -1;
    
    memset(&cfg, 0, sizeof(cfg));
    
    cJSON* json_tree = {0};
    cJSON* camera_params = {0};
    char *buf = NULL;
    int result = -1;

    json_tree = parse_file(config_path);

    if (json_tree == NULL)
    {
        printf("Parsing file %s failed\n", config_path);
        goto end;
    }
    
    camera_params = cJSON_GetObjectItem(json_tree, "camera");

    buf = cJSON_GetObjectItem(camera_params, "device_path")->valuestring;

    int written = snprintf(cfg->device_path, 32, "%s", buf);

    if (written < 0){
        fprintf(stderr, "snprintf error : %s", strerror(errno));
        goto end;
    }

    cfg->format.format.fmt.pix_mp.width = cJSON_GetObjectItem(camera_params, "width")->valueint;
    cfg->format.format.fmt.pix_mp.height = cJSON_GetObjectItem(camera_params, "height")->valueint;
    cfg->format.format.fmt.pix_mp.pixelformat = cJSON_GetObjectItem(camera_params, "pixelFormat")->valueint;
    cfg->buf_cfg.buf_config.memory = cJSON_GetObjectItem(camera_params, "memoryType")->valueint;
    cfg->buf_cfg.buf_config.count = cJSON_GetObjectItem(camera_params, "bufferCount")->valueint;

    if (json_tree != NULL)
    {
        cJSON_Delete(json_tree);
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
    
    if (camera_open_video_interface(c, config->device_path) < 0) {
        fprintf(stderr, "failed opening camera interface\n");
        return -1;
    }

    if (camera_check_capabilities(c) < 0) {
        fprintf(stderr, "v4l2 capabilities failed\n");
        return -1;
    } // checking capabilities of V4L2
    
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
    

    return 0;
}

// @TODO: complete all the options that can be added to the config.
// @TODO: it's time to think about error handling during format setting




// @TODO: add functionality and integrate it into the existing system
// hardcoded config version
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

    if (camera_open_video_interface(&app->cam, video_node) < 0) {
        fprintf(stderr, "failed opening camera interface\n");
        goto end;
    }

    if (camera_check_capabilities(&app->cam) < 0) {
        fprintf(stderr, "v4l2 capabilities failed\n");
        goto end;
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

    result = 0;
end:
    return result;
};

int app_prepare_camera_buffers(application * app){
    int result = -1;

    if (app == NULL) goto end;

    if (camera_map_buffers(&app->cam) < 0) {
        fprintf(stderr, "failed mapping of camera buffers\n");
        goto end;
    } // mmap usage here
    
    if (camera_queue_buffers(&app->cam) < 0) {
        fprintf(stderr, "failed queue camera buffers");
        goto end;
    }
    result = 0;
end:
    return result;
}

int app_run_camera_stream(application * app){
    int result = -1;
    camera_stream_on(&app->cam);

    app->is_opened = gpio_monitor_pin_value(&app->fd_gpio, app->gpio_button, O_RDONLY);

    if (app->is_opened == 0) {
        while (1) // soon I'll change it, but now we have what we have
        {
            if (gpio_read(&app->fd_gpio, app->gpio_button) == 0)
            {
                create_timestamp_name(app->output_filename);
                if (camera_capture_frame(&app->cam, app->output_filename) < 0)
                {
                    fprintf(stderr, "failed to capture frame\n"); // continue anyway...
                    return 1;
                }
                
            }
            usleep(20000);
        }
    }
    else {
        goto end;
    }

    result = 0;
end:
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