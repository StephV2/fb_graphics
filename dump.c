#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "fb.h"

#define SQRT3_R 0.5773502691896258
#define RGB_MAX 255

#define MK_GRADIENT(axis, axis_res) \
    ((char)(((double)(axis) / (axis_res)) * RGB_MAX))

#ifdef DEBUG
#define D_PRINTF printf
#else
#define D_PRINTF(...) ((void)0)
#endif

static inline uint32_t min(uint32_t x, uint32_t y) {
    return x < y ? x : y;
}

int main() {
    fb_t fb;
    fb_init(&fb, "/dev/fb0");

    char *fb_dup = malloc(fb.size);
    memcpy(fb_dup, fb.buf, fb.size);

    const uint32_t midpoint = fb.xres / 2;
    const uint32_t sqlen = min(fb.xres, fb.yres);

    for (uint32_t y = 0; y < fb.yres; y++) {
        const uint32_t equi_offset = y * SQRT3_R;
        const uint32_t min_x = midpoint - equi_offset;
        const uint32_t max_x = midpoint + equi_offset;

        for (uint32_t x = 0; x < fb.xres; x++) {
            const uint32_t location = (fb.xres * y + x) * fb.pixel_size;

            if (min_x < max_x && x > min_x && x < max_x) {
                char pixel[4];

                // interesting effect if enabled alone
                // pixel[0] = (char)((sqrt((double)y) / RGB_MAX) * x);
                // pixel[1] = (char)(((double)x / RGB_MAX) * y * y);
                // pixel[2] = (char)(((double)y / RGB_MAX) * min_x * max_x);

                // pixel[0] = MK_GRADIENT(fb.yres - y, fb.yres);     // blue
                // pixel[1] = MK_GRADIENT(x, fb.yres * 2);           // green
                // pixel[2] = MK_GRADIENT(xres - x, fb.yres * 2);    // red

                pixel[0] = ~MK_GRADIENT(y, sqlen);              // blue
                pixel[1] = MK_GRADIENT(x, sqlen * 2);           // green
                pixel[2] = ~MK_GRADIENT(x, sqlen * 2);          // red
                pixel[3] = '\0';                                // alpha

                memcpy(fb.buf + location, pixel, fb.pixel_size);
            } else {
                memcpy(fb.buf + location, fb_dup + location, fb.pixel_size);
            }
        }
    }

    fgetc(stdin);

    fb_deinit(&fb);
    free(fb_dup);
    fb_dup = NULL;

    return errno;
}
