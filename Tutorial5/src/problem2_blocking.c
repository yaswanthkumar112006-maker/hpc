#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int rank, size;
    int sizes[] = {250000, 2500000, 12500000};
    char *names[] = {"1 MB", "10 MB", "50 MB"};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) printf("Run with 2 processes: mpirun -np 2 ./bin/problem2_blocking\n");
        MPI_Finalize();
        return 0;
    }

    for (int s = 0; s < 3; s++) {
        int n = sizes[s];
        double total_time = 0.0;

        for (int trial = 1; trial <= 3; trial++) {
            int *arr = (int *)malloc(n * sizeof(int));

            if (rank == 0) {
                for (int i = 0; i < n; i++) arr[i] = i;
            }

            MPI_Barrier(MPI_COMM_WORLD);
            double t1 = MPI_Wtime();

            if (rank == 0) {
                MPI_Send(arr, n, MPI_INT, 1, 0, MPI_COMM_WORLD);
            } else if (rank == 1) {
                MPI_Status status;
                MPI_Recv(arr, n, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            }

            MPI_Barrier(MPI_COMM_WORLD);
            double t2 = MPI_Wtime();
            double elapsed = t2 - t1;

            if (rank == 1) {
                printf("[Process 1] Received %s (Trial %d): first = %d, last = %d\n", names[s], trial, arr[0], arr[n - 1]);
            }
            if (rank == 0) {
                printf("[Process 0] Size %s, Trial %d: %f s\n", names[s], trial, elapsed);
                total_time += elapsed;
            }

            free(arr);
        }

        if (rank == 0) {
            printf("--> Average time for %s: %f s\n\n", names[s], total_time / 3.0);
        }
    }

    MPI_Finalize();
    return 0;
}
