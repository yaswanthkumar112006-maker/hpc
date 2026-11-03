#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int rank, size;
    const int ROWS = 2;
    const int COLS = 2;
    int row, col;
    int up, down, left, right;
    int recv_up = -1, recv_down = -1, recv_left = -1, recv_right = -1;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0) printf("Error: 2x2 grid requires 4 processes.\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) printf("Problem 5: 2D Grid Topology (2x2)\n\n");
    MPI_Barrier(MPI_COMM_WORLD);

    /* Row-major grid coordinates */
    row = rank / COLS;
    col = rank % COLS;

    /* Boundary neighbor calculation (MPI_PROC_NULL for edges) */
    up    = (row > 0)        ? (row - 1) * COLS + col : MPI_PROC_NULL;
    down  = (row < ROWS - 1) ? (row + 1) * COLS + col : MPI_PROC_NULL;
    left  = (col > 0)        ? row * COLS + (col - 1) : MPI_PROC_NULL;
    right = (col < COLS - 1) ? row * COLS + (col + 1) : MPI_PROC_NULL;

    int coord_token = 0;
    if (rank != 0) {
        MPI_Recv(&coord_token, 1, MPI_INT, rank - 1, 994, MPI_COMM_WORLD, &status);
    }
    char up_str[32], down_str[32], left_str[32], right_str[32];
    if (up == MPI_PROC_NULL) sprintf(up_str, "NONE"); else sprintf(up_str, "Rank %d", up);
    if (down == MPI_PROC_NULL) sprintf(down_str, "NONE"); else sprintf(down_str, "Rank %d", down);
    if (left == MPI_PROC_NULL) sprintf(left_str, "NONE"); else sprintf(left_str, "Rank %d", left);
    if (right == MPI_PROC_NULL) sprintf(right_str, "NONE"); else sprintf(right_str, "Rank %d", right);

    printf("[Process %d] Grid (%d, %d) -> Up: %s, Down: %s, Left: %s, Right: %s\n",
           rank, row, col, up_str, down_str, left_str, right_str);
    fflush(stdout);

    if (rank < size - 1) {
        MPI_Send(&coord_token, 1, MPI_INT, rank + 1, 994, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* Exchange messages with valid neighbors */
    int send_val = rank * 100;

    MPI_Sendrecv(&send_val, 1, MPI_INT, right, 1, &recv_left, 1, MPI_INT, left, 1, MPI_COMM_WORLD, &status);
    MPI_Sendrecv(&send_val, 1, MPI_INT, left, 2, &recv_right, 1, MPI_INT, right, 2, MPI_COMM_WORLD, &status);
    MPI_Sendrecv(&send_val, 1, MPI_INT, down, 3, &recv_up, 1, MPI_INT, up, 3, MPI_COMM_WORLD, &status);
    MPI_Sendrecv(&send_val, 1, MPI_INT, up, 4, &recv_down, 1, MPI_INT, down, 4, MPI_COMM_WORLD, &status);

    if (rank == 0) printf("\n--- Received Messages ---\n");
    MPI_Barrier(MPI_COMM_WORLD);

    int token = 0;
    if (rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, rank - 1, 995, MPI_COMM_WORLD, &status);
    }
    printf("[Process %d] (Sent %d):\n", rank, send_val);
    if (up != MPI_PROC_NULL)    printf("    From UP    (Rank %d): %d\n", up, recv_up);
    if (down != MPI_PROC_NULL)  printf("    From DOWN  (Rank %d): %d\n", down, recv_down);
    if (left != MPI_PROC_NULL)  printf("    From LEFT  (Rank %d): %d\n", left, recv_left);
    if (right != MPI_PROC_NULL) printf("    From RIGHT (Rank %d): %d\n", right, recv_right);
    fflush(stdout);

    if (rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, rank + 1, 995, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
