#include "../device/camera.h"
#include "../gpio/gpio_manager.h"
#include "../json/config.h"

/*

    app.c logic will help make state machine in main.c

*/

typedef struct {

    camera cam;
    camera_format format;
    char output_filename[64];
    // char config_path[32]; hardcoded now but in future it will be console arg
    char device[32];
    camera_buffer_config buf_cfg;
    int fd_gpio;
    int is_opened;
} camera_application;

int init_application(camera_application *app);
int config_application_from_json(camera_application *app);
int config_application_default(camera_application *app); // using when it's impossible to use json config
int run_application(camera_application *app);
int cleanup_application(camera_application *app); // the most harsh part, I need more time to think it over