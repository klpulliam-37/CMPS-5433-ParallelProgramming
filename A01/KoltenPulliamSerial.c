// Garrett Mathers & Kolten Pulliam
// mpicc KoltenPulliamSerial.c -o mycodeseq

#include <stdio.h>
#include <time.h>

#define N 640000

int main() {
    unsigned long long sum = 0;
    int nums[N];
    for (int i = 0; i < N; i++) {
        nums[i] = i + 1;
    }

    // For measuring wall clock time with high resolution
    struct timespec start, end;

    // Start timer
    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < N; i++) {
        sum += nums[i];
    }

    // Stop timer
    clock_gettime(CLOCK_REALTIME, &end);

    // Calculate time spent summing values. Convert to milliseconds.
    double time_spent_ms = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    printf("The sum is: %lld\n", sum);
    printf("Time taken (milliseconds): %.2f\n", time_spent_ms);
}
