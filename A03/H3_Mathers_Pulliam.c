// Garrett Mathers and Kolten Pulliam
// Homework 3 - In class assignment
// Compilation:
// mpicc H3_Mathers_Pulliam.c -o main.exe
//
// Execution:
// mpiexec -n <number of processes> ./main.exe

#include <stdio.h>
#include <mpi.h>

int main(){
    int my_rank, comm_sz; 

    int start_num = 191, added_num = 111, sum = 0; 

    /* Let the system do what it needs to start up MPI */
    MPI_Init(NULL, NULL);

    /* Get my process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    if (my_rank == 0){
        MPI_Send(&start_num, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        
        MPI_Recv(&sum, 1, MPI_INT, comm_sz-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("The final sum is: %d\n", sum);
    } else {
        MPI_Recv(&sum, 1, MPI_INT, my_rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        sum += added_num * my_rank;
        if (my_rank == comm_sz - 1) {
            MPI_Send(&sum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        } else {
            MPI_Send(&sum, 1, MPI_INT, my_rank+1, 0, MPI_COMM_WORLD);
        }
        
    }

    MPI_Finalize();
}