#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <pthread.h>

#define THREAD_NUM 4

typedef struct {
    int start_row;
    int end_row;
    png_bytep *rows;
    png_bytep *new_rows;
    int width;
    int height;
} ThreadData;

png_bytep *read_png_file(char *filename, png_structp *png_ptr, png_infop *info_ptr, int *width, int *height, png_byte *color_type, png_byte *bit_depth) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) abort();

    *png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!*png_ptr) abort();

    *info_ptr = png_create_info_struct(*png_ptr);
    if (!*info_ptr) abort();

    if (setjmp(png_jmpbuf(*png_ptr))) abort();

    png_init_io(*png_ptr, fp);
    png_read_info(*png_ptr, *info_ptr);

    *width = png_get_image_width(*png_ptr, *info_ptr);
    *height = png_get_image_height(*png_ptr, *info_ptr);
    *color_type = png_get_color_type(*png_ptr, *info_ptr);
    *bit_depth = png_get_bit_depth(*png_ptr, *info_ptr);

    if (*bit_depth == 16)
        png_set_strip_16(*png_ptr);

    if (*color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(*png_ptr);

    // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
    if (*color_type == PNG_COLOR_TYPE_GRAY && *bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(*png_ptr);

    if (png_get_valid(*png_ptr, *info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(*png_ptr);

    png_read_update_info(*png_ptr, *info_ptr);

    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * (*height));
    for (int y = 0; y < *height; y++) {
        row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(*png_ptr, *info_ptr));
    }

    png_read_image(*png_ptr, row_pointers);

    fclose(fp);
    return row_pointers;
}

void write_png_file(char *filename, png_bytep *rows, int width, int height, png_byte color_type, png_byte bit_depth) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) abort();

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) abort();

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) abort();

    if (setjmp(png_jmpbuf(png_ptr))) abort();

    png_init_io(png_ptr, fp);

    // Output is 8bit depth, RGB format.
    png_set_IHDR(
        png_ptr,
        info_ptr,
        width, height,
        bit_depth,
        color_type,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE
    );

    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, rows);
    png_write_end(png_ptr, NULL);

    for (int y = 0; y < height; y++) {
        free(rows[y]);
    }
    free(rows);

    fclose(fp);
}

void *apply_convolution(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    
    // int kernel[3][3] = {
    //     {1/3, 1/3, 1/3},
    //     {1/3, 1/3, 1/3},
    //     {1/3, 1/3, 1/3}
    // };

    // int sumKernel = 9; // Sum of the kernel for normalization

    float kernel[5][5] = {
        {1/25, 1/25, 1/25, 1/25, 1/25},
        {1/25, 1/25, 1/25, 1/25, 4/25},
        {1/25, 1/25, 1/25, 1/25, 1/25},
        {1/25, 1/25, 1/25, 1/25, 1/25},
        {1/25, 1/25, 1/25, 1/25, 1/25}
    };
    int sumKernel = 625; // Sum of all weights in the kernel

    int kCenter = 5 / 2;

    for (int y = data->start_row; y < data->end_row; y++) {
    for (int x = 0; x < data->width; x++) {
        float sumR = 0, sumG = 0, sumB = 0;
        for (int ky = -kCenter; ky <= kCenter; ky++) {
            for (int kx = -kCenter; kx <= kCenter; kx++) {
                int posY = y + ky;
                int posX = x + kx;
                if (posX >= 0 && posX < data->width && posY >= 0 && posY < data->height) {
                    png_byte* pixel = &(data->rows[posY][posX * 3]);
                    float weight = kernel[ky + kCenter][kx + kCenter];
                    sumR += pixel[0] * weight;
                    sumG += pixel[1] * weight;
                    sumB += pixel[2] * weight;
                }
            }
        }
        png_byte* newPixel = &(data->new_rows[y][x * 3]);
        newPixel[0] = (png_byte)(sumR); // Normalize if necessary
        newPixel[1] = (png_byte)(sumG);
        newPixel[2] = (png_byte)(sumB);
    }
}

    pthread_exit(NULL);

}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *input_file = argv[1];
    char *output_file = argv[2];
    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    png_bytep *row_pointers = read_png_file(input_file, &png_ptr, &info_ptr, &width, &height, &color_type, &bit_depth);

    png_bytep *new_row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
    for (int y = 0; y < height; y++) {
        new_row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));
    }

    pthread_t threads[THREAD_NUM];
    ThreadData thread_data[THREAD_NUM];
    for (int i = 0; i < THREAD_NUM; i++) {
        thread_data[i].start_row = i * height / THREAD_NUM;
        thread_data[i].end_row = (i + 1) * height / THREAD_NUM;
        thread_data[i].rows = row_pointers;
        thread_data[i].new_rows = new_row_pointers;
        thread_data[i].width = width;
        thread_data[i].height = height;
        pthread_create(&threads[i], NULL, apply_convolution, (void *)&thread_data[i]);
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

    write_png_file(output_file, new_row_pointers, width, height, color_type, bit_depth);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    for (int y = 0; y < height; y++) {
        free(row_pointers[y]);
    }
    free(row_pointers);

    return 0;
}
