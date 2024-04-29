#include <stdlib.h>
#include <stdio.h>
#include "bitmap_image.hpp"
#include <sys/time.h>

bitmap_image image("images/MARBLES.bmp");
bitmap_image out;
rgb_t** inImage;

void loadPixelsToArray() {
    inImage = (rgb_t**) malloc(image.height() * sizeof(rgb_t*));
    for (int i = 0; i < image.height(); i++) {
        inImage[i] = (rgb_t*) malloc(image.width() * sizeof(rgb_t));
    }

    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            image.get_pixel(x, y, inImage[y][x]);
        }
    }
}

void processImage() {
    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            int red = 0, green = 0, blue = 0, neighbors = 0;
            rgb_t color;

            for (int i = x-1; i <= x+1; i++) {
                for (int j = y-1; j <= y+1; j++) {
                    if (i >= 0 && j >= 0 && i < image.width() && j < image.height()) {
                        neighbors++;
                        color = inImage[j][i];

                        red += color.red;
                        green += color.green;
                        blue += color.blue;
                    }
                }
            }
            red /= neighbors;
            green /= neighbors;
            blue /= neighbors;
            color = make_colour(red, green, blue);
            out.set_pixel(x, y, color);
        }
    }
}

int main(int argc, const char * argv[]) {
    if (!image) {
        printf("Error - Failed to open image\n");
        return 1;
    }
    out = bitmap_image(image.width(), image.height());
    loadPixelsToArray();

    struct timeval start, end;
    gettimeofday(&start, NULL);

    processImage();

    gettimeofday(&end, NULL);
    float delta = ((end.tv_sec - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

    printf("Duration: %f seconds\n", delta);

    out.save_image("images/out.bmp");

    return 0;
}
