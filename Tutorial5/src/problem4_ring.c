#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int rank, size;
    int next, previous;
    int send_val, recv_val;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* Compute logical ring neighbors */
    next = (rank + 1) % size;
    previous = (rank - 1 + size) % size;

    if (rank == 0) {
        printf("Problem 4: Ring Communication (Size = %d)\n\n", size);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    int token = 0;
    if (rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, rank - 1, 990, MPI_COMM_WORLD, &status);
    }
    printf("[Process %d] Neighbors: previous = %d, next = %d\n", rank, previous, next);
    fflush(stdout);
    if (rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, rank + 1, 990, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* Method 1: Deadlock-free communication using MPI_Sendrecv */
    if (rank == 0) printf("\n--- Method 1: MPI_Sendrecv ---\n");
    MPI_Barrier(MPI_COMM_WORLD);

    send_val = (rank * 10) + 100;
    recv_val = -1;

    MPI_Sendrecv(&send_val, 1, MPI_INT, next, 10,
                 &recv_val, 1, MPI_INT, previous, 10,
                 MPI_COMM_WORLD, &status);

    if (rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, rank - 1, 991, MPI_COMM_WORLD, &status);
    }
    printf("[Process %d] Sent %d to Rank %d | Received %d from Rank %d\n",
           rank, send_val, next, recv_val, previous);
    fflush(stdout);
    if (rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, rank + 1, 991, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* Method 2: Even-Odd ordering to avoid circular deadlock */
    if (rank == 0) printf("\n--- Method 2: Even-Odd Ordering ---\n");
    MPI_Barrier(MPI_COMM_WORLD);

    send_val = (rank * 50) + 10;
    recv_val = -1;

    if (rank % 2 == 0) {
        MPI_Send(&send_val, 1, MPI_INT, next, 20, MPI_COMM_WORLD);
        MPI_Recv(&recv_val, 1, MPI_INT, previous, 20, MPI_COMM_WORLD, &status);
    } else {
        MPI_Recv(&recv_val, 1, MPI_INT, previous, 20, MPI_COMM_WORLD, &status);
        MPI_Send(&send_val, 1, MPI_INT, next, 20, MPI_COMM_WORLD);
    }

    if (rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, rank - 1, 992, MPI_COMM_WORLD, &status);
    }
    printf("[Process %d] (Even-Odd) Sent %d to Rank %d | Received %d from Rank %d\n",
           rank, send_val, next, recv_val, previous);
    fflush(stdout);
    if (rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, rank + 1, 992, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}
