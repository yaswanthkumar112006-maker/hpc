#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1000

int main() {

    int image[N][N];
    int output[N][N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            image[i][j] = rand() % 256;
        }
    }

    int kernel[3][3] = {
        {-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 1; i < N - 1; i++) {
        for (int j = 1; j < N - 1; j++) {

            int sum = 0;

            for (int ki = -1; ki <= 1; ki++) {
                for (int kj = -1; kj <= 1; kj++) {
                    sum += image[i + ki][j + kj]
                         * kernel[ki + 1][kj + 1];
                }
            }

            if (sum < 0)
                sum = 0;

            if (sum > 255)
                sum = 255;

            output[i][j] = sum;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Image size: %d x %d\n", N, N);
    printf("Execution time: %.6f seconds\n", elapsed);

    return 0;
}