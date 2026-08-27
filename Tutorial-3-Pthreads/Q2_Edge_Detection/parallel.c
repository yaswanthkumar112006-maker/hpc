#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 1000
#define MAX_THREADS 16

typedef struct {
    int start_row;
    int end_row;
    int (*image)[N];
    int (*output)[N];
} ThreadData;

int kernel[3][3] = {
    {-1, -1, -1},
    {-1,  8, -1},
    {-1, -1, -1}
};

void* edge_detection(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 1; j < N - 1; j++) {

            int sum = 0;

            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    sum += data->image[i + ki][j + kj]
                         * kernel[ki + 1][kj + 1];
                }
            }

            if (sum < 0)
                sum = 0;

            if (sum > 255)
                sum = 255;

            data->output[i][j] = sum;
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Usage: %s <threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);

    if (num_threads < 1 || num_threads > MAX_THREADS) {
        printf("Threads must be between 1 and %d\n", MAX_THREADS);
        return 1;
    }

    static int image[N][N];
    static int output[N][N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            image[i][j] = rand() % 256;
        }
    }

    pthread_t threads[MAX_THREADS];
    ThreadData data[MAX_THREADS];

    int rows_per_thread = (N - 2) / num_threads;
    int remainder = (N - 2) % num_threads;

    int current_row = 1;

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {

        int rows = rows_per_thread;

        if (i < remainder)
            rows++;

        data[i].start_row = current_row;
        data[i].end_row = current_row + rows;
        data[i].image = image;
        data[i].output = output;

        pthread_create(
            &threads[i],
            NULL,
            edge_detection,
            &data[i]
        );

        current_row += rows;
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    double serial_time = 0.036293;

    double speedup = serial_time / elapsed;

    double efficiency =
        (speedup / num_threads) * 100;

    printf("Threads: %d\n", num_threads);
    printf("Image size: %d x %d\n", N, N);
    printf("Execution time: %.6f seconds\n", elapsed);
    printf("Speedup: %.6f\n", speedup);
    printf("Efficiency: %.2f%%\n", efficiency);

    return 0;
}