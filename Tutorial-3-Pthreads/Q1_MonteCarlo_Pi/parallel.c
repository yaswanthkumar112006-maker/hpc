#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 16

typedef struct {
    long long points;
    unsigned int seed;
    long long inside;
} ThreadData;

void* calculate_pi(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    data->inside = 0;

    for (long long i = 0; i < data->points; i++) {
        double x = (double)rand_r(&data->seed) / RAND_MAX;
        double y = (double)rand_r(&data->seed) / RAND_MAX;

        if (x * x + y * y <= 1.0) {
            data->inside++;
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {

    struct timespec start, end;

    if (argc != 3) {
        printf("Usage: %s <threads> <points>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    long long total_points = atoll(argv[2]);

    if (num_threads < 1 || num_threads > MAX_THREADS) {
        printf("Threads must be between 1 and %d\n", MAX_THREADS);
        return 1;
    }

    if (total_points < 1) {
        printf("Points must be greater than 0\n");
        return 1;
    }

    pthread_t threads[MAX_THREADS];
    ThreadData data[MAX_THREADS];

    long long points_per_thread = total_points / num_threads;
    long long remainder = total_points % num_threads;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) {
        data[i].points = points_per_thread;

        if (i < remainder) {
            data[i].points++;
        }

        data[i].inside = 0;
        data[i].seed = time(NULL) + i;

        pthread_create(&threads[i], NULL, calculate_pi, &data[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    long long total_inside = 0;

    for (int i = 0; i < num_threads; i++) {
        total_inside += data[i].inside;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec)
                   + (end.tv_nsec - start.tv_nsec) / 1e9;

    double pi = 4.0 * total_inside / total_points;

    double serial_time = 0.024451;
    double speedup = serial_time / elapsed;
    double efficiency = (speedup / num_threads) * 100;

    printf("Threads: %d\n", num_threads);
    printf("Total points: %lld\n", total_points);
    printf("Points inside circle: %lld\n", total_inside);
    printf("Estimated Pi: %.10f\n", pi);
    printf("Execution time: %.6f seconds\n", elapsed);
    printf("Speedup: %.6f\n", speedup);
    printf("Efficiency: %.2f%%\n", efficiency);

    return 0;
}