#ifndef FB_H
#define FB_H

// #include <fcntl.h>
// #include <linux/fb.h>
#include <stdint.h>
// #include <stdlib.h>
// #include <sys/ioctl.h>
#include <sys/mman.h>
// #include <unistd.h>

#define TRIANGLE(...)               \
    {                               \
        .vertices = { __VA_ARGS__ } \
    }

typedef struct {
    uint32_t xres;
    uint32_t yres;
    uint32_t size;
    uint32_t pixel_size;
    uint32_t *buf;
    int fd;
} fb_t;

typedef struct {
    uint32_t x;
    uint32_t y;
} point2d_t;

typedef union {
    struct { point2d_t va, vb, vc; };
    point2d_t vertices[3];
} triangle_t;

typedef enum {
    FB_ASYNC = MS_ASYNC, // asynchronous (fb_sync returns when write operations are queued)
    FB_SYNC = MS_SYNC    // synchronous (fb_sync returns when buffer is written)
} fb_sync_mode_t;

typedef enum {
    FB_SUCCESS,
    FB_ERR_OUTSIDE_BUF
} fb_status_t;

// this is usually unnecessary, but use it if you want to flush your changes immediately instead of offloading the behavior to the kernel
// error = return -1 and set errno; success = return >= 0
static inline int fb_sync(char *fb_buf, uint32_t size, fb_sync_mode_t sync_mode) {
    return msync(fb_buf, size, sync_mode);
}

// error = return -1 and set errno; success = return >= 0
int fb_init(fb_t *fb, const char *dev);

// error = return -1 and set errno; success = return >= 0
int fb_deinit(fb_t *const fb);

// cast triangle onto screen buffer fb
fb_status_t cast_triangle(const triangle_t *triangle, fb_t *fb, uint32_t col);

#ifdef DEBUG
#include <stdio.h>
#include <string.h>
void print_triangle(const triangle_t *const triangle, const char *const alias); 
#else
#define print_triangle(...) ((void)0)
#endif

#endif
