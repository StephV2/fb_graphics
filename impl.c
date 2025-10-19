#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "fb.h"

#define RGBA_MAX 0xFFFFFFFF

uint32_t rand_int(uint32_t max) {
    return rand() % max;
}

int main() {
    fb_t fb;
    fb_init(&fb, "/dev/fb0");

    const triangle_t tri = TRIANGLE(
        { 0, 1079 },
        { 1919, 1079 },
        { 960, 0 }
    );

    cast_triangle(&tri, &fb, RGBA_MAX);

    srand(time(NULL));

    // TODO: fix segfault issue, potential cause: x or y coordinate is 0?
    // could be related to the issue where 2 180 degree rotated triangles are rendered instead of 1
    // so (x, y) turns into (-x, -y) for some reason
    while (fgetc(stdin)) {
        // const triangle_t tri_n = TRIANGLE({ rand_int(1920), rand_int(1080) }, { rand_int(1920), rand_int(1080) }, { 0, 0 });
        const triangle_t tri_n = TRIANGLE({ rand_int(1920), rand_int(1080) }, { rand_int(1920), rand_int(1080) }, { rand_int(1920), rand_int(1080) });
        // const triangle_t tri_n = TRIANGLE({ 0, 100 }, { 604, 0 }, { 0, 0 });
        cast_triangle(&tri_n, &fb, rand_int(0xFFFFFFFF));
    }

    fb_deinit(&fb);

    return errno;
}
