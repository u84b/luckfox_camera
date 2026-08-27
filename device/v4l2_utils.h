#ifndef V4L2_UTILS_H
#define V4L2_UTILS_H

#include <linux/videodev2.h>

int check_if_null(const void *ptr, const char *name);

// VIDIOC_QUERYCAP usage
int v4l2_query_capability(
    int fd,
    struct v4l2_capability *cap);

// VIDIOC_S_FMT usage
int v4l2_set_format(
    int fd,
    struct v4l2_format *fmt);

// VIDIOC req_buf usage
int v4l2_request_buffers(
    int fd,
    struct v4l2_requestbuffers *req);

// VIDIOC_QUERYBUF usage
int v4l2_query_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes);

// VIDIOC_QBUF usage
int v4l2_queue_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes);
// VIDIOC_DQBUF usage
int v4l2_dequeue_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes);

// VIDIOC_STREAMON usage
int v4l2_stream_on(
    int fd,
    enum v4l2_buf_type type);
// VIDIOC_STREAMOFF usage
int v4l2_stream_off(
    int fd,
    enum v4l2_buf_type type);

#endif
