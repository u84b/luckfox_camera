#include "app/app.h"
#include <signal.h>

volatile sig_atomic_t keep_running = 1;

void handle_termination(int signum){
    if (signum == SIGTERM){
        keep_running = 0;
    }
}

int main(){  
    signal(SIGTERM, handle_termination);
    signal(SIGINT, handle_termination);
// settings: (it's just like building a house)
    application app; // the most important structure, it contains almost everything...
    
    init_application(&app); // reseting all structure fields
    
    if (gpio_configuration(app.gpio_button) < 0) { // analyzing gpio state and configuring it
        printf("gpio%d configuration failed\n", app.gpio_button);
        return -1; // just save it for now
    }
    
    camera_config cfg;
    memset(&cfg, 0, sizeof(cfg));
    const char *config_path = "/userdata/config.json";
    
    if (load_config_from_json(config_path, &cfg) < 0) {
        fprintf(stderr, "failed to load camera config from json\n");
        if (app_camera_config_default(&app) < 0){ 
            fprintf(stderr, "application config failed: %s\n", strerror(errno));    
            return -1;
        }
    }

    if (app_camera_init_from_config(&app.cam, &cfg) < 0)
    {
        fprintf(stderr, "failed to init camera from config");
        if (app_camera_config_default(&app) < 0){ 
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