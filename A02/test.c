#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int rank, size, N;

    MPI_Init(&argc, &argv); // Initialize the MPI environment
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get the number of processes

    N = 200000; // Size of the main array. Choose such that N is divisible by size for simplicity.

    // Allocate memory for the main array in the root process
    int *data = NULL;
    if (rank == 0) {
        data = (int *)malloc(N * sizeof(int));
        printf("Original array:\n");
        for (int i = 0; i < N; i++) {
            data[i] = i; // Initialize each element with its index
            printf("%d ", data[i]);
        }
        printf("\n");
    }

    // Calculate the size of the sub-array for each process
    int local_n = N / size;

    // Allocate memory for the sub-array in each process
    int *sub_data = (int *)malloc(local_n * sizeof(int));

    // Distribute parts of the array to all processes
    MPI_Scatter(data, local_n, MPI_INT, sub_data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process performs the addition operation on its portion of the array
    for (int i = 0; i < local_n; i++) {
        sub_data[i] += 1; // Add one to each element
    }

    // Gather the modified sub-arrays back to the root process
    MPI_Gather(sub_data, local_n, MPI_INT, data, local_n, MPI_INT, 0, MPI_COMM_WORLD);

    // The root process prints the final array
    if (rank == 0) {
        printf("Modified array:\n");
        for (int i = 0; i < N; i++) {
            printf("%d ", data[i]);
        }
        printf("\n");
        free(data);
    }

    free(sub_data);
    MPI_Finalize(); // Finalize the MPI environment
    return 0;
}
