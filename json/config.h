#include "lib/cJSON.h"
#include "lib/cJSON_Utils.h"
#include <stdio.h>
#include <stdlib.h>

#define CAMERA_CONFIG_JSON_PATH "/userspace/config.json"

char* read_file(const char *filename);

cJSON *parse_file(const char *filename);

