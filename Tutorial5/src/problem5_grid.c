#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int rank, size;
    int row, col;
    int up, down, left, right;
    int recv_up = -1, recv_down = -1, recv_left = -1, recv_right = -1;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0) printf("Run with 4 processes for 2x2 grid\n");
        MPI_Finalize();
        return 0;
    }

    // 2x2 grid calculation
    row = rank / 2;
    col = rank % 2;

    up    = (row > 0) ? (row - 1) * 2 + col : MPI_PROC_NULL;
    down  = (row < 1) ? (row + 1) * 2 + col : MPI_PROC_NULL;
    left  = (col > 0) ? row * 2 + (col - 1) : MPI_PROC_NULL;
    right = (col < 1) ? row * 2 + (col + 1) : MPI_PROC_NULL;

    printf("Process %d at (%d, %d) -> Up: %d, Down: %d, Left: %d, Right: %d\n",
           rank, row, col, up, down, left, right);

    int val = rank * 100;

    // Exchange with neighbors
    MPI_Sendrecv(&val, 1, MPI_INT, right, 1, &recv_left, 1, MPI_INT, left, 1, MPI_COMM_WORLD, &status);
    MPI_Sendrecv(&val, 1, MPI_INT, left, 2, &recv_right, 1, MPI_INT, right, 2, MPI_COMM_WORLD, &status);
    MPI_Sendrecv(&val, 1, MPI_INT, down, 3, &recv_up, 1, MPI_INT, up, 3, MPI_COMM_WORLD, &status);
    MPI_Sendrecv(&val, 1, MPI_INT, up, 4, &recv_down, 1, MPI_INT, down, 4, MPI_COMM_WORLD, &status);

    printf("Process %d received -> Up: %d, Down: %d, Left: %d, Right: %d\n",
           rank, recv_up, recv_down, recv_left, recv_right);

    MPI_Finalize();
    return 0;
}
