#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * HPC-2026 - MPI Programming Tutorial
 * Problem 3: Non-Blocking Communication and Overlap
 * 
 * Objective:
 * Replace MPI_Send / MPI_Recv with MPI_Isend / MPI_Irecv and MPI_Wait.
 * Investigate whether independent dummy computation can overlap with
 * communication for:
 *   - 1 MB  (250,000 4-byte integers)
 *   - 10 MB (2,500,000 4-byte integers)
 *   - 50 MB (12,500,000 4-byte integers)
 * Compare non-blocking overlapped execution against sequential blocking execution.
 */

/* Dummy computation function (independent work) */
void do_dummy_computation(long iterations) {
    volatile double dummy_sum = 0.0;
    for (long i = 0; i < iterations; i++) {
        dummy_sum += (double)i * 0.0001;
    }
}

int main(int argc, char **argv) {
    int rank, size;
    int num_sizes = 3;

    /* Message sizes in number of 4-byte integers based on decimal MB */
    int counts[3] = {250000, 2500000, 12500000};
    const char *labels[3] = {"1 MB", "10 MB", "50 MB"};
    long dummy_iters = 50000000L; /* Reasonable computational workload */

    double blocking_times[3];
    double nonblocking_times[3];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("Error: This program requires at least 2 processes.\n");
            printf("Usage: mpirun -np 2 ./problem3_nonblocking\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("=================================================================\n");
        printf("  Problem 3: Non-Blocking Communication & Computation Overlap   \n");
        printf("  Using MPI_Isend, MPI_Irecv, and MPI_Wait                       \n");
        printf("=================================================================\n\n");
    }

    for (int s = 0; s < num_sizes; s++) {
        int count = counts[s];

        /* Allocate memory */
        int *send_buf = NULL;
        int *recv_buf = NULL;

        if (rank == 0) {
            send_buf = (int *)malloc(count * sizeof(int));
            if (!send_buf) {
                printf("[Process 0] Memory allocation failed for %s!\n", labels[s]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            for (int i = 0; i < count; i++) {
                send_buf[i] = i + 10;
            }
        } else if (rank == 1) {
            recv_buf = (int *)malloc(count * sizeof(int));
            if (!recv_buf) {
                printf("[Process 1] Memory allocation failed for %s!\n", labels[s]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            for (int i = 0; i < count; i++) {
                recv_buf[i] = 0;
            }
        }

        /* ------------------------------------------------------------- */
        /* Part A: Sequential Blocking Communication + Computation       */
        /* ------------------------------------------------------------- */
        MPI_Barrier(MPI_COMM_WORLD);
        double t_block_start = MPI_Wtime();

        if (rank == 0) {
            /* Send data first */
            MPI_Send(send_buf, count, MPI_INT, 1, 100, MPI_COMM_WORLD);
            /* Perform dummy computation afterwards */
            do_dummy_computation(dummy_iters);
        } else if (rank == 1) {
            MPI_Status status;
            /* Receive data first */
            MPI_Recv(recv_buf, count, MPI_INT, 0, 100, MPI_COMM_WORLD, &status);
            /* Perform dummy computation afterwards */
            do_dummy_computation(dummy_iters);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double t_block_end = MPI_Wtime();
        blocking_times[s] = t_block_end - t_block_start;

        /* Reset receive buffer for the non-blocking test */
        if (rank == 1) {
            for (int i = 0; i < count; i++) {
                recv_buf[i] = 0;
            }
        }

        /* ------------------------------------------------------------- */
        /* Part B: Non-Blocking Overlapped Communication + Computation   */
        /* ------------------------------------------------------------- */
        MPI_Barrier(MPI_COMM_WORLD);
        double t_nonblock_start = MPI_Wtime();
        MPI_Request request;

        if (rank == 0) {
            /* 1. Initiate non-blocking send */
            MPI_Isend(send_buf, count, MPI_INT, 1, 200, MPI_COMM_WORLD, &request);

            /* 2. Overlap: Perform independent computation while send is in progress */
            do_dummy_computation(dummy_iters);

            /* 3. Wait for send completion before releasing/reusing buffer */
            MPI_Wait(&request, MPI_STATUS_IGNORE);
        } else if (rank == 1) {
            /* 1. Initiate non-blocking receive */
            MPI_Irecv(recv_buf, count, MPI_INT, 0, 200, MPI_COMM_WORLD, &request);

            /* 2. Overlap: Perform independent computation while receive is in progress */
            do_dummy_computation(dummy_iters);

            /* 3. Wait for receive completion before accessing received data */
            MPI_Wait(&request, MPI_STATUS_IGNORE);

            /* Verification of received elements */
            printf("[Process 1] Received %s via MPI_Irecv: First = %d, Last = %d\n",
                   labels[s], recv_buf[0], recv_buf[count - 1]);
            fflush(stdout);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        double t_nonblock_end = MPI_Wtime();
        nonblocking_times[s] = t_nonblock_end - t_nonblock_start;

        if (rank == 0) {
            printf("[Size: %s] Blocking Time: %.6f s | Non-Blocking Time: %.6f s | Time Saved: %.6f s\n",
                   labels[s], blocking_times[s], nonblocking_times[s],
                   blocking_times[s] - nonblocking_times[s]);
            fflush(stdout);
        }

        if (send_buf) free(send_buf);
        if (recv_buf) free(recv_buf);
    }

    /* Print final comparison table required by Section 7.2 of PDF */
    if (rank == 0) {
        printf("\n========================================================================================\n");
        printf("                        PROBLEM 3: COMPARISON TABLE                                     \n");
        printf("========================================================================================\n");
        printf("%-12s | %-17s | %-21s | %-25s\n",
               "Message Size", "Blocking Time (s)", "Non-Blocking Time (s)", "Observation");
        printf("----------------------------------------------------------------------------------------\n");
        for (int s = 0; s < num_sizes; s++) {
            const char *obs = (nonblocking_times[s] < blocking_times[s]) ?
                              "Overlap reduced total time" :
                              "Minimal overlap benefit";
            printf("%-12s | %-17.6f | %-21.6f | %-25s\n",
                   labels[s], blocking_times[s], nonblocking_times[s], obs);
        }
        printf("========================================================================================\n");

        /* Save to CSV */
        FILE *f = fopen("results/nonblocking_comparison.csv", "w");
        if (f != NULL) {
            fprintf(f, "Message_Size,Blocking_Time_s,Non_Blocking_Time_s,Difference_s\n");
            for (int s = 0; s < num_sizes; s++) {
                fprintf(f, "%s,%.6f,%.6f,%.6f\n",
                        labels[s], blocking_times[s], nonblocking_times[s],
                        blocking_times[s] - nonblocking_times[s]);
            }
            fclose(f);
            printf("\nSaved comparison data to results/nonblocking_comparison.csv\n");
        }
    }

    MPI_Finalize();
    return 0;
}
