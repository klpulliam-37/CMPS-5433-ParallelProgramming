//**************************************************************
// Assignment #2
// Name: Kolten Pulliam and Garrett Mathers
// Parallel Programming Date: 03/28/2024
//***************************************************************
// Place your general program documentation here. It should
// be quite a few lines explaining the programs duty carefully.
// It should also indicate how to run the program and data
// input format, filenames etc
//*****************************************************************
//*******************************************************************
// FOR ALL FUNCTIONS 
// function Name::MethodName()
// Parameters: List them here and comment
// A discussion of what the method/function does and required
// parameters as well as return value.
//********************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PI 3.14159265358979323846
#define N 16384 // Assuming N is a power of 2 for simplicity

// Complex number structure
struct Complex {
    double real;
    double imag;
};

// Function to compute the FFT
void fft(struct Complex* samples, int n) {
    if (n <= 1) return;

    // Divide step: Split the samples into even and odd elements
    struct Complex even[n/2], odd[n/2];
    for (int i = 0; i < n / 2; i++) {
        even[i] = samples[2 * i];
        odd[i] = samples[2 * i + 1];
    }

    // Recursive conquer step
    fft(even, n / 2);
    fft(odd, n / 2);

    // Combine step
    for (int k = 0; k < n / 2; k++) {
        // Compute the twiddle factor
        double angle = -2 * PI * k / n;
        struct Complex t = {cos(angle), sin(angle)}; // Twiddle factor

        // Apply the butterfly operation
        struct Complex tmp = {t.real * odd[k].real - t.imag * odd[k].imag,
                       t.real * odd[k].imag + t.imag * odd[k].real};

        samples[k].real = even[k].real + tmp.real;
        samples[k].imag = even[k].imag + tmp.imag;
        samples[k + n / 2].real = even[k].real - tmp.real;
        samples[k + n / 2].imag = even[k].imag - tmp.imag;
    }
}

struct Complex* createSamples(int n) {
    // Allocate memory for the array of complex numbers
    struct Complex *samples = (struct Complex *)calloc(n, sizeof(struct Complex));

    // Initialize the first 8 values
    // Example values, adjust as needed
    samples[0] = (struct Complex){3.6, 2.6};
    samples[1] = (struct Complex){2.9, 6.3};
    samples[2] = (struct Complex){5.6, 4};
    samples[3] = (struct Complex){4.8, 9.1};
    samples[4] = (struct Complex){3.3, 0.4};
    samples[5] = (struct Complex){5.9, 4.8};
    samples[6] = (struct Complex){5, 2.6};
    samples[7] = (struct Complex){4.3, 4.1};

    return samples;
}

// Main function to demonstrate the FFT function
int main() {
    double sum = 0.f;
    int shouldPrint = 1;
    // Calculate average time of execution
    for (int itr = 0; itr < 3; itr++) {
        // Create sample array
        struct Complex* samples = createSamples(N);

        clock_t start = clock();
        // Perform FFT
        fft(samples, N);
        clock_t end = clock();

        // Calculate time spent summing values
        double time_spent_s = (double)(end - start) / CLOCKS_PER_SEC;
        double time_spent_ms = time_spent_s * 1000;
        sum += time_spent_ms;

        if (shouldPrint) {
            // Print the FFT result
            printf("TOTAL PROCESSED SAMPLES: %d\n", N);
            printf("================================\n");
            for (int i = 0; i < 11; i++) {
                printf("XR[%d]: %.6f    XI[%d]: %.6f\n", i, samples[i].real, i, samples[i].imag);
                printf("================================\n");
            }

            shouldPrint = 0;
        }

        printf("Time spent: %.6f\n", time_spent_ms);

        free(samples);
    }
    double average_time = sum / 3;
    printf("Average time (milliseconds): %.6f\n", average_time);
    
    return 0;
}
