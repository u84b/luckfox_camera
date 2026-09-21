#include "app/app.h"
#include <signal.h>



int main(){  

// settings: (it's just like building a house)
    application app; // the most important structure, it contains almost everything...
    puts("Application init");
    init_application(&app); // reseting all structure fields
    puts("Gpio configuration");
    if (gpio_configuration(app.gpio_button) < 0) { // analyzing gpio state and configuring it
        printf("gpio%d configuration failed\n", app.gpio_button);
        return -1; // just save it for now
    }
    puts("Camera configuration");
    camera_config cfg;
    //memset(&cfg, 0, sizeof(cfg));
    const char *config_path = "/userdata/config.json";
    puts("Load config from JSON");
    if (load_config_from_json(config_path, &cfg) < 0) {
        fprintf(stderr, "failed to load camera config from json\n");
        if (app_camera_config_default(&app) < 0){ 
            puts("Default configuration");
            fprintf(stderr, "application config failed: %s\n", strerror(errno));    
            return -1;
        }
    }
    puts("Camera init from config");
    if (app_camera_init_from_config(&app.cam, &cfg) < 0)
    {
        fprintf(stderr, "failed to init camera from config");
        if (app_camera_config_default(&app) < 0){ 
            puts("Camera init default");
            fprintf(stderr, "application config failed: %s\n", strerror(errno));    
            return -1;
        }
    }
// prepare: (it's just like intention to do something with this house)
    if (app_prepare_camera_buffers(&app) < 0)
    {
        fprintf(stderr, "failed to prepare buffers for camera usage\n");
        return -1;
    }
// action:
    if (app_run_camera_stream(&app) < 0)
    {
        printf("camera streaming loop stopped\n");
    }
    
    app_cleanup(&app);
    return 0;
}