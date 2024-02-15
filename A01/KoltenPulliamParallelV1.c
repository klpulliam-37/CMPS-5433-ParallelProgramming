#include <stdio.h>
#include <mpi.h>

#define N 640000

int sum(int array[]);

int main(void) {
    int my_rank, comm_sz, local_n;
    int local_start, local_end;
    unsigned long long local_sum;
    int source;
    double start_time, end_time, total_time;

    /* Let the system do what it needs to start up MPI */
    MPI_Init(NULL, NULL);

    /* Get my process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    local_n = N / comm_sz;  // The number of elements to add for each process

    if (my_rank != 0) { 
        int sub_samples[local_n];
        MPI_Recv(sub_samples, local_n, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 

        // Calculate given portion
        for (int i = 0; i < local_n; i++) {
            local_sum += sub_samples[i];
        }

        // Send back calculation
        MPI_Send(&local_sum, 1, MPI_UNSIGNED_LONG_LONG, 0, 0, MPI_COMM_WORLD);

    } else {
        // Delcare summation array
        unsigned long long summations[comm_sz];

        // Initialize the samples array
        int samples[N];
        for (int i = 0; i < N; i++) {
            samples[i] = i + 1;
        }

        // Send out portions of array
        start_time = MPI_Wtime();
        for (source = 1; source < comm_sz; source++) {
            local_start = source * local_n;
            local_end = local_start + local_n;

            // Copy over array portion
            int sub_samples[local_n];
            for (int i = 0; i < local_n; i++, local_start++) {
                sub_samples[i] = samples[local_start];
            }

            // Send the copied portion
            MPI_Send(sub_samples, local_n, MPI_INT, source, 0, MPI_COMM_WORLD); 
        }
        
        // Calculate first portion
        for (int i = 0; i < local_n; i++) {
            local_sum += samples[i];
        }

        // Receive each local sum and assign to respective slot in summations array
        summations[0] = local_sum;
        for (source = 1; source < comm_sz; source++) {
            MPI_Recv(&local_sum, 1, MPI_UNSIGNED_LONG_LONG, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            // printf("local_sum for P%d: %lld\n", source, local_sum);
            summations[source] = local_sum;
        }

        // Calculate total based on partial sums
        unsigned long long total_sum = 0;
        for (int i = 0; i < comm_sz; i++) {
            // printf("summations[%d] = %lld\n", i, summations[i]);
            total_sum += summations[i];
        }
        end_time = MPI_Wtime();

        total_time = end_time - start_time;

        printf("In total, it took %.2f ms to calculate the sum.\n", (total_time * 1000));

        printf("With n = %d elements, the sum\n", N);
        printf("from %d to %d = %lld\n", 1, N, total_sum);
    } 

    /* Shut down MPI */
    MPI_Finalize();

    return 0;
}