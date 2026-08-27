#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <points>\n", argv[0]);
        return 1;
    }

    long long points = atoll(argv[1]);
    long long inside = 0;

    if (points < 1) {
        printf("Points must be greater than 0\n");
        return 1;
    }

    srand(time(NULL));

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long long i = 0; i < points; i++) {
        double x = (double)rand() / RAND_MAX;
        double y = (double)rand() / RAND_MAX;

        if (x * x + y * y <= 1.0) {
            inside++;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec)
                   + (end.tv_nsec - start.tv_nsec) / 1e9;

    double pi = 4.0 * inside / points;

    printf("Total points: %lld\n", points);
    printf("Points inside circle: %lld\n", inside);
    printf("Estimated Pi: %.10f\n", pi);
    printf("Execution time: %.6f seconds\n", elapsed);

    return 0;
}