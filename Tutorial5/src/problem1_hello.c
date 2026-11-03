#include <mpi.h>
#include <stdio.h>

/*
 * HPC-2026 - MPI Programming Tutorial
 * Problem 1: Hello World with MPI
 * 
 * Objective:
 * Learn the basic structure of an MPI program using:
 * - MPI_Init
 * - MPI_Comm_rank
 * - MPI_Comm_size
 * - MPI_Finalize
 */

int main(int argc, char **argv) {
    int rank, size;

    /* Initialize the MPI execution environment */
    MPI_Init(&argc, &argv);

    /* Get the unique rank (ID) of this process in MPI_COMM_WORLD */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* Get the total number of processes launched in MPI_COMM_WORLD */
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Print rank and total communicator size */
    printf("Hello World from process %d of %d\n", rank, size);

    /* Clean up all MPI state and finalize */
    MPI_Finalize();

    return 0;
}
