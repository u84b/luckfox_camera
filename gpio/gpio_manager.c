#include "gpio_manager.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

int write_str(const char *path, const char *s) {
    int fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "failed opening file %s : %s\n", path, strerror(errno));
        return -1;
    }

    size_t len = strlen(s);
    size_t written = 0;

    while (written < len) {
        ssize_t res = write(fd, s + written, len - written);
        if (res < 0) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "failed writing in file %s : %s\n", path, strerror(errno));
            close(fd);
            return -1;
        }
        written += (size_t)res;
    }

    close(fd);
    return 0;
}

int gpio_export(int gpio) {
    char buf[32];
    int written = snprintf(buf, sizeof(buf), "%d", gpio);
    
    if (written < 0 || (size_t)written >= sizeof(buf)) {
        fprintf(stderr, "gpio number %d is too large or invalid\n", gpio);
        return -1;
    }

    int res = write_str("/sys/class/gpio/export", buf);
    
    if (res < 0) {
        if (errno == EBUSY) {
            return 0; // pin was already exported
        }
        return -1;
    }

    // just for kernel
    usleep(100000);
    return 0;
}

// unexport GPIO (cleaunp)
int gpio_unexport(int gpio) {
    char buf[32];
    int written = snprintf(buf, sizeof(buf), "%d", gpio);
    if (written < 0 || (size_t)written >= sizeof(buf)) {
        return -1;
    }

    int res = write_str("/sys/class/gpio/unexport", buf);
    if (res < 0) {
        if (errno == ENOENT) {
            return 0; // pin was already unexported
        }
        return -1; // real failure
    }
    return 0;
}

int gpio_direction(int gpio, char const * dir){
    char buf[64];
    int written = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", gpio);

    if (written < 0 || (size_t)written >= sizeof(buf)) {
        return -1;
    }

    if (write_str(buf, dir) < 0) {
        if (errno == ENOENT)
            return 0;
        return -1;
    }

    return 0;
}

int gpio_write(int gpio, char *const data){
    char buf[32];
    int written = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
    
    if (written < 0 || (size_t)written >= sizeof(buf)) {
        return -1;
    }

    if (write_str(buf, data) < 0) {
        if (errno == ENOENT)
            return 0;
        
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