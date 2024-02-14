#include <stdio.h>
#include <time.h>

#define N 640000

int main() {
    unsigned long long sum = 0;

    // Initialize the array
    int nums[N];
    for (int i = 0; i < N; i++) {
        nums[i] = i + 1;
    }

    // Finding the sum of the addition of all values in the array
    clock_t start = clock();
    for (int i = 0; i < N; i++) {
        sum += nums[i];
    }
    clock_t end = clock();

    // Calculate time spent summing values
    double time_spent_s = (double)(end - start) / CLOCKS_PER_SEC;
    double time_spent_ms = time_spent_s * 1000;

    printf("The sum is: %lld\n", sum);
    printf("Time taken (milliseconds): %.2f\n", time_spent_ms);
}