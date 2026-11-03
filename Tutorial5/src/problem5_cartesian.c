#include <mpi.h>
#include <stdio.h>

/*
 * HPC-2026 - MPI Programming Tutorial
 * Problem 5 Extension: 2D Grid using MPI_Cart_create and MPI_Cart_shift
 * 
 * Objective:
 * Implement a 2x2 process grid using MPI virtual topology functions:
 *   - MPI_Cart_create: creates a Cartesian communicator
 *   - MPI_Cart_coords: retrieves coordinates of a process in the Cartesian grid
 *   - MPI_Cart_shift:  automatically determines source and destination neighbors
 * 
 * Grid Configuration:
 *   Dimensions: [2, 2] (2 rows, 2 columns)
 *   Periods:    [0, 0] (non-periodic boundaries, so no wrap-around)
 *   When a neighbor does not exist, MPI_Cart_shift sets it to MPI_PROC_NULL.
 */

int main(int argc, char **argv) {
    int rank, size;
    int cart_rank;
    MPI_Comm cart_comm;

    int dims[2] = {2, 2};       /* 2x2 grid */
    int periods[2] = {0, 0};    /* Non-periodic: edges do not wrap */
    int reorder = 0;            /* Do not reorder ranks */
    int coords[2];

    int up, down, left, right;
    int recv_up = -1, recv_down = -1, recv_left = -1, recv_right = -1;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0) {
            printf("Error: This Cartesian extension requires exactly 4 processes.\n");
            printf("Usage: mpirun -np 4 ./problem5_cartesian\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("=================================================================\n");
        printf("  Problem 5 Extension: MPI Cartesian Virtual Topology            \n");
        printf("  Using MPI_Cart_create and MPI_Cart_shift                       \n");
        printf("=================================================================\n\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    /* 1. Create 2D Cartesian Communicator */
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cart_comm);
    MPI_Comm_rank(cart_comm, &cart_rank);

    /* 2. Retrieve (row, column) coordinates */
    MPI_Cart_coords(cart_comm, cart_rank, 2, coords);

    /*
     * 3. Find neighbors along each dimension:
     *    Dimension 0: vertical (shift by +1 -> source is up, destination is down)
     *    Dimension 1: horizontal (shift by +1 -> source is left, destination is right)
     */
    MPI_Cart_shift(cart_comm, 0, 1, &up, &down);
    MPI_Cart_shift(cart_comm, 1, 1, &left, &right);

    /* Print process coordinates and discovered neighbors in order */
    int token = 0;
    if (cart_rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, cart_rank - 1, 980, cart_comm, &status);
    }
    char up_str[32], down_str[32], left_str[32], right_str[32];
    if (up == MPI_PROC_NULL) sprintf(up_str, "NONE (MPI_PROC_NULL)"); else sprintf(up_str, "Rank %d", up);
    if (down == MPI_PROC_NULL) sprintf(down_str, "NONE (MPI_PROC_NULL)"); else sprintf(down_str, "Rank %d", down);
    if (left == MPI_PROC_NULL) sprintf(left_str, "NONE (MPI_PROC_NULL)"); else sprintf(left_str, "Rank %d", left);
    if (right == MPI_PROC_NULL) sprintf(right_str, "NONE (MPI_PROC_NULL)"); else sprintf(right_str, "Rank %d", right);

    printf("[Cartesian Process %d] Coordinates: row=%d, col=%d\n", cart_rank, coords[0], coords[1]);
    printf("    Up (Dim 0, -1):    %s\n", up_str);
    printf("    Down (Dim 0, +1):  %s\n", down_str);
    printf("    Left (Dim 1, -1):  %s\n", left_str);
    printf("    Right (Dim 1, +1): %s\n\n", right_str);
    fflush(stdout);

    if (cart_rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, cart_rank + 1, 980, cart_comm);
    }
    MPI_Barrier(cart_comm);

    /* 4. Exchange messages safely with neighbors using MPI_Sendrecv */
    int send_val = (cart_rank + 1) * 25;

    /* Horizontal exchange */
    MPI_Sendrecv(&send_val, 1, MPI_INT, right, 10,
                 &recv_left, 1, MPI_INT, left, 10,
                 cart_comm, &status);

    MPI_Sendrecv(&send_val, 1, MPI_INT, left, 20,
                 &recv_right, 1, MPI_INT, right, 20,
                 cart_comm, &status);

    /* Vertical exchange */
    MPI_Sendrecv(&send_val, 1, MPI_INT, down, 30,
                 &recv_up, 1, MPI_INT, up, 30,
                 cart_comm, &status);

    MPI_Sendrecv(&send_val, 1, MPI_INT, up, 40,
                 &recv_down, 1, MPI_INT, down, 40,
                 cart_comm, &status);

    /* 5. Print exchange results in rank order */
    if (cart_rank == 0) {
        printf("--- Cartesian Grid Exchange Verification ---\n");
        fflush(stdout);
    }
    MPI_Barrier(cart_comm);

    int print_token = 0;
    if (cart_rank != 0) {
        MPI_Recv(&print_token, 1, MPI_INT, cart_rank - 1, 981, cart_comm, &status);
    }

    printf("[Cartesian Process %d] (Sent value %d):\n", cart_rank, send_val);
    if (up != MPI_PROC_NULL)    printf("    Received from UP    (Rank %d): %d\n", up, recv_up);
    if (down != MPI_PROC_NULL)  printf("    Received from DOWN  (Rank %d): %d\n", down, recv_down);
    if (left != MPI_PROC_NULL)  printf("    Received from LEFT  (Rank %d): %d\n", left, recv_left);
    if (right != MPI_PROC_NULL) printf("    Received from RIGHT (Rank %d): %d\n", right, recv_right);
    printf("\n");
    fflush(stdout);

    if (cart_rank < size - 1) {
        MPI_Send(&print_token, 1, MPI_INT, cart_rank + 1, 981, cart_comm);
    }
    MPI_Barrier(cart_comm);

    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}
