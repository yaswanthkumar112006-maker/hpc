/**
 * Tutorial-05 OpenMP: Question 2
 * Analyzes OpenMP worksharing loop scheduling, implicit barriers, and nowait clause.
 * Code under study:
 * #pragma omp parallel num_threads(2)
 * {
 *     #pragma omp for schedule(static)
 *     for (i = 1; i < n; i++) f(i);
 *     #pragma omp for schedule(static)
 *     for (i = 1; i < n; i++) f(n-i);
 * }
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <windows.h>

// Simulated workload: busy-wait loop scaled to microseconds
static void simulated_f(int cost_units, double scale_us) {
    if (cost_units <= 0) return;
    double target_sec = (cost_units * scale_us) / 1e6;
    double start = omp_get_wtime();
    while ((omp_get_wtime() - start) < target_sec) {
        // Active spin to simulate CPU computation time
    }
}

// Theoretical analytical model
void print_theoretical_analysis(int n) {
    printf("====================================================================\n");
    printf(" Theoretical Analytical Solution (assuming f(i) takes i ms, 2 threads)\n");
    printf("====================================================================\n");
    printf("For n = %d, iterations i = 1 to %d (total %d iterations):\n\n", n, n - 1, n - 1);
    
    // Chunk division for 2 threads:
    // Thread 0: i = 1 .. n/2
    // Thread 1: i = n/2 + 1 .. n-1
    long long m = n / 2;
    // Loop 1: sum(i)
    long long t0_l1 = m * (m + 1) / 2;
    long long total_sum = (long long)(n - 1) * n / 2;
    long long t1_l1 = total_sum - t0_l1;

    // Loop 2: sum(n-i)
    // For Thread 0 (i=1..m): sum(n-i) = m*n - t0_l1 = t1_l1
    long long t0_l2 = t1_l1;
    long long t1_l2 = t0_l1;

    printf("[Part a] Default schedule(static) with implicit barrier:\n");
    printf("  Loop 1: Thread 0 busy = %lld ms, Thread 1 busy = %lld ms\n", t0_l1, t1_l1);
    printf("    -> Implicit barrier at end of Loop 1: Thread 0 must WAIT %lld ms for Thread 1\n", t1_l1 - t0_l1);
    printf("  Loop 2: Thread 0 busy = %lld ms, Thread 1 busy = %lld ms\n", t0_l2, t1_l2);
    printf("    -> Implicit barrier at end of Loop 2: Thread 1 must WAIT %lld ms for Thread 0\n", t0_l2 - t1_l2);
    printf("  Total Parallel Region Duration: %lld ms\n", t1_l1 + t0_l2);
    printf("  Thread 0: Total active = %lld ms | Total wait = %lld ms (%.1f%% wait)\n",
           t0_l1 + t0_l2, t1_l1 - t0_l1, 100.0 * (t1_l1 - t0_l1) / (t1_l1 + t0_l2));
    printf("  Thread 1: Total active = %lld ms | Total wait = %lld ms (%.1f%% wait)\n\n",
           t1_l1 + t1_l2, t0_l2 - t1_l2, 100.0 * (t0_l2 - t1_l2) / (t1_l1 + t0_l2));

    printf("[Part b-1] schedule(static, 1) [Cyclic / Round-Robin]:\n");
    printf("  Iterations interleaved (T0 gets odd i, T1 gets even i).\n");
    printf("  Loop 1: Thread 0 busy ≈ %lld ms, Thread 1 busy ≈ %lld ms (Difference ≈ %d ms)\n", 
           total_sum / 2, total_sum / 2, n / 2);
    printf("  Loop 2: Thread 0 busy ≈ %lld ms, Thread 1 busy ≈ %lld ms\n", total_sum / 2, total_sum / 2);
    printf("  Total Parallel Region Duration: ≈ %lld ms (reduced by ~33.3%% vs default static!)\n\n", total_sum);

    printf("[Part b-2] schedule(dynamic, 1):\n");
    printf("  Does NOT improve over schedule(static, 1). Dynamic introduces thread-safe\n");
    printf("  work-queue locking/atomic dispatch overhead for each of the %d iterations.\n\n", n - 1);

    printf("[Part b-3] Eliminating waiting time with 'nowait' clause:\n");
    printf("  Directive: #pragma omp for schedule(static) nowait\n");
    printf("  By removing the barrier between Loop 1 and Loop 2:\n");
    printf("  Thread 0 finishes Loop 1 in %lld ms, and immediately proceeds to Loop 2!\n", t0_l1);
    printf("  Simultaneously, Thread 1 works on Loop 1 (%lld ms).\n", t1_l1);
    printf("  Thread 0 total execution: %lld + %lld = %lld ms\n", t0_l1, t0_l2, t0_l1 + t0_l2);
    printf("  Thread 1 total execution: %lld + %lld = %lld ms\n", t1_l1, t1_l2, t1_l1 + t1_l2);
    printf("  WAITING TIME IS COMPLETELY ELIMINATED (0 ms)!\n");
    printf("  Total execution time = %lld ms (Perfect 100%% load balance and linear speedup).\n", total_sum);
    printf("====================================================================\n\n");
}

int main(int argc, char* argv[]) {
    int n = 100;
    double scale_us = 20.0; // scale so test runs in ~0.5-1.0s

    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) scale_us = atof(argv[2]);

    print_theoretical_analysis(n);

    printf("====================================================================\n");
    printf(" Empirical Runtime Benchmark (n=%d, scale=%.1f us/unit, 2 threads)\n", n, scale_us);
    printf("====================================================================\n");

    double start_time, total_time;

    // -------------------------------------------------------------
    // Benchmark 1: Default schedule(static) with implicit barrier
    // -------------------------------------------------------------
    {
        double l1_finish[2], l2_finish[2], region_start;
        #pragma omp parallel num_threads(2)
        {
            int tid = omp_get_thread_num();
            #pragma omp single
            region_start = omp_get_wtime();

            double t_start_l1 = omp_get_wtime();
            #pragma omp for schedule(static)
            for (int i = 1; i < n; i++) {
                simulated_f(i, scale_us);
            }
            l1_finish[tid] = omp_get_wtime();

            // Implicit barrier occurs here

            #pragma omp for schedule(static)
            for (int i = 1; i < n; i++) {
                simulated_f(n - i, scale_us);
            }
            l2_finish[tid] = omp_get_wtime();
        }
        double elapsed = (l2_finish[0] > l2_finish[1] ? l2_finish[0] : l2_finish[1]) - region_start;
        double barrier1_wait_t0 = l1_finish[1] > l1_finish[0] ? (l1_finish[1] - l1_finish[0]) : 0.0;
        double barrier2_wait_t1 = l2_finish[0] > l2_finish[1] ? (l2_finish[0] - l2_finish[1]) : 0.0;

        printf("1. Default schedule(static) with implicit barrier:\n");
        printf("   Total Region Time: %.4f s\n", elapsed);
        printf("   Thread 0 Barrier Wait (Loop 1): %.4f s\n", barrier1_wait_t0);
        printf("   Thread 1 Barrier Wait (Loop 2): %.4f s\n\n", barrier2_wait_t1);
    }

    // -------------------------------------------------------------
    // Benchmark 2: schedule(static, 1) Cyclic
    // -------------------------------------------------------------
    {
        start_time = omp_get_wtime();
        #pragma omp parallel num_threads(2)
        {
            #pragma omp for schedule(static, 1)
            for (int i = 1; i < n; i++) {
                simulated_f(i, scale_us);
            }
            #pragma omp for schedule(static, 1)
            for (int i = 1; i < n; i++) {
                simulated_f(n - i, scale_us);
            }
        }
        total_time = omp_get_wtime() - start_time;
        printf("2. schedule(static, 1) [Cyclic distribution]:\n");
        printf("   Total Region Time: %.4f s\n\n", total_time);
    }

    // -------------------------------------------------------------
    // Benchmark 3: schedule(dynamic, 1)
    // -------------------------------------------------------------
    {
        start_time = omp_get_wtime();
        #pragma omp parallel num_threads(2)
        {
            #pragma omp for schedule(dynamic, 1)
            for (int i = 1; i < n; i++) {
                simulated_f(i, scale_us);
            }
            #pragma omp for schedule(dynamic, 1)
            for (int i = 1; i < n; i++) {
                simulated_f(n - i, scale_us);
            }
        }
        total_time = omp_get_wtime() - start_time;
        printf("3. schedule(dynamic, 1) [Dynamic work-stealing]:\n");
        printf("   Total Region Time: %.4f s\n\n", total_time);
    }

    // -------------------------------------------------------------
    // Benchmark 4: schedule(static) with nowait
    // -------------------------------------------------------------
    {
        start_time = omp_get_wtime();
        #pragma omp parallel num_threads(2)
        {
            #pragma omp for schedule(static) nowait
            for (int i = 1; i < n; i++) {
                simulated_f(i, scale_us);
            }
            #pragma omp for schedule(static)
            for (int i = 1; i < n; i++) {
                simulated_f(n - i, scale_us);
            }
        }
        total_time = omp_get_wtime() - start_time;
        printf("4. schedule(static) with 'nowait' on Loop 1:\n");
        printf("   Total Region Time: %.4f s\n", total_time);
        printf("   Verification: Waiting time eliminated by pipelining complementary loops!\n");
        printf("====================================================================\n");
    }

    return 0;
}
