/**
 * Tutorial-05 OpenMP: Question 3
 * Monte Carlo Estimation of Pi using OpenMP parallel reduction.
 *
 * Circle inscribed in a square:
 * Square Area = (2R)^2 = 4R^2
 * Circle Area = pi * R^2
 * Ratio P = Circle Area / Square Area = pi / 4  =>  pi = 4 * P
 * P = (Points inside circle) / (Total random points)
 *
 * Runs across points: 10^1, 10^2, 10^4, 10^6, 10^7, 10^8, 10^9
 * Measures error: |estimated_pi - M_PI|
 * Benchmarks thread scalability across 1, 2, 4, 8, 16 threads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <omp.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Fast 64-bit XorShift pseudo-random generator (thread-safe, high throughput)
static inline uint64_t xorshift64(uint64_t* state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

// Generates uniform double in [0.0, 1.0)
static inline double rand_unit(uint64_t* state) {
    return (double)(xorshift64(state) >> 11) * (1.0 / 9007199254740992.0);
}

// Monte Carlo estimation kernel
double estimate_pi_openmp(long long total_points, int num_threads, double* elapsed_time) {
    long long inside_circle = 0;
    double t_start = omp_get_wtime();

    #pragma omp parallel num_threads(num_threads) reduction(+:inside_circle)
    {
        int tid = omp_get_thread_num();
        // Deterministic distinct seed per thread
        uint64_t rng_state = 123456789012345ULL + (uint64_t)tid * 987654321ULL;
        // Warm up RNG
        for (int w = 0; w < 20; w++) xorshift64(&rng_state);

        #pragma omp for schedule(static)
        for (long long i = 0; i < total_points; i++) {
            double x = rand_unit(&rng_state);
            double y = rand_unit(&rng_state);
            if (x * x + y * y <= 1.0) {
                inside_circle++;
            }
        }
    }

    *elapsed_time = omp_get_wtime() - t_start;
    return 4.0 * (double)inside_circle / (double)total_points;
}

int main(int argc, char* argv[]) {
    printf("====================================================================\n");
    printf(" OpenMP Tutorial 05 - Question 3: Monte Carlo Estimation of Pi\n");
    printf("====================================================================\n\n");

    // Requested points in Tutorial 5:
    // 10, 10^2, 10^4, 10^6, 10^7, 10^8, 10^9
    long long points_list[] = {
        10LL,
        100LL,
        10000LL,
        1000000LL,
        10000000LL,
        100000000LL,
        1000000000LL
    };
    int num_experiments = sizeof(points_list) / sizeof(points_list[0]);
    int default_threads = omp_get_max_threads();
    if (default_threads > 16) default_threads = 16;

    FILE* fp_points = fopen("results/pi_results.csv", "w");
    if (fp_points) {
        fprintf(fp_points, "points,estimated_pi,error,time_sec\n");
    }

    printf("[Part 1] Points vs Estimated Pi & Error (using %d threads):\n", default_threads);
    printf("--------------------------------------------------------------------------------\n");
    printf("%-12s | %-16s | %-16s | %-10s\n", "Points", "Estimated Pi", "Absolute Error", "Time (s)");
    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < num_experiments; i++) {
        long long pts = points_list[i];
        double elapsed = 0.0;
        double pi_est = estimate_pi_openmp(pts, default_threads, &elapsed);
        double err = fabs(pi_est - M_PI);

        printf("%-12lld | %-16.10f | %-16.10f | %-10.4f\n", pts, pi_est, err, elapsed);
        if (fp_points) {
            fprintf(fp_points, "%lld,%.12f,%.12f,%.6f\n", pts, pi_est, err, elapsed);
        }
    }
    if (fp_points) fclose(fp_points);
    printf("--------------------------------------------------------------------------------\n\n");

    // [Part 2] Thread scalability benchmark (at N = 10^8 points)
    long long benchmark_n = 100000000LL;
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_tc = sizeof(thread_counts) / sizeof(thread_counts[0]);

    printf("[Part 2] Multi-Thread Scalability Benchmark (N = %lld points):\n", benchmark_n);
    printf("--------------------------------------------------------------------------------\n");
    printf("%-8s | %-12s | %-10s | %-10s | %-12s\n", 
           "Threads", "Time (s)", "Speedup", "Efficiency", "Estimated Pi");
    printf("--------------------------------------------------------------------------------\n");

    FILE* fp_scale = fopen("results/pi_scaling.csv", "w");
    if (fp_scale) {
        fprintf(fp_scale, "threads,time_sec,speedup,efficiency,estimated_pi\n");
    }

    double t_serial = 0.0;
    for (int t = 0; t < num_tc; t++) {
        int th = thread_counts[t];
        double elapsed = 0.0;
        double pi_est = estimate_pi_openmp(benchmark_n, th, &elapsed);
        if (th == 1) t_serial = elapsed;

        double speedup = (t_serial > 0.0) ? (t_serial / elapsed) : 1.0;
        double efficiency = (speedup / th) * 100.0;

        printf("%-8d | %-12.4f | %-10.2fx | %9.1f%% | %-12.8f\n",
               th, elapsed, speedup, efficiency, pi_est);

        if (fp_scale) {
            fprintf(fp_scale, "%d,%.6f,%.4f,%.2f,%.10f\n",
                    th, elapsed, speedup, efficiency, pi_est);
        }
    }
    if (fp_scale) fclose(fp_scale);

    printf("--------------------------------------------------------------------------------\n");
    printf("Results successfully exported to:\n");
    printf("  - results/pi_results.csv\n");
    printf("  - results/pi_scaling.csv\n");
    printf("====================================================================\n");

    return 0;
}
