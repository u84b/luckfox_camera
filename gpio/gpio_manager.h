#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int write_str(const char *path, const char *s);
int gpio_export(int gpio);
int gpio_unexport(int gpio);
int gpio_direction(int gpio, char  const * dir);
int gpio_write(int gpio, char * const data);
int gpio_read(int gpio);
