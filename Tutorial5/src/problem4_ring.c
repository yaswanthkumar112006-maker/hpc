#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int next = (rank + 1) % size;
    int prev = (rank - 1 + size) % size;

    printf("Process %d: previous = %d, next = %d\n", rank, prev, next);

    int send_val = rank * 10;
    int recv_val = -1;

    // Use MPI_Sendrecv to avoid deadlock
    MPI_Sendrecv(&send_val, 1, MPI_INT, next, 0,
                 &recv_val, 1, MPI_INT, prev, 0,
                 MPI_COMM_WORLD, &status);

    printf("Process %d sent %d to %d and received %d from %d\n", rank, send_val, next, recv_val, prev);

    MPI_Finalize();
    return 0;
}
