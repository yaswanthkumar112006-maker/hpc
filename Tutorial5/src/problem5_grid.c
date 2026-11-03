#include <mpi.h>
#include <stdio.h>

/*
 * HPC-2026 - MPI Programming Tutorial
 * Problem 5: Two-Dimensional Grid Topology
 * 
 * Objective:
 * Arrange processes in a 2D grid (2x2) and exchange messages with valid
 * neighbors in the up, down, left, and right directions.
 * 
 * Layout:
 *   Process 0 | Process 1
 *   ----------+----------
 *   Process 2 | Process 3
 * 
 * Coordinates:
 *   row    = rank / C;
 *   column = rank % C;
 * 
 * Neighbors for 2x2:
 *   - Process 0: right = 1, down = 2 (left = None, up = None)
 *   - Process 1: left = 0, down = 3  (right = None, up = None)
 *   - Process 2: up = 0, right = 3   (down = None, left = None)
 *   - Process 3: up = 1, left = 2    (down = None, right = None)
 */

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
        if (rank == 0) {
            printf("Error: Problem 5 requires exactly 4 processes for a 2x2 grid.\n");
            printf("Usage: mpirun -np 4 ./problem5_grid\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("=================================================================\n");
        printf("  Problem 5: Two-Dimensional Grid Topology (2x2 Grid)            \n");
        printf("=================================================================\n\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* 1. Calculate row and column in row-major layout */
    row = rank / COLS;
    col = rank % COLS;

    /*
     * 2. Identify neighbors and handle boundaries.
     * Use MPI_PROC_NULL for non-existent neighbors beyond the boundary.
     * MPI automatically ignores sends and receives to MPI_PROC_NULL safely.
     */
    up    = (row > 0)        ? (row - 1) * COLS + col : MPI_PROC_NULL;
    down  = (row < ROWS - 1) ? (row + 1) * COLS + col : MPI_PROC_NULL;
    left  = (col > 0)        ? row * COLS + (col - 1) : MPI_PROC_NULL;
    right = (col < COLS - 1) ? row * COLS + (col + 1) : MPI_PROC_NULL;

    /* Print process coordinates and identified neighbors */
    int coord_token = 0;
    if (rank != 0) {
        MPI_Recv(&coord_token, 1, MPI_INT, rank - 1, 994, MPI_COMM_WORLD, &status);
    }
    char up_str[32], down_str[32], left_str[32], right_str[32];
    if (up == MPI_PROC_NULL) sprintf(up_str, "NONE (Boundary)"); else sprintf(up_str, "Rank %d", up);
    if (down == MPI_PROC_NULL) sprintf(down_str, "NONE (Boundary)"); else sprintf(down_str, "Rank %d", down);
    if (left == MPI_PROC_NULL) sprintf(left_str, "NONE (Boundary)"); else sprintf(left_str, "Rank %d", left);
    if (right == MPI_PROC_NULL) sprintf(right_str, "NONE (Boundary)"); else sprintf(right_str, "Rank %d", right);

    printf("[Process %d] at Grid Position (row=%d, col=%d):\n", rank, row, col);
    printf("    Up:    %s\n", up_str);
    printf("    Down:  %s\n", down_str);
    printf("    Left:  %s\n", left_str);
    printf("    Right: %s\n\n", right_str);
    fflush(stdout);

    if (rank < size - 1) {
        MPI_Send(&coord_token, 1, MPI_INT, rank + 1, 994, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* ------------------------------------------------------------- */
    /* 3. Exchange messages with valid neighbors                     */
    /* ------------------------------------------------------------- */
    int send_val = rank * 100;

    /* Exchange horizontally: Send to Right, Receive from Left */
    MPI_Sendrecv(&send_val, 1, MPI_INT, right, 1,
                 &recv_left, 1, MPI_INT, left, 1,
                 MPI_COMM_WORLD, &status);

    /* Exchange horizontally: Send to Left, Receive from Right */
    MPI_Sendrecv(&send_val, 1, MPI_INT, left, 2,
                 &recv_right, 1, MPI_INT, right, 2,
                 MPI_COMM_WORLD, &status);

    /* Exchange vertically: Send to Down, Receive from Up */
    MPI_Sendrecv(&send_val, 1, MPI_INT, down, 3,
                 &recv_up, 1, MPI_INT, up, 3,
                 MPI_COMM_WORLD, &status);

    /* Exchange vertically: Send to Up, Receive from Down */
    MPI_Sendrecv(&send_val, 1, MPI_INT, up, 4,
                 &recv_down, 1, MPI_INT, down, 4,
                 MPI_COMM_WORLD, &status);

    /* 4. Print received messages for each process */
    if (rank == 0) {
        printf("--- Message Exchange Verification ---\n");
        fflush(stdout);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    int token = 0;
    if (rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, rank - 1, 995, MPI_COMM_WORLD, &status);
    }
    printf("[Process %d] (Sent value %d):\n", rank, send_val);
    if (up != MPI_PROC_NULL)    printf("    Received from UP    (Rank %d): %d\n", up, recv_up);
    if (down != MPI_PROC_NULL)  printf("    Received from DOWN  (Rank %d): %d\n", down, recv_down);
    if (left != MPI_PROC_NULL)  printf("    Received from LEFT  (Rank %d): %d\n", left, recv_left);
    if (right != MPI_PROC_NULL) printf("    Received from RIGHT (Rank %d): %d\n", right, recv_right);
    printf("\n");
    fflush(stdout);
    if (rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, rank + 1, 995, MPI_COMM_WORLD);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();
    return 0;
}
