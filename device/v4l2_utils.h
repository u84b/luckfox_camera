#ifndef V4L2_UTILS_H
#define V4L2_UTILS_H

#include <linux/videodev2.h>

int check_if_null(const void *ptr, const char *name);

int v4l2_query_capability(
    int fd,
    struct v4l2_capability *cap);

int v4l2_set_format(
    int fd,
    struct v4l2_format *fmt);

int v4l2_request_buffers(
    int fd,
    struct v4l2_requestbuffers *req);

int v4l2_query_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes);

int v4l2_queue_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes);

int v4l2_dequeue_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes);

int v4l2_stream_on(
    int fd,
    enum v4l2_buf_type type);

int v4l2_stream_off(
    int fd,
    enum v4l2_buf_type type);

#endif
