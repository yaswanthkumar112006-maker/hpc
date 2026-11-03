#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv) {
    int rank, size;
    int cart_rank;
    MPI_Comm cart_comm;

    int dims[2] = {2, 2};       /* 2x2 grid */
    int periods[2] = {0, 0};    /* Non-periodic */
    int reorder = 0;
    int coords[2];

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

    if (rank == 0) printf("Problem 5 Extension: MPI Cartesian Topology\n\n");
    MPI_Barrier(MPI_COMM_WORLD);

    /* Create 2D Cartesian Communicator */
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, reorder, &cart_comm);
    MPI_Comm_rank(cart_comm, &cart_rank);
    MPI_Cart_coords(cart_comm, cart_rank, 2, coords);

    /* Determine neighbors along dimensions 0 and 1 */
    MPI_Cart_shift(cart_comm, 0, 1, &up, &down);
    MPI_Cart_shift(cart_comm, 1, 1, &left, &right);

    int token = 0;
    if (cart_rank != 0) {
        MPI_Recv(&token, 1, MPI_INT, cart_rank - 1, 980, cart_comm, &status);
    }
    char up_str[32], down_str[32], left_str[32], right_str[32];
    if (up == MPI_PROC_NULL) sprintf(up_str, "NONE"); else sprintf(up_str, "Rank %d", up);
    if (down == MPI_PROC_NULL) sprintf(down_str, "NONE"); else sprintf(down_str, "Rank %d", down);
    if (left == MPI_PROC_NULL) sprintf(left_str, "NONE"); else sprintf(left_str, "Rank %d", left);
    if (right == MPI_PROC_NULL) sprintf(right_str, "NONE"); else sprintf(right_str, "Rank %d", right);

    printf("[Cartesian %d] Coords (%d, %d) -> Up: %s, Down: %s, Left: %s, Right: %s\n",
           cart_rank, coords[0], coords[1], up_str, down_str, left_str, right_str);
    fflush(stdout);

    if (cart_rank < size - 1) {
        MPI_Send(&token, 1, MPI_INT, cart_rank + 1, 980, cart_comm);
    }
    MPI_Barrier(cart_comm);

    /* Exchange messages */
    int send_val = (cart_rank + 1) * 25;

    MPI_Sendrecv(&send_val, 1, MPI_INT, right, 10, &recv_left, 1, MPI_INT, left, 10, cart_comm, &status);
    MPI_Sendrecv(&send_val, 1, MPI_INT, left, 20, &recv_right, 1, MPI_INT, right, 20, cart_comm, &status);
    MPI_Sendrecv(&send_val, 1, MPI_INT, down, 30, &recv_up, 1, MPI_INT, up, 30, cart_comm, &status);
    MPI_Sendrecv(&send_val, 1, MPI_INT, up, 40, &recv_down, 1, MPI_INT, down, 40, cart_comm, &status);

    if (cart_rank == 0) printf("\n--- Cartesian Exchange Verification ---\n");
    MPI_Barrier(cart_comm);

    int print_token = 0;
    if (cart_rank != 0) {
        MPI_Recv(&print_token, 1, MPI_INT, cart_rank - 1, 981, cart_comm, &status);
    }

    printf("[Cartesian %d] (Sent %d):\n", cart_rank, send_val);
    if (up != MPI_PROC_NULL)    printf("    From UP    (Rank %d): %d\n", up, recv_up);
    if (down != MPI_PROC_NULL)  printf("    From DOWN  (Rank %d): %d\n", down, recv_down);
    if (left != MPI_PROC_NULL)  printf("    From LEFT  (Rank %d): %d\n", left, recv_left);
    if (right != MPI_PROC_NULL) printf("    From RIGHT (Rank %d): %d\n", right, recv_right);
    fflush(stdout);

    if (cart_rank < size - 1) {
        MPI_Send(&print_token, 1, MPI_INT, cart_rank + 1, 981, cart_comm);
    }
    MPI_Barrier(cart_comm);

    MPI_Comm_free(&cart_comm);
    MPI_Finalize();
    return 0;
}
