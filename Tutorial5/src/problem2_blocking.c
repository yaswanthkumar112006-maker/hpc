#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int rank, size;
    int num_sizes = 3;
    int num_trials = 3;

    /* Message sizes in number of 4-byte integers (1 MB, 10 MB, 50 MB decimal) */
    int counts[3] = {250000, 2500000, 12500000};
    const char *labels[3] = {"1 MB", "10 MB", "50 MB"};

    double times[3][3];
    double avg_times[3];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("Error: This program requires at least 2 processes.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("Problem 2: Blocking Communication (1 MB, 10 MB, 50 MB)\n\n");
    }

    for (int s = 0; s < num_sizes; s++) {
        int count = counts[s];
        double sum_time = 0.0;

        for (int t = 0; t < num_trials; t++) {
            int *buffer = (int *)malloc(count * sizeof(int));
            if (!buffer) {
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            if (rank == 0) {
                for (int i = 0; i < count; i++) {
                    buffer[i] = i + 1;
                }
            }

            MPI_Barrier(MPI_COMM_WORLD);
            double start = MPI_Wtime();

            if (rank == 0) {
                MPI_Send(buffer, count, MPI_INT, 1, 0, MPI_COMM_WORLD);
            } else if (rank == 1) {
                MPI_Status status;
                MPI_Recv(buffer, count, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            }

            MPI_Barrier(MPI_COMM_WORLD);
            double elapsed = MPI_Wtime() - start;

            if (rank == 1) {
                printf("[Process 1] Received %s (Trial %d): first = %d, last = %d\n",
                       labels[s], t + 1, buffer[0], buffer[count - 1]);
                fflush(stdout);
            }

            if (rank == 0) {
                times[s][t] = elapsed;
                sum_time += elapsed;
                printf("[Process 0] %s Trial %d: %.6f seconds\n", labels[s], t + 1, elapsed);
                fflush(stdout);
            }

            free(buffer);
        }

        if (rank == 0) {
            avg_times[s] = sum_time / num_trials;
            printf("[Process 0] Average for %s: %.6f seconds\n\n", labels[s], avg_times[s]);
        }
    }

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

        FILE *f = fopen("results/blocking_results.csv", "w");
        if (f) {
            fprintf(f, "Message_Size_MB,Size_Bytes,Trial1_s,Trial2_s,Trial3_s,Average_s\n");
            int mb_vals[3] = {1, 10, 50};
            for (int s = 0; s < num_sizes; s++) {
                fprintf(f, "%d,%ld,%.6f,%.6f,%.6f,%.6f\n",
                        mb_vals[s], (long)counts[s] * sizeof(int),
                        times[s][0], times[s][1], times[s][2], avg_times[s]);
            }
            fclose(f);
        }
    }

    MPI_Finalize();
    return 0;
}
