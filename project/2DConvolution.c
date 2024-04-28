#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define KERNEL_SIZE 3
#define IMAGE_SIZE 5

int kernel[KERNEL_SIZE][KERNEL_SIZE] = {
    {0, 1, 0},
    {1, -4, 1},
    {0, 1, 0}
};

int image[IMAGE_SIZE][IMAGE_SIZE] = {
    {1, 2, 3, 4, 5},
    {5, 6, 7, 8, 9},
    {9, 10, 11, 12, 13},
    {13, 14, 15, 16, 17},
    {17, 18, 19, 20, 21}
};

int output[IMAGE_SIZE][IMAGE_SIZE];

void *convolution(void *threadid) {
    long tid;
    tid = (long)threadid;
    int start_row = (IMAGE_SIZE / NUM_THREADS) * tid;
    int end_row = (tid == NUM_THREADS - 1) ? IMAGE_SIZE : start_row + IMAGE_SIZE / NUM_THREADS;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < IMAGE_SIZE; j++) {
            int sum = 0;
            for (int m = -KERNEL_SIZE/2; m <= KERNEL_SIZE/2; m++) {
                for (int n = -KERNEL_SIZE/2; n <= KERNEL_SIZE/2; n++) {
                    int x = i + m;
                    int y = j + n;
                    if (x >= 0 && x < IMAGE_SIZE && y >= 0 && y < IMAGE_SIZE) {
                        sum += image[x][y] * kernel[m + KERNEL_SIZE/2][n + KERNEL_SIZE/2];
                    }
                }
            }
            output[i][j] = sum;
        }
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;
    for(t = 0; t < NUM_THREADS; t++){
        rc = pthread_create(&threads[t], NULL, convolution, (void *)t);
        if (rc){
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    for(t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // Print the output image
    printf("Output Image:\n");
    for (int i = 0; i < IMAGE_SIZE; i++) {
        for (int j = 0; j < IMAGE_SIZE; j++) {
            printf("%d ", output[i][j]);
        }
        printf("\n");
    }

    pthread_exit(NULL);
}
