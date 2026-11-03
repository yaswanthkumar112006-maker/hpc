#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int rank, size;

    /* Initialize MPI */
    MPI_Init(&argc, &argv);

    /* Get current process rank and total number of processes */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    printf("Hello World from process %d of %d\n", rank, size);

    /* Finalize MPI */
    MPI_Finalize();
    return 0;
}
