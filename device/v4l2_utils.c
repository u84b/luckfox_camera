#define _FILE_OFFSET_BITS 64

#include "v4l2_utils.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

// ioctl with error handling for v4l2 driver
static int xioctl(
    int fd,
    unsigned long request,
    void *arg,
    const char *request_name)
{
    int ret;

    do {
        ret = ioctl(fd, request, arg);
    } while (ret == -1 && errno == EINTR);

    if (ret == -1) {
        fprintf(stderr,
                "V4L2: %s failed: %s\n",
                request_name,
                strerror(errno));
    }

    return ret;
}

// check if null pointer
int check_if_null(const void *ptr, const char *name)
{
    if (ptr == NULL) {
        fprintf(stderr, "V4L2: NULL pointer: %s\n", name);
        errno = EINVAL;
        return -1;
    }

    return 0;
}
//
int v4l2_query_capability(
    int fd,
    struct v4l2_capability *cap)
{
    if (check_if_null(cap, "cap") < 0) // null pointer check
        return -1;

    return xioctl(
        fd,
        VIDIOC_QUERYCAP,
        cap,
        "VIDIOC_QUERYCAP"
    );
}

int v4l2_set_format(
    int fd,
    struct v4l2_format *fmt)
{
    if (check_if_null(fmt, "fmt") < 0) // null pointer check
        return -1;

    return xioctl(
        fd,
        VIDIOC_S_FMT,
        fmt,
        "VIDIOC_S_FMT"
    );
}

int v4l2_request_buffers(
    int fd,
    struct v4l2_requestbuffers *req)
{
    int result = 0;

    if (check_if_null(req, "req") < 0) result = -1; // null pointer check
        
    fprintf(stderr,
        "REQBUFS: count=%u type=%u memory=%u capabilities=0x%x\n",
        req->count,
        req->type,
        req->memory,
        req->capabilities);

    if (xioctl(fd,VIDIOC_REQBUFS,
        req,"VIDIOC_REQBUFS") < 0) {
        result = -1;
    }

    if (req->count == 0) {
        fprintf(stderr,"V4L2: driver returned zero buffers\n");
        errno = ENOBUFS;
        result = -1;
    }

    return result;
}

int v4l2_query_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes)
{
    if (check_if_null(buf, "buf") < 0)
        return -1;

    if (buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ||
        buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {

        if (check_if_null(planes, "planes") < 0)
            return -1;

        buf->m.planes = planes;
        }

    return xioctl(fd,VIDIOC_QUERYBUF,
        buf,"VIDIOC_QUERYBUF");
}

int v4l2_queue_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes)
{
    if (check_if_null(buf, "buf") < 0)
        return -1;

    if (buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE || buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
            if (check_if_null(planes, "planes") < 0) return -1;
            buf->m.planes = planes;
    }
    return xioctl(fd,VIDIOC_QBUF,
            buf,"VIDIOC_QBUF");
}

int v4l2_dequeue_buffer(
    int fd,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes)
{
    if (check_if_null(buf, "buf") < 0)
        return -1;

    if (buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE ||
        buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {

        if (check_if_null(planes, "planes") < 0)
            return -1;

        buf->m.planes = planes;
    }

    return xioctl(fd,VIDIOC_DQBUF,
            buf,"VIDIOC_DQBUF");
}

int v4l2_stream_on(
    int fd,
    enum v4l2_buf_type type)
{
    return xioctl(
        fd,
        VIDIOC_STREAMON,
        &type,
        "VIDIOC_STREAMON"
    );
}

int v4l2_stream_off(
    int fd,
    enum v4l2_buf_type type)
{
    return xioctl(
        fd,
        VIDIOC_STREAMOFF,
        &type,
        "VIDIOC_STREAMOFF"
    );
}
