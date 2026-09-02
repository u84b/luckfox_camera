#ifndef CAMERA_H
#define CAMERA_H

#include "v4l2_utils.h"

#include <bits/types.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define PLANE_COUNT  VIDEO_MAX_PLANES
#define BUFFER_COUNT 2
#define POLL_TIMEOUT 5000
#define SKIP_FRAMES  9

enum {
    FLAG_DEBUG = 0,
    FLAG_LOG = 1,
    FLAG_ERR = 2
};

#define MASK_DEBUG (1U << FLAG_DEBUG)
#define MASK_LOG (1U << FLAG_LOG)
#define MASK_ERR (1U << FLAG_ERR)

struct buffer {
    void *addr[PLANE_COUNT];
    size_t length[PLANE_COUNT];
};

typedef struct {
    struct v4l2_format format;
} camera_format;

typedef struct {
    struct v4l2_requestbuffers buf_config;
} camera_buffer_config;

typedef struct {
    struct buffer buffers[BUFFER_COUNT];
    camera_format format;
    struct v4l2_capability cap;
    camera_buffer_config cfg;
    int fd;
    int stream_started;
    uint32_t buffer_count;
    uint32_t plane_count;
    uint32_t OPTIONS_MASK;
    enum v4l2_buf_type type;
} camera;



/*
CAMERA FORMAT PROCEDURES
*/
void format_set_format(camera_format const * c_format, struct v4l2_format *format);
void format_set_frame_size(camera_format * const c_format, const uint32_t width, const uint32_t height);
void format_set_pixel_format(camera_format * const c_format, const uint32_t pixel_format);
void format_set_field(camera_format * const c_format, const uint32_t field);

/*
BUFFER CONFIGURATION PROCEDURES
*/
void buffer_config_set_count(camera_buffer_config * const buf_cfg, uint32_t count);
void buffer_config_set_memory(camera_buffer_config * const buf_cfg, uint32_t memory);


/*
CAMERA PROCEDURES
*/
int camera_init(camera * const c);
int camera_open_video_interface(camera * const c, const char * const filename);
void camera_set_type(camera * const c, const uint32_t type);
int camera_check_capabilities(camera * const c);
int camera_set_format(camera * const c, camera_format c_format);
int camera_set_buffer_config(camera * const c, camera_buffer_config buf_cfg);
int camera_map_buffers(camera * const c);
int camera_queue_buffers(camera * const c);
void camera_cleanup_buffers(camera * const c);
int camera_stream_on(camera * const c);
int camera_capture_frame(camera * const c, const char * const output);
void camera_stream_off(camera * const c);
int camera_off(camera * const c);


void cleanup(camera * const c);

// ADDITIONAL
int save_frame(camera * const c,
    const char *filename,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes);

#endif
