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
    snprintf(buf, sizeof(buf), "%d", gpio);
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
    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
    
    if (write_str(buf, data) < 0) {
        return -1;
    }
    return 0;
}

int gpio_open(int * const fd_ptr, char * const path, int oflag){
    //char path[64];
    int result = -1;
    *fd_ptr = open(path, oflag);
    if (*fd_ptr < 0) {
        fprintf(stderr, "open(%s): %s\n", path, strerror(errno));
        goto end;
    }
    result = 0;
end:
    return result;
}

int gpio_monitor_pin_value(int * const fd_ptr, int gpio, int oflag){
    char path[64];
    int result = -1;
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);
    if (gpio_open(fd_ptr, path, oflag) < 0){
        fprintf(stderr, "failed monitoring gpio%d value: %s\n", gpio, strerror(errno));
        goto end;
    }
    result = 0;
end:
    return result;
}

int gpio_read(int * const fd_ptr, int gpio)
{
    char c = '0';
    int result = -1;

    if (lseek(*fd_ptr, 0, SEEK_SET) == -1) {
        fprintf(stderr, "lseek(%s): gpio%d value\n", strerror(errno), gpio);
        goto end;
    }

    if (read(*fd_ptr, &c, 1) != 1) {
        fprintf(stderr, "read(%s): gpio%d value\n", strerror(errno), gpio);
        goto end;
    }
    
    result = (c == '1') ? 1 : 0;
end:
    return result;
// out:
    // return gpio_close(fd_ptr, result);
}

int gpio_close(int * const fd_ptr, int result){
    close(*fd_ptr);
    return result;
}