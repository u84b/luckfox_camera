#include "../json/config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

void print_json(char *tree){
    if (tree != NULL) printf("%s\n", tree);
}

int main(){
    cJSON *tree = {0};
    cJSON *camera_params = {0};

    //cJSON *camera_params = {0};
    const char *filename = "example.json";

    char *device_path = NULL;

    char *actual = NULL;

    char *params = NULL;

    uint32_t width = 0;

    uint32_t height = 0;

    tree = parse_file(filename);

    if (tree == NULL) {
        printf("Parsing file %s failed\n", filename);
    }

    actual = cJSON_Print(tree);

    camera_params = cJSON_GetObjectItem(tree, "camera");
    params = cJSON_Print(camera_params);

    print_json(params);
    
    device_path = cJSON_GetObjectItem(camera_params, "device_path")->valuestring;
    printf("%s\n", device_path);
    width = cJSON_GetObjectItem(camera_params, "width")->valueint;
    printf("%d\n", width);
    height = cJSON_GetObjectItem(camera_params, "height")->valueint;
    printf("%d\n", height);
    
    free(params);
    free(actual);

    //cJSON_Delete(camera_params);
    cJSON_Delete(tree);

    return 0;
}