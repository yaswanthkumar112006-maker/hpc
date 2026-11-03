#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Comm grid_comm;
    int dims[2] = {2, 2};
    int periods[2] = {0, 0};
    int coords[2];
    int up, down, left, right;
    int val, recv_val;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 4) {
        if (rank == 0) printf("Run with 4 processes\n");
        MPI_Finalize();
        return 0;
    }

    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &grid_comm);
    MPI_Cart_coords(grid_comm, rank, 2, coords);

    MPI_Cart_shift(grid_comm, 0, 1, &up, &down);
    MPI_Cart_shift(grid_comm, 1, 1, &left, &right);

    printf("Cartesian process %d at (%d, %d) -> Up: %d, Down: %d, Left: %d, Right: %d\n",
           rank, coords[0], coords[1], up, down, left, right);

    val = rank * 10;
    MPI_Sendrecv(&val, 1, MPI_INT, right, 0, &recv_val, 1, MPI_INT, left, 0, grid_comm, &status);

    MPI_Comm_free(&grid_comm);
    MPI_Finalize();
    return 0;
}
