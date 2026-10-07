#include "app/app.h"


// @TODO: state machine
int main(int argc, char* argv[]){

    int load_flag = 0;
    const char *config_path;
// settings: (it's just like building a house)
    application app; // the most important structure, it contains almost everything...
    
    set_options();

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("==== APPLICATION INIT");
    }
    
    init_application(&app); // reseting all structure fields
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("==== GPIO CONFIGURATION");
    }
    
    if (gpio_configuration(app.gpio_button) < 0) { // analyzing gpio state and configuring it
        printf("gpio%d configuration failed\n", app.gpio_button);
        goto cleanup; // just save it for now
    }

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("==== CAMERA CONFIGURATION");
    }

    
    camera_config cfg;

    if (argc == 2)
    {
        config_path = argv[1];
    }
    else {
        config_path = "/userdata/config.json";
    }
    
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("==== LOAD CONFIG FROM JSON");
    }
    
    
    if (load_config_from_json(config_path, &cfg) == 0) {
        if (OPTIONS_MASK & DEBUG_MODE)
        {
            puts("==== CAMERA INIT FROM CONFIG");
        }
        
        if (app_camera_init_from_config(&app.cam, &cfg) < 0)
        {
            printf("camera init from config went wrong: %m\n");
            
        } else {
            load_flag = 1;
        }
    }

    printf("    CURRENT LOAD FLAG: %d\n", load_flag);

    if (!load_flag) {
        if (OPTIONS_MASK & DEBUG_MODE)
        {
            puts("==== CAMERA INIT BY DEFAULT");
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
        puts("==== CAMERA PREPARE BUFFERS");
    }
    

    if (app_prepare_camera_buffers(&app) < 0)
    {
        fprintf(stderr, "failed to prepare buffers for camera usage\n");
        goto cleanup;
    }
// action:

    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("==== CAMERA RUN STREAMING LOOP");
    }
    

    if (app_run_camera_stream(&app) < 0)
    {
        printf("camera streaming loop stopped\n");
    }
cleanup:
    if (OPTIONS_MASK & DEBUG_MODE)
    {
        puts("==== APPLICATION CLEANUP");
    }
    
    app_cleanup(&app);
    return 0;
}
