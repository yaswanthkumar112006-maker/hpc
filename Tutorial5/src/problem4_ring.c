#include <mpi.h>
#include <stdio.h>

/*
 * HPC-2026 - MPI Programming Tutorial
 * Problem 4: Ring Communication Topology
 * 
 * Objective:
 * Connect processes in a logical ring. Each process sends a message to its next
 * neighbor and receives a message from its previous neighbor.
 * 
 * Formulations:
 *   next     = (rank + 1) % size;
 *   previous = (rank - 1 + size) % size;
 * 
 * Deadlock Avoidance:
 * If every process executes MPI_Send first before MPI_Recv, and system buffers
 * are insufficient or synchronous protocols are used, all processes block
 * waiting for receivers -> circular deadlock.
 * 
 * We demonstrate two safe implementations:
 *   Method 1: MPI_Sendrecv (built-in atomic send and receive)
 *   Method 2: Carefully designed Even-Odd ordering
 */

int main(int argc, char **argv) {
    int rank, size;
    int next, previous;
    int send_val, recv_val;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 4) {
        if (rank == 0) {
            printf("Warning: PDF recommends testing with at least 4 processes.\n");
            printf("Current size: %d processes.\n\n", size);
        }
    }

    /* Compute logical ring neighbors */
    next = (rank + 1) % size;
    previous = (rank - 1 + size) % size;

    if (rank == 0) {
        printf("=================================================================\n");
        printf("  Problem 4: Ring Communication Topology (Size = %d processes)   \n", size);
        printf("=================================================================\n\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* Print neighbor mapping for each rank in order using a simple token pass */
    int token = 0;
    if (rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, rank - 1, 990, MPI_COMM_WORLD, &status);
    }
    printf("[Process %d] Neighbors -> Previous: %d | Next: %d\n", rank, previous, next);
    fflush(stdout);
    if (rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, rank + 1, 990, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* ------------------------------------------------------------- */
    /* Method 1: Deadlock-free communication using MPI_Sendrecv       */
    /* ------------------------------------------------------------- */
    if (rank == 0) {
        printf("\n--- Step 1: Communication using MPI_Sendrecv ---\n");
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* Each process generates a distinct message value: (rank * 10) + 100 */
    send_val = (rank * 10) + 100;
    recv_val = -1;

    /* MPI_Sendrecv combines send and receive in one call without deadlock */
    MPI_Sendrecv(&send_val, 1, MPI_INT, next, 10,
                 &recv_val, 1, MPI_INT, previous, 10,
                 MPI_COMM_WORLD, &status);

    /* Ordered printing of Step 1 results */
    if (rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, rank - 1, 991, MPI_COMM_WORLD, &status);
    }
    printf("[Process %d] Sent %d to Next (Rank %d) | Received %d from Previous (Rank %d)\n",
           rank, send_val, next, recv_val, previous);
    fflush(stdout);
    if (rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, rank + 1, 991, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* ------------------------------------------------------------- */
    /* Method 2: Deadlock-free communication using Even-Odd Ordering  */
    /* ------------------------------------------------------------- */
    if (rank == 0) {
        printf("\n--- Step 2: Communication using Even-Odd Send/Recv Ordering ---\n");
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    send_val = (rank * 50) + 10;
    recv_val = -1;

    /*
     * Even ranks send first then receive.
     * Odd ranks receive first then send.
     * This breaks circular wait and eliminates deadlock.
     */
    if (rank % 2 == 0) {
        MPI_Send(&send_val, 1, MPI_INT, next, 20, MPI_COMM_WORLD);
        MPI_Recv(&recv_val, 1, MPI_INT, previous, 20, MPI_COMM_WORLD, &status);
    } else {
        MPI_Recv(&recv_val, 1, MPI_INT, previous, 20, MPI_COMM_WORLD, &status);
        MPI_Send(&send_val, 1, MPI_INT, next, 20, MPI_COMM_WORLD);
    }

    /* Ordered printing of Step 2 results */
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
