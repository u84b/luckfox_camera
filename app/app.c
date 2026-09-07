#include "app.h"
#include <stdio.h>
#include <string.h>


int init_application(camera_application *app){
    
    memset(&app->cam, 0, sizeof(app->cam));
    memset(&app->format, 0, sizeof(app->format));
    memset(&app->buf_cfg, 0, sizeof(app->buf_cfg));

    app->fd_gpio = -1;
    app->is_opened = -1;

    return 0;
}

// @TODO: complete all the options that can be added to the config.
// @TODO: it's time to think about error handling during format setting
int config_application_from_json(camera_application *app){
    cJSON* json_tree = {0};
    cJSON* camera_params = {0};
    char *buf = NULL;
    int width = 0;
    int height = 0;
    int memory_type = 0;
    int result = -1;

    json_tree = parse_file(CAMERA_CONFIG_JSON_PATH);

    if (json_tree == NULL) {
        printf("Parsing file %s failed\n", CAMERA_CONFIG_JSON_PATH);
        return -1; // then I will create another option for this case, I meant config_application_defau
    }

    camera_params = cJSON_GetObjectItem(json_tree, "camera");
    
    buf = cJSON_GetObjectItem(json_tree, "device_path")->valuestring;
    
    int written = snprintf(app->device, 32, "%s", buf);
    
    if (written < 0){
        fprintf(stderr, "snprintf error : %s", strerror(errno));
        return -1;
    }

    width = cJSON_GetObjectItem(json_tree, "width")->valueint;
    height = cJSON_GetObjectItem(json_tree, "height")->valueint;
    memory_type = cJSON_GetObjectItem(json_tree, "memory_type")->valueint;

    if (camera_init(&app->cam) < 0){
        fprintf(stderr, "camera init failed: %s\n", strerror(errno));
        goto end;
    } // initializing general V4L2 structures
    
    puts("Camera init started...");
    
    if (camera_open_video_interface(&app->cam, app->device) < 0) {
        fprintf(stderr, "failed opening camera interface\n");
        goto end;
    } // opening /dev/video11 file
    
    puts("Opened camera interface...");

    if (camera_check_capabilities(&app->cam) < 0) {
        fprintf(stderr, "v4l2 capabilities failed\n");
        goto end;
    }; // checking capabilities of V4L2
    puts("Checked camera capabilities\n");
    camera_set_type(&app->cam, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE); // setting type for future tasks related to buffer and &app->format configuration
    
    format_set_frame_size(&app->format, width, height); // setting frame size for camera capture
    format_set_pixel_format(&app->format, V4L2_PIX_FMT_NV12);
    format_set_field(&app->format, V4L2_FIELD_NONE);

    if (camera_set_format(&app->cam, *&app->format) < 0){
        fprintf(stderr, "failed setting camera &app->format\n");
        goto end;
    } // copying &app->format structure to camera
    puts("Successfully set camera &app->format...\n");
    buffer_config_set_count(&app->buf_cfg, BUFFER_COUNT);
    buffer_config_set_memory(&app->buf_cfg, memory_type);
    if (camera_set_buffer_config(&app->cam, app->buf_cfg) < 0){
        fprintf(stderr, "failed setting buffer configuration\n");
        goto end;
    } // setting buffer configuration for correct work with memory
    puts("Successfully set camera buffer configuration\n");
// preparations:
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
    if (json_tree != NULL) {
        cJSON_Delete(json_tree);
    }
    return result;
}

// @TODO: add functionality and integrate it into the existing system
int config_application_default(camera_application *app){
    return 0;
};