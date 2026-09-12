#include "../device/camera.h"
#include "../gpio/gpio_manager.h"
#include "../json/config.h"

/*

    app.c logic will help make state machine in main.c

*/

typedef struct {
    camera cam;
    char output_filename[64];
    // char config_path[32]; hardcoded now but in future it will be console arg
    char device[32];
    int gpio_button;
    int fd_gpio;
    int is_opened;
} application;

int init_application(application *app); // just reset the application structure
int load_config_from_json(const char * const config_path, camera_config * const cfg); // trying to load config from .json
int app_camera_init_from_config(camera * const c, camera_config * const config); // configure camera from config
int app_camera_config_default(application *app); // using when it's impossible to use .json config
int app_prepare_camera_buffers(application *app); // query-->mmap-->queue
int app_run_camera_stream(application *app); // stream on-->running--> stream off in case of failure or interruption
int app_cleanup(application *app); // you know :)