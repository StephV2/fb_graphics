#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
// #include <stdlib.h>
// #include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "fb.h"

// this is just bad, remove it
#ifdef DEBUG
#include <stdio.h>
// debug function for figuring out the values of a triangle (use alias to print a human readable identifier for your triangle)
int print_triangle(const triangle_t *const triangle, const char *const alias) {
    const char msg[] = "Information for triangle %p (%s):\n";
    char border[100];
    memset(border, '=', 98);
    border[98] = '\n';
    fputs(border, stdout);
    printf(msg, triangle, alias);
    for (int i = 0; i < 3; i++) {
        point2d_t cur_vertex = triangle->vertices[i];
        printf("v%c(x, y): (%d, %d)\n", 'a' + i, cur_vertex.x, cur_vertex.y);
    }
    fputs(border, stdout);
}
#else
static inline int noop(void *_, ...) { return 0; }
#define printf noop
#define puts noop
#endif

// a ^= b;
// b ^= a;
// a ^= b;
#define SWAP_VARS(a, b) do {     \
    __typeof__(a) _TEMP_VAR = a; \
    a = b;                       \
    b = _TEMP_VAR;               \
} while (0)

typedef struct fb_var_screeninfo fb_var_screeninfo_t;

// error = return -1 and set errno; success = return >= 0
int fb_init(fb_t *fb, const char *dev) {
    fb_var_screeninfo_t fb_screeninfo;

    fb->fd = open(dev, O_RDWR | O_NONBLOCK);
  
    if (fb->fd < 0 || ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb_screeninfo) < 0)
        return -1;

    fb->xres = fb_screeninfo.xres;
    fb->yres = fb_screeninfo.yres;
    fb->pixel_size = fb_screeninfo.bits_per_pixel / 8;
    fb->size = fb->xres * fb->yres * fb->pixel_size;

    void *const mmap_ret = mmap(NULL, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);

    if (mmap_ret == MAP_FAILED) {
        fb->buf = NULL;
        return -1;
    }

    fb->buf = mmap_ret;

    return 0;
}

// error = return -1 and set errno; success = return >= 0
int fb_deinit(fb_t *fb) {
    int ret_val = 0;

    if (close(fb->fd) < 0)
        ret_val = -1;
    if (munmap(fb->buf, fb->size) < 0)
        ret_val = -1;
    else
        fb->buf = NULL;

    return ret_val;
}

// return a clone of a triangle whose vertices' y values are sorted from least to greatest
// if necessary, rework this function to be more generic in the future
static inline triangle_t sort_triangle_vertices_y(const triangle_t *triangle) {
    triangle_t sorted = *triangle;

    if (sorted.va.y > sorted.vc.y)
        SWAP_VARS(sorted.va, sorted.vc);

    if (sorted.va.y > sorted.vb.y)
        SWAP_VARS(sorted.va, sorted.vb);

    if (sorted.vb.y > sorted.vc.y)
        SWAP_VARS(sorted.vb, sorted.vc);

    return sorted;
}

static inline void fill_pixels(fb_t *fb, uint32_t y, uint32_t bound1, uint32_t bound2, uint32_t col) {
    uint32_t pixels;

    if (bound1 > bound2)
        pixels = bound1 - bound2;
    else
        pixels = bound2 - bound1;

    // printf("%u - %u\n", bound1, bound2);

    uint32_t *fb_buf_offset = fb->buf + (fb->xres * y + bound1); //* fb->pixel_size;
    const uint32_t *const fb_buf_offset_end = fb_buf_offset + pixels; //* fb->pixel_size;

    do {
        *fb_buf_offset = col;
    } while (++fb_buf_offset < fb_buf_offset_end);

    // memset(fb->buf + (fb->xres * y + bound1) * fb->pixel_size, col, pixels * fb->pixel_size);
}

static inline double slope_r(const point2d_t *va, const point2d_t *vb) {
    const double diff_y = (double)vb->y - va->y;

    printf("(vb.x - va.x) / (vb.y - va.y); delta_x / delta_y\n(%d - %d) / (%d - %d); %d / %d\n", (double)vb->x, (double)va->x, (double)vb->y, (double)va->y, (double)((double)vb->x - va->x), (double)diff_y);
    return diff_y == 0.0f ? 0.0f : ((double)vb->x - va->x) / diff_y;
}

// cast triangle onto screen buffer fb
// TODO: maybe add an option to wrap the triangle vertices (aka modulo) if it's outside the bounds of the screen?
fb_status_t cast_triangle(const triangle_t *tri, fb_t *fb, uint32_t col) {
    for (int i = 0; i < 3; i++) {
        const point2d_t vertex = tri->vertices[i];

        if (vertex.x >= fb->xres || vertex.y >= fb->yres)
            return FB_ERR_OUTSIDE_BUF;
    }

    const triangle_t sorted_tri = sort_triangle_vertices_y(tri);

    const double mr_ab = slope_r(&sorted_tri.va, &sorted_tri.vb);
    const double mr_ac = slope_r(&sorted_tri.va, &sorted_tri.vc);
    const double mr_bc = slope_r(&sorted_tri.vb, &sorted_tri.vc);

    printf("mr_ab: %f; mr_ac: %f; mr_bc: %f\n", mr_ab, mr_ac, mr_bc);

    print_triangle(tri, "original");
    print_triangle(&sorted_tri, "sorted");

    for (uint32_t y = sorted_tri.va.y; y < sorted_tri.vb.y; y++) {
        const uint32_t bound1 = (y - sorted_tri.va.y) * mr_ab + sorted_tri.va.x;
        const uint32_t bound2 = (y - sorted_tri.va.y) * mr_ac + sorted_tri.va.x;

        // printf("1: y=%d; ", y);
        fill_pixels(fb, y, bound1, bound2, col);
    }

    puts("DONE");

    for (uint32_t y = sorted_tri.vb.y; y <= sorted_tri.vc.y; y++) {
        const uint32_t bound1 = (sorted_tri.vc.y - y) * mr_ac - sorted_tri.vc.x;
        const uint32_t bound2 = (sorted_tri.vc.y - y) * mr_bc - sorted_tri.vc.x;

        // printf("2: y=%d; ", y);
        fill_pixels(fb, y, bound1, bound2, col);
    }

    puts("DONE2");

    return FB_SUCCESS;
}
