#include "app/app.h"
#include <signal.h>


// @TODO: state machine
int main(int argc, char* argv[]){
    OPTIONS_MASK |= DEBUG_MODE;
    OPTIONS_MASK |= DEBUG_CAMERA_STREAM;

    int load_flag = 0;
    const char *config_path;
// settings: (it's just like building a house)
    application app; // the most important structure, it contains almost everything...
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("Application init");
    }
    
    init_application(&app); // reseting all structure fields
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("Gpio configuration");
    }
    
    if (gpio_configuration(app.gpio_button) < 0) { // analyzing gpio state and configuring it
        printf("gpio%d configuration failed\n", app.gpio_button);
        goto cleanup; // just save it for now
    }

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("Camera configuration");
    }

    
    camera_config cfg;
    //memset(&cfg, 0, sizeof(cfg));

    if (argc == 2)
    {
        config_path = argv[1];
    }
    else {
        config_path = "/userdata/config.json";
    }
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("Load config from JSON");
    }
    
    
    if (load_config_from_json(config_path, &cfg) == 0) {
        if (OPTIONS_MASK & DEBUG_MODE)
        {
            puts("Camera init from config");
        }
        
        if (app_camera_init_from_config(&app.cam, &cfg) < 0)
        {
            load_flag = 1;
            
        }
    }

    if (!load_flag) {
        if (OPTIONS_MASK & DEBUG_MODE)
        {
            puts("Camera init by default");
        }

        if (app_camera_config_default(&app) < 0)
        {
            fprintf(stderr, "application config failed: %s\n", strerror(errno));
            goto cleanup;
        }
        
    }
    

// prepare: (it's just like intention to do something with this house)

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("Camera prepare buffers");
    }
    

    if (app_prepare_camera_buffers(&app) < 0)
    {
        fprintf(stderr, "failed to prepare buffers for camera usage\n");
        goto cleanup;
    }
// action:

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("Camera run streaming loop");
    }
    

    if (app_run_camera_stream(&app) < 0)
    {
        printf("camera streaming loop stopped\n");
    }
cleanup:
    app_cleanup(&app);
    return 0;
}