#include "gpio_manager.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int write_str(const char *path, const char *s){
    int fd = open(path, O_WRONLY);
    int result = -1;
    if (fd < 0) {
        fprintf(stderr, "failed opening file %s : %s\n", path, strerror(errno));
        goto end;
    }

    if (write(fd, s, strlen(s)) < 0){
        fprintf(stderr, "failed writing in file %s : %s\n", path, strerror(errno));
        goto end;
    }
    result = 0;
end:
    close(fd);
    return result;
}


int gpio_export(int gpio){
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", gpio);
    if (write_str("/sys/class/gpio/export", buf) < 0){
        if (errno == EBUSY) return 0;
        return -1;
    }
    usleep(500000);
    return 0;
}


int gpio_unexport(int gpio){
    char buf[32];
    snprintf(buf, strlen(buf), "%d", gpio);
    if (write_str("/sys/class/gpio/unexport", buf) < 0){
        if (errno == EBUSY) return 0;
        return -1;
    }
    return 0;
}

int gpio_direction(int gpio, char const * dir){
    char buf[64];
    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);

    if (write_str(buf, dir) < 0) return -1;

    return 0;
}

int gpio_write(int gpio, char *const data){
    char buf[32];
    snprintf(buf, strlen(buf), "/sys/class/gpio/gpio%d/value", gpio);
    
    if (write_str(buf, data) < 0) {
        return -1;
    }
    return 0;
}

int gpio_read(int gpio)
{
    char path[64];
    char c = '0';
    int fd, ret = -1;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open(%s): %s\n", path, strerror(errno));
        return -1;
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        fprintf(stderr, "lseek(%s): %s\n", path, strerror(errno));
        goto out;
    }

    if (read(fd, &c, 1) != 1) {
        fprintf(stderr, "read(%s): %s\n", path, strerror(errno));
        goto out;
    }
    
    ret = (c == '1') ? 1 : 0;

out:
    close(fd);
    return ret;
}