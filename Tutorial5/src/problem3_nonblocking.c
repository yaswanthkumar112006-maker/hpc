#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void dummy_work() {
    double x = 0.0;
    for (long i = 0; i < 50000000; i++) {
        x += i * 0.001;
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int sizes[] = {250000, 2500000, 12500000};
    char *names[] = {"1 MB", "10 MB", "50 MB"};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0) printf("Run with 2 processes: mpirun -np 2 ./bin/problem3_nonblocking\n");
        MPI_Finalize();
        return 0;
    }

    for (int s = 0; s < 3; s++) {
        int n = sizes[s];
        int *buf = (int *)malloc(n * sizeof(int));
        MPI_Request req;

        if (rank == 0) {
            for (int i = 0; i < n; i++) buf[i] = i;
        }

        /* 1. Blocking send + compute */
        MPI_Barrier(MPI_COMM_WORLD);
        double b_start = MPI_Wtime();
        if (rank == 0) {
            MPI_Send(buf, n, MPI_INT, 1, 0, MPI_COMM_WORLD);
            dummy_work();
        } else {
            MPI_Status st;
            MPI_Recv(buf, n, MPI_INT, 0, 0, MPI_COMM_WORLD, &st);
            dummy_work();
        }
        MPI_Barrier(MPI_COMM_WORLD);
        double b_time = MPI_Wtime() - b_start;

        /* 2. Non-blocking send with overlap */
        MPI_Barrier(MPI_COMM_WORLD);
        double nb_start = MPI_Wtime();
        if (rank == 0) {
            MPI_Isend(buf, n, MPI_INT, 1, 1, MPI_COMM_WORLD, &req);
            dummy_work();
            MPI_Wait(&req, MPI_STATUS_IGNORE);
        } else {
            MPI_Irecv(buf, n, MPI_INT, 0, 1, MPI_COMM_WORLD, &req);
            dummy_work();
            MPI_Wait(&req, MPI_STATUS_IGNORE);
            printf("[Process 1] Received %s: first = %d, last = %d\n", names[s], buf[0], buf[n - 1]);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        double nb_time = MPI_Wtime() - nb_start;

        if (rank == 0) {
            printf("%s -> Blocking: %f s, Non-blocking: %f s\n", names[s], b_time, nb_time);
        }

        free(buf);
    }

    MPI_Finalize();
    return 0;
}
