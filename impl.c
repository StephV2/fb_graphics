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

    while (fgetc(stdin)) {
        const triangle_t tri_n = TRIANGLE(rand_int(1920), rand_int(1080), rand_int(1920), rand_int(1080));
        printf("%d\n", rand_int(0xFFFFFFFF));
        cast_triangle(&tri_n, &fb, rand_int(0xFFFFFFFF));
    }

    fb_deinit(&fb);

    #ifdef DEBUG
    printf("%s\n", "debug works");
    #endif

    return errno;
}
