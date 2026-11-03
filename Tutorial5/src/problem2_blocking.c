#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * HPC-2026 - MPI Programming Tutorial
 * Problem 2: Blocking Point-to-Point Communication
 * 
 * Objective:
 * Send an integer array from Process 0 to Process 1 using MPI_Send and MPI_Recv,
 * and measure elapsed time using MPI_Wtime for:
 *   - 1 MB  (250,000 4-byte integers)
 *   - 10 MB (2,500,000 4-byte integers)
 *   - 50 MB (12,500,000 4-byte integers)
 * Run 3 trials for each size and compute the average time.
 */

int main(int argc, char **argv) {
    int rank, size;
    int num_sizes = 3;
    int num_trials = 3;

    /* Message sizes in number of 4-byte integers based on decimal MB */
    int counts[3] = {250000, 2500000, 12500000};
    const char *labels[3] = {"1 MB", "10 MB", "50 MB"};

    /* Array to store trial times and average times on process 0 */
    double times[3][3];
    double avg_times[3];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* This experiment is designed for 2 processes */
    if (size < 2) {
        if (rank == 0) {
            printf("Error: This program requires at least 2 processes.\n");
            printf("Usage: mpirun -np 2 ./problem2_blocking\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("=================================================================\n");
        printf("  Problem 2: Blocking Point-to-Point Communication (MPI_Send/Recv)\n");
        printf("  Testing 1 MB, 10 MB, and 50 MB with 3 trials each\n");
        printf("=================================================================\n\n");
    }

    /* Loop through each message size */
    for (int s = 0; s < num_sizes; s++) {
        int count = counts[s];

        if (rank == 0) {
            printf("--- Testing Message Size: %s (%d integers, ~%ld bytes) ---\n",
                   labels[s], count, (long)count * sizeof(int));
        }

        double sum_time = 0.0;

        for (int t = 0; t < num_trials; t++) {
            int *buffer = NULL;
            MPI_Status status;

            /* Allocate memory on both processes */
            buffer = (int *)malloc(count * sizeof(int));
            if (buffer == NULL) {
                printf("[Process %d] Memory allocation failed for %s!\n", rank, labels[s]);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            /* Process 0 initializes the array data */
            if (rank == 0) {
                for (int i = 0; i < count; i++) {
                    buffer[i] = i + 1; /* Sample data: 1, 2, 3... */
                }
            }

            /* Synchronize before timing to ensure both processes are ready */
            MPI_Barrier(MPI_COMM_WORLD);
            double start_time = MPI_Wtime();

            /* Process 0 sends, Process 1 receives */
            if (rank == 0) {
                MPI_Send(buffer, count, MPI_INT, 1, 0, MPI_COMM_WORLD);
            } else if (rank == 1) {
                MPI_Recv(buffer, count, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            }

            /* Synchronize after communication to capture full transfer time */
            MPI_Barrier(MPI_COMM_WORLD);
            double elapsed_time = MPI_Wtime() - start_time;

            /* Process 1 verifies and prints first and last element (as required by PDF) */
            if (rank == 1) {
                printf("[Process 1] Received %s (Trial %d): First element = %d, Last element = %d\n",
                       labels[s], t + 1, buffer[0], buffer[count - 1]);
                fflush(stdout);
            }

            /* Process 0 records timing */
            if (rank == 0) {
                times[s][t] = elapsed_time;
                sum_time += elapsed_time;
                printf("[Process 0] Trial %d: Elapsed time = %.6f seconds\n", t + 1, elapsed_time);
                fflush(stdout);
            }

            /* Free buffer */
            free(buffer);
        }

        if (rank == 0) {
            avg_times[s] = sum_time / num_trials;
            printf("[Process 0] Average time for %s = %.6f seconds\n\n", labels[s], avg_times[s]);
        }
    }

    /* Print final results table on Process 0 */
    if (rank == 0) {
        printf("=================================================================\n");
        printf("                   TIMING RESULTS SUMMARY TABLE                  \n");
        printf("=================================================================\n");
        printf("%-12s | %-12s | %-12s | %-12s | %-12s\n",
               "Message Size", "Trial 1 (s)", "Trial 2 (s)", "Trial 3 (s)", "Average (s)");
        printf("-----------------------------------------------------------------\n");
        for (int s = 0; s < num_sizes; s++) {
            printf("%-12s | %-12.6f | %-12.6f | %-12.6f | %-12.6f\n",
                   labels[s], times[s][0], times[s][1], times[s][2], avg_times[s]);
        }
        printf("=================================================================\n");

        /* Save results to CSV for easy plotting */
        FILE *f = fopen("results/blocking_results.csv", "w");
        if (f != NULL) {
            fprintf(f, "Message_Size_MB,Size_Bytes,Trial1_s,Trial2_s,Trial3_s,Average_s\n");
            int mb_vals[3] = {1, 10, 50};
            for (int s = 0; s < num_sizes; s++) {
                fprintf(f, "%d,%ld,%.6f,%.6f,%.6f,%.6f\n",
                        mb_vals[s], (long)counts[s] * sizeof(int),
                        times[s][0], times[s][1], times[s][2], avg_times[s]);
            }
            fclose(f);
            printf("\nResults saved to results/blocking_results.csv for plotting.\n");
        }
    }

    MPI_Finalize();
    return 0;
}
