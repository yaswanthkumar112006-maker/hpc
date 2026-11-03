#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/* Independent dummy computation loop */
void do_dummy_computation(long iterations) {
    volatile double sum = 0.0;
    for (long i = 0; i < iterations; i++) {
        sum += (double)i * 0.0001;
    }
}

int main(int argc, char **argv) {
    int rank, size;
    int num_sizes = 3;
    int counts[3] = {250000, 2500000, 12500000};
    const char *labels[3] = {"1 MB", "10 MB", "50 MB"};
    long dummy_iters = 50000000L;

    double blocking_times[3];
    double nonblocking_times[3];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) printf("Error: At least 2 processes required.\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("Problem 3: Non-Blocking Communication and Overlap\n\n");
    }

    for (int s = 0; s < num_sizes; s++) {
        int count = counts[s];
        int *send_buf = NULL;
        int *recv_buf = NULL;

        if (rank == 0) {
            send_buf = (int *)malloc(count * sizeof(int));
            for (int i = 0; i < count; i++) send_buf[i] = i + 10;
        } else if (rank == 1) {
            recv_buf = (int *)malloc(count * sizeof(int));
            for (int i = 0; i < count; i++) recv_buf[i] = 0;
        }

        /* 1. Sequential Blocking transfer + computation */
        MPI_Barrier(MPI_COMM_WORLD);
        double t_block_start = MPI_Wtime();

        if (rank == 0) {
            MPI_Send(send_buf, count, MPI_INT, 1, 100, MPI_COMM_WORLD);
            do_dummy_computation(dummy_iters);
        } else if (rank == 1) {
            MPI_Status status;
            MPI_Recv(recv_buf, count, MPI_INT, 0, 100, MPI_COMM_WORLD, &status);
            do_dummy_computation(dummy_iters);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        blocking_times[s] = MPI_Wtime() - t_block_start;

        if (rank == 1) {
            for (int i = 0; i < count; i++) recv_buf[i] = 0;
        }

        /* 2. Non-blocking transfer overlapping with computation */
        MPI_Barrier(MPI_COMM_WORLD);
        double t_nonblock_start = MPI_Wtime();
        MPI_Request req;

        if (rank == 0) {
            MPI_Isend(send_buf, count, MPI_INT, 1, 200, MPI_COMM_WORLD, &req);
            do_dummy_computation(dummy_iters);
            MPI_Wait(&req, MPI_STATUS_IGNORE);
        } else if (rank == 1) {
            MPI_Irecv(recv_buf, count, MPI_INT, 0, 200, MPI_COMM_WORLD, &req);
            do_dummy_computation(dummy_iters);
            MPI_Wait(&req, MPI_STATUS_IGNORE);

            printf("[Process 1] Received %s via MPI_Irecv: first = %d, last = %d\n",
                   labels[s], recv_buf[0], recv_buf[count - 1]);
            fflush(stdout);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        nonblocking_times[s] = MPI_Wtime() - t_nonblock_start;

        if (rank == 0) {
            printf("[%s] Blocking: %.6f s | Non-blocking: %.6f s | Saved: %.6f s\n",
                   labels[s], blocking_times[s], nonblocking_times[s],
                   blocking_times[s] - nonblocking_times[s]);
            fflush(stdout);
        }

        if (send_buf) free(send_buf);
        if (recv_buf) free(recv_buf);
    }

    if (rank == 0) {
        printf("\n========================================================================================\n");
        printf("                        PROBLEM 3: COMPARISON TABLE                                     \n");
        printf("========================================================================================\n");
        printf("%-12s | %-17s | %-21s | %-25s\n",
               "Message Size", "Blocking Time (s)", "Non-Blocking Time (s)", "Observation");
        printf("----------------------------------------------------------------------------------------\n");
        for (int s = 0; s < num_sizes; s++) {
            const char *obs = (nonblocking_times[s] < blocking_times[s]) ?
                              "Overlap reduced total time" : "Minimal overlap benefit";
            printf("%-12s | %-17.6f | %-21.6f | %-25s\n",
                   labels[s], blocking_times[s], nonblocking_times[s], obs);
        }
        printf("========================================================================================\n");

        FILE *f = fopen("results/nonblocking_comparison.csv", "w");
        if (f) {
            fprintf(f, "Message_Size,Blocking_Time_s,Non_Blocking_Time_s,Difference_s\n");
            for (int s = 0; s < num_sizes; s++) {
                fprintf(f, "%s,%.6f,%.6f,%.6f\n",
                        labels[s], blocking_times[s], nonblocking_times[s],
                        blocking_times[s] - nonblocking_times[s]);
            }
            fclose(f);
        }
    }

    MPI_Finalize();
    return 0;
}
