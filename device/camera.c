#include "camera.h"

int save_frame(camera * const c,
    const char *filename,
    struct v4l2_buffer *buf,
    struct v4l2_plane *planes) // !
{
    FILE *file;
    unsigned int p;

    if (c == NULL ||
        filename == NULL ||
        buf == NULL ||
        planes == NULL) {
        errno = EINVAL;
    return -1;
        }

    file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }

    for (p = 0; p < c->plane_count; p++) {
        size_t bytes_used;

        bytes_used = planes[p].bytesused;

        if (bytes_used == 0)
            continue;

        if (fwrite(c->buffers[buf->index].addr[p],
            1,bytes_used,file) != bytes_used) {
            perror("fwrite");
            fclose(file);
            return -1;
        }
    
        if (c->OPTIONS_MASK & 1) printf("Plane %u: %u bytes\n", p, planes[p].bytesused);
    
    }

        if (fclose(file) != 0) {
            perror("fclose");
            return -1;
        }

        if (c->OPTIONS_MASK & 1) printf("Frame saved to %s\n", filename);

        return 0;
}

static int wait_for_frame(int fd)
{
    struct pollfd pfd;
    int result = -1;

    memset(&pfd, 0, sizeof(pfd));

    pfd.fd = fd;
    pfd.events = POLLIN;

    if (poll(&pfd, 1, POLL_TIMEOUT) < 0) {
        perror("poll");
        goto end;
    }

    if (pfd.revents & POLLERR) {
        fprintf(stderr, "Camera stream error\n");
        goto end;
    }

    if (pfd.revents & POLLHUP) {
        fprintf(stderr, "Camera stream hangup\n");
        goto end;
    }

    if (!(pfd.revents & POLLIN)) {
        fprintf(stderr, "Camera frame timeout\n");
        errno = ETIMEDOUT;
        goto end;
    }
    result = 0;
end:
    return result;
}


int camera_init(camera * const c){ //camera init start
    c->fd = -1;
    c->stream_started = 0;
    //c->result = EXIT_FAILURE;
    c->buffer_count = 0;
    c->plane_count = 0;
    c->OPTIONS_MASK |= MASK_DEBUG;


    memset(&c->cap, 0, sizeof(c->cap));
    memset(&c->format, 0, sizeof(c->format));
    memset(&c->cfg, 0, sizeof(c->cfg));
    memset(c->buffers, 0, sizeof(c->buffers));

    return 0;
};

void camera_cleanup_buffers(camera * const c) // necessary for cleanup and closing camera
{
    uint32_t i;
    uint32_t p;


    for (i = 0; i < c->buffer_count; i++) {
        for (p = 0; p < c->plane_count; p++) {
            if (c->buffers[i].addr[p] != NULL &&
                c->buffers[i].addr[p] != MAP_FAILED) {
                munmap(
                    c->buffers[i].addr[p],
                    c->buffers[i].length[p]
                );
                }
        }
    }
}


// @TODO: it's relevant to make it more efficient/able to handle more errno
// @TODO: think about cleanup...
int camera_open_video_interface(camera * const c, const char * const filename){
    c->fd = open(filename, O_RDWR);
    if (c->fd < 0) {
        perror("open");
        cleanup(c);
        return -1;
    }
    return 0;
};

int camera_check_capabilities(camera * const c){
    int result = -1;
    if (v4l2_query_capability(c->fd, &c->cap) < 0){
        goto cleanup;
    }
        
    /*              
    ===============DEBUG INFO================
    */
    if (c->OPTIONS_MASK & MASK_DEBUG) {
        printf("Driver: %s\n", c->cap.driver);
        printf("Card:   %s\n", c->cap.card);
        printf("Bus:    %s\n", c->cap.bus_info);
    }

    if (!(c->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)) {
        fprintf(stderr, "Device does not support multi-planar capture\n");
        goto cleanup;
    }

    if (!(c->cap.capabilities & V4L2_CAP_STREAMING)) {
        fprintf(stderr, "Device does not support streaming I/O\n");
        goto cleanup;
    }
    result = 0;

cleanup:
    cleanup(c);
    return result;
}



void camera_set_type(camera  * const c, const uint32_t type){
    c->type = type;
}

int camera_set_format(camera * const c, camera_format c_format){
    c_format.format.type = c->type;
    c->format = c_format;
    int result = -1;

    if (v4l2_set_format(c->fd, &c->format.format) < 0){
        goto end;
    }
        

    c->plane_count = c->format.format.fmt.pix_mp.num_planes;

    if (c->plane_count == 0 || c->plane_count > PLANE_COUNT) {
        fprintf(stderr, "Invalid plane count: %u\n", c->plane_count);
        goto end;
    }

    if (c->OPTIONS_MASK & MASK_DEBUG) {
        printf(
            "Format: %c%c%c%c\n",
            c->format.format.fmt.pix_mp.pixelformat & 0xff,
            (c->format.format.fmt.pix_mp.pixelformat >> 8) & 0xff,
               (c->format.format.fmt.pix_mp.pixelformat >> 16) & 0xff,
               (c->format.format.fmt.pix_mp.pixelformat >> 24) & 0xff
        );

        printf(
        "Size:   %ux%u\n",
        c->format.format.fmt.pix_mp.width,
        c->format.format.fmt.pix_mp.height
        );

        printf("Planes: %u\n", c->plane_count);
    }    
    
    result = 0;
end:
    cleanup(c);
    return result;
}   

int camera_set_buffer_config(camera * const c, camera_buffer_config buf_cfg){
    buf_cfg.buf_config.type = c->type;
    c->cfg = buf_cfg;
    
    int result = -1;

    if (v4l2_request_buffers(c->fd, &c->cfg.buf_config) < 0){
        goto end;
    }
        

    c->buffer_count = c->cfg.buf_config.count;

    if (c->buffer_count > BUFFER_COUNT) {
        fprintf(stderr,
                "Too many buffers returned: %u\n",
                c->buffer_count);
        goto end;
    }

end:
    cleanup(c);
    return result;
}

int camera_map_buffers(camera * const c){
    struct v4l2_plane planes[PLANE_COUNT];
    struct v4l2_buffer buf;

    for (uint32_t i = 0; i < c->buffer_count; i++) {

        memset(&buf, 0, sizeof(buf));
        memset(planes, 0, sizeof(planes));

        buf.type = c->type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        buf.length = PLANE_COUNT;
        buf.m.planes = planes;
        
        /*
            Get parameters of current buffer
        */
        
        if (v4l2_query_buffer(c->fd, &buf, planes) < 0){
            cleanup(c);
            return -1;
        }

        for (int p = 0; p < c->plane_count; p++) {
            c->buffers[i].length[p] = planes[p].length;
            
            /*
                Getting address of buffer
            */
            
            c->buffers[i].addr[p] = mmap(NULL,planes[p].length,PROT_READ | PROT_WRITE,
                MAP_SHARED,c->fd,planes[p].m.mem_offset
            );

            if (c->buffers[i].addr[p] == MAP_FAILED) {
                perror("mmap");
                c->buffers[i].addr[p] = NULL;
                cleanup(c);
                return -1;
            }
        }
    }
    return 0;
}

int camera_queue_buffers(camera *const c){
    
    struct v4l2_plane planes[PLANE_COUNT];
    struct v4l2_buffer buf;

    for (uint32_t i = 0; i < c->buffer_count; i++) {

        memset(&buf, 0, sizeof(buf));
        memset(planes, 0, sizeof(planes));

        buf.type = c->type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;
        buf.length = PLANE_COUNT;
        buf.m.planes = planes;

        if (v4l2_queue_buffer(c->fd, &buf, planes) < 0){
            cleanup(c);
            return -1;
        }
    }

    return 0;
}

int camera_stream_on(camera * const c){
    if (v4l2_stream_on(c->fd, c->type) < 0){
        return -1;
    }

    c->stream_started = 1;

    return 0;
}
// @TODO: determine which way of function termination/error handling is better
// @TODO: optimize it including main.c context (while-loop)
int camera_capture_frame(camera * const c, const char * const output){
    struct v4l2_plane planes[PLANE_COUNT];
    struct v4l2_buffer buf;
    int result = 0;

    for (uint32_t i = 0; i < 10; i++) {
        

        if (wait_for_frame(c->fd) < 0){
            result = 1;
        }
        memset(&buf, 0, sizeof(buf));
        memset(planes, 0, sizeof(planes));

        buf.type = c->type;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.length = PLANE_COUNT;
        buf.m.planes = planes;

        if (v4l2_dequeue_buffer(c->fd, &buf, planes) < 0){
            result = 1;
        }
        if (i == SKIP_FRAMES){
            if (save_frame(c, output, &buf, planes) < 0){
                result = 1;
            }
        } 
        
        if (v4l2_queue_buffer(c->fd, &buf, planes) < 0){
            result = 1;
        }
    }
    return result;
}


void camera_stream_off(camera * const c){
    if (c->stream_started) {
        if (v4l2_stream_off(c->fd, c->type) < 0)
            fprintf(stderr, "Warning: failed to stop stream\n");
    }
}

int camera_off(camera * const c){
    return close(c->fd);
}

void cleanup(camera * const c){
    camera_stream_off(c);
    camera_cleanup_buffers(c);
    camera_off(c);
}

/*

============== CAMERA FORMAT IMPLEMENTATION =================

*/

void format_set_frame_size(camera_format * const c_format, const uint32_t width, const uint32_t height){
    c_format->format.fmt.pix_mp.width = width;
    c_format->format.fmt.pix_mp.height = height;
}

void format_set_pixel_format(camera_format * const c_format, const uint32_t pixel_format){
    c_format->format.fmt.pix_mp.pixelformat = pixel_format;
}

void format_set_field(camera_format * const c_format, const uint32_t field) {
    c_format->format.fmt.pix_mp.field = field;
}

/*

============== CAMERA BUFFER CONFIG IMPLEMENTATION =================

*/

void buffer_config_set_count(camera_buffer_config * const buf_cfg, uint32_t count){
    buf_cfg->buf_config.count = count;
}

void buffer_config_set_memory(camera_buffer_config * const buf_cfg, uint32_t memory){
    buf_cfg->buf_config.memory = memory;
}



