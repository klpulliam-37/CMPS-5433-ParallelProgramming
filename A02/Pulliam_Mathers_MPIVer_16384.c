//**************************************************************
// Assignment #2
// Name: Kolten Pulliam and Garrett Mathers
// Parallel Programming Date: 03/29/2024
//***************************************************************
// This program demonstrates an implementation of the Cooley-Tukey
// FFT algorithm using MPI to split the workload of calculating the 
// DFT given a sample set of 16384 values.
//
// To compile the program:
// mpicc Pulliam_Mathers_MPIVer_16384.c -o bigparallel -lm
//
// To run the program without the script file (locally):
// mpiexec -n <number-of-processes> ./bigparallel
// 
// To run the program with the script file on frontera:
// sbatch ParallelFrontera16big
// or
// sbatch ParallelFrontera32big
//*****************************************************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#define PI 3.14159265358979323846
#define N 16384

// Complex number struct
struct Complex {
    double real;
    double imag;
};

//*******************************************************************
// fft
// Parameters: 
// - struct Complex* samples -> Array of complex numbers
// - int n -> the number of elements in the array
// This function performs the Cooley-Tukey algorithm on an array of
// complex numbers, and returns nothing as the samples array is 
// passed by pointer.
//********************************************************************
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
        struct Complex t = {cos(angle), sin(angle)};

        // Apply the butterfly operation
        struct Complex tmp = {t.real * odd[k].real - t.imag * odd[k].imag,
                       t.real * odd[k].imag + t.imag * odd[k].real};

        samples[k].real = even[k].real + tmp.real;
        samples[k].imag = even[k].imag + tmp.imag;
        samples[k + n / 2].real = even[k].real - tmp.real;
        samples[k + n / 2].imag = even[k].imag - tmp.imag;
    }
}

//*******************************************************************
// createSamples
// Parameters: 
// - int n -> the number of elements to be created
// This function creates an array of structs that represent the 
// complex numbers. It initializes the first eight values according 
// to the table provided in the instructions, and all other values 
// are initialized to zero. This function returns a pointer to the
// struct Complex array.
//********************************************************************
struct Complex* createSamples(int n) {
    // Allocate memory for the array of complex numbers
    struct Complex *samples = (struct Complex *)calloc(n, sizeof(struct Complex));

    // Initialize the first 8 values
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

int main() {
    double sum = 0.f;
    int shouldPrint = 1;

    int rank, size;
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Create a datatype for the Complex struct to pass through MPI_Scatter/Gather
    MPI_Datatype ComplexType;
    MPI_Type_contiguous(2, MPI_DOUBLE, &ComplexType);
    MPI_Type_commit(&ComplexType);

    // Calculate the number of elements to send to each process
    int local_n = N / size;

    // Calculate average time of execution across three runs
    for (int itr = 0; itr < 3; itr++) {
        struct Complex *samples;
        if (rank == 0) {
            samples = createSamples(N); // Root process creates the full sample array
        }

        // Create an array of complex number structs based on local_n size
        struct Complex *local_samples = (struct Complex *)malloc(local_n * sizeof(struct Complex));

        // Start the timer
        double start_time = MPI_Wtime();

        // Distribute parts of the array to all processes
        MPI_Scatter(samples, local_n, ComplexType, local_samples, local_n, ComplexType, 0, MPI_COMM_WORLD);

        // Each process performs FFT on its portion of the data
        fft(local_samples, local_n);

        // Gather the sub arrays back to process zero
        MPI_Gather(local_samples, local_n, ComplexType, samples, local_n, ComplexType, 0, MPI_COMM_WORLD);
        double end_time = MPI_Wtime();

        // Calculate time spent
        double total_time = end_time - start_time;
        double total_time_ms = (total_time * 1000);
        sum += total_time_ms;

        if (shouldPrint) {
            // Print the FFT result if process rank is 0
            if (rank == 0) {
                printf("TOTAL PROCESSED SAMPLES: %d\n", N);
                printf("================================\n");
                for (int i = 0; i < 11; i++) {
                    printf("XR[%d]: %.6f    XI[%d]: %.6f\n", i, samples[i].real, i, samples[i].imag);
                    printf("================================\n");
                }

                // Free memory allocated by process 0
                free(samples);
            }

            shouldPrint = 0;
        }

        // Free memory for local_sample allocated by each process
        free(local_samples);
    }

    if (rank == 0) {
        double average_time = sum / 3;
        printf("Average time (milliseconds): %.6f\n", average_time);
    }

    MPI_Type_free(&ComplexType);
    MPI_Finalize();
    return 0;
}
