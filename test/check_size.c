// I needed fast check for structure/values size determining
#include "../device/camera.h"
//#include "../json/lib/cJSON.h"
#include <stddef.h>
#include <stdio.h>


int main() {
    printf("=== CAMERA STRUCTURE MEMORY MAP ===\n\n");
    printf("Total Structure Size: %zu bytes\n\n", sizeof(camera));

    printf("%-20s %-12s %-12s\n", "Member Name", "Byte Offset", "Size (Bytes)");
    printf("------------------------------------------------------\n");

    // Print offsets and individual sizes of each element
    printf("%-20s %-12zu %-12zu\n", "buffers",        offsetof(camera, buffers),        sizeof(((camera*)0)->buffers));
    printf("%-20s %-12zu %-12zu\n", "format",         offsetof(camera, format),         sizeof(((camera*)0)->format));
    printf("%-20s %-12zu %-12zu\n", "cap",            offsetof(camera, cap),            sizeof(((camera*)0)->cap));
    printf("%-20s %-12zu %-12zu\n", "cfg",            offsetof(camera, cfg),            sizeof(((camera*)0)->cfg));
    printf("%-20s %-12zu %-12zu\n", "fd",             offsetof(camera, fd),             sizeof(((camera*)0)->fd));
    printf("%-20s %-12zu %-12zu\n", "stream_started", offsetof(camera, stream_started), sizeof(((camera*)0)->stream_started));
    printf("%-20s %-12zu %-12zu\n", "buffer_count",   offsetof(camera, buffer_count),   sizeof(((camera*)0)->buffer_count));
    printf("%-20s %-12zu %-12zu\n", "plane_count",    offsetof(camera, plane_count),    sizeof(((camera*)0)->plane_count));
    printf("%-20s %-12zu %-12zu\n", "OPTIONS_MASK",   offsetof(camera, OPTIONS_MASK),   sizeof(((camera*)0)->OPTIONS_MASK));
    printf("%-20s %-12zu %-12zu\n", "type",           offsetof(camera, type),           sizeof(((camera*)0)->type));
    

    printf("==================================================\n");
    printf("     SUB-STRUCTURE SIZE & ALIGNMENT ANALYSIS      \n");
    printf("==================================================\n\n");

    // --- 1. ANALYSIS OF struct buffer ---
    printf("1. struct buffer\n");
    printf("   - Total Size:         %zu bytes\n", sizeof(struct buffer));
    printf("   - Offset of 'addr':   %zu bytes\n", offsetof(struct buffer, addr));
    printf("   - Offset of 'length': %zu bytes\n", offsetof(struct buffer, length));
    printf("   - Size of elements:   (void*) = %zu bytes, (size_t) = %zu bytes\n\n", 
           sizeof(void*), sizeof(size_t));

    // --- 2. ANALYSIS OF camera_format ---
    printf("2. camera_format (Wraps struct v4l2_format)\n");
    printf("   - Total Wrapper Size: %zu bytes\n", sizeof(camera_format));
    printf("   - Underlying V4L2 Size: %zu bytes\n\n", sizeof(struct v4l2_format));

    // --- 3. ANALYSIS OF camera_buffer_config ---
    printf("3. camera_buffer_config (Wraps struct v4l2_requestbuffers)\n");
    printf("   - Total Wrapper Size: %zu bytes\n", sizeof(camera_buffer_config));
    printf("   - Underlying V4L2 Size: %zu bytes\n", sizeof(struct v4l2_requestbuffers));
    printf("==================================================\n");

    return 0;
}