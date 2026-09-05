#include "../json/lib/cJSON.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>

char* read_file(const char *filename) {
    FILE *file = NULL;
    long length = 0;
    char *content = NULL;
    size_t read_chars = 0;

    /* open in read binary mode */
    file = fopen(filename, "rb");
    if (file == NULL)
    {
        goto cleanup;
    }

    /* get the length */
    if (fseek(file, 0, SEEK_END) != 0)
    {
        goto cleanup;
    }
    length = ftell(file);
    if (length < 0)
    {
        goto cleanup;
    }
    if (fseek(file, 0, SEEK_SET) != 0)
    {
        goto cleanup;
    }

    /* allocate content buffer */
    content = (char*)malloc((size_t)length + sizeof(""));
    if (content == NULL)
    {
        goto cleanup;
    }

    /* read the file into memory */
    read_chars = fread(content, sizeof(char), (size_t)length, file);
    if ((long)read_chars != length)
    {
        free(content);
        content = NULL;
        goto cleanup;
    }
    content[read_chars] = '\0';


cleanup:
    if (file != NULL)
    {
        fclose(file);
    }

    return content;
}

static cJSON *parse_file(const char *filename)
{
    cJSON *parsed = NULL;
    char *content = read_file(filename);

    parsed = cJSON_Parse(content);

    if (content != NULL)
    {
        free(content);
    }

    return parsed;
}

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