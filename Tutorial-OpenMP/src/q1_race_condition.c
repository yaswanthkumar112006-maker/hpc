/**
 * Tutorial-05 OpenMP: Question 1
 * Demonstrates and explains differences between:
 * 1) Unsynchronized parallel loop with potential data races:
 *    v[indices[i]] += f(i);
 * 2) Critical section synchronization (correct but high lock contention):
 *    #pragma omp critical
 *    v[indices[i]] += f(i);
 * 3) Atomic synchronization (correct, low overhead, fine-grained):
 *    #pragma omp atomic
 *    v[indices[i]] += f(i);
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// Simple function f(i)
static inline double f(int i) {
    return (double)(i % 10 + 1) * 0.5;
}

int main(int argc, char* argv[]) {
    int n = 1000000;  // Number of iterations
    int m = 50;       // Length of array v (forces frequent index collisions)
    int num_threads = 4;

    if (argc > 1) n = atoi(argv[1]);
    if (argc > 2) m = atoi(argv[2]);
    if (argc > 3) num_threads = atoi(argv[3]);

    omp_set_num_threads(num_threads);

    printf("====================================================================\n");
    printf(" OpenMP Tutorial 05 - Question 1: Race Conditions & Synchronization\n");
    printf("====================================================================\n");
    printf("Iterations (n): %d | Array size (m): %d | Threads: %d\n\n", n, m, num_threads);

    // Allocate arrays
    int* indices = (int*)malloc(n * sizeof(int));
    double* v_seq = (double*)calloc(m, sizeof(double));
    double* v_race = (double*)calloc(m, sizeof(double));
    double* v_crit = (double*)calloc(m, sizeof(double));
    double* v_atom = (double*)calloc(m, sizeof(double));

    // Fill indices with non-unique values in [0, m-1] to induce collision
    for (int i = 0; i < n; i++) {
        indices[i] = (i * 17 + 11) % m;
    }

    // -------------------------------------------------------------
    // Baseline: Sequential Execution (Ground Truth)
    // -------------------------------------------------------------
    double t0 = omp_get_wtime();
    for (int i = 0; i < n; i++) {
        v_seq[indices[i]] += f(i);
    }
    double t_seq = omp_get_wtime() - t0;
    double expected_sum = 0.0;
    for (int j = 0; j < m; j++) expected_sum += v_seq[j];

    printf("[1] Sequential Execution (Baseline):\n");
    printf("    Time: %f s | Total Sum: %.2f | Correct: YES\n\n", t_seq, expected_sum);

    // -------------------------------------------------------------
    // Snippet 1: Unsynchronized Parallel Loop
    // -------------------------------------------------------------
    int i;
    t0 = omp_get_wtime();
    #pragma omp parallel for default(none) shared(v_race, indices, n) private(i)
    for (i = 0; i < n; i++) {
        v_race[indices[i]] += f(i);
    }
    double t_race = omp_get_wtime() - t0;
    double race_sum = 0.0;
    for (int j = 0; j < m; j++) race_sum += v_race[j];
    double race_diff = expected_sum - race_sum;

    printf("[2] Snippet 1 (No synchronization):\n");
    printf("    Time: %f s | Total Sum: %.2f | Discrepancy: %.2f\n", t_race, race_sum, race_diff);
    if (race_diff != 0.0) {
        printf("    Result: INCORRECT! Lost updates occurred due to DATA RACE.\n\n");
    } else {
        printf("    Result: MATCHED (coincidentally or n too small).\n\n");
    }

    // -------------------------------------------------------------
    // Snippet 2: Critical Section Synchronization
    // -------------------------------------------------------------
    t0 = omp_get_wtime();
    #pragma omp parallel for default(none) shared(v_crit, indices, n) private(i)
    for (i = 0; i < n; i++) {
        #pragma omp critical
        v_crit[indices[i]] += f(i);
    }
    double t_crit = omp_get_wtime() - t0;
    double crit_sum = 0.0;
    for (int j = 0; j < m; j++) crit_sum += v_crit[j];

    printf("[3] Snippet 2 (#pragma omp critical):\n");
    printf("    Time: %f s | Total Sum: %.2f | Slowdown vs Seq: %.2fx\n", 
           t_crit, crit_sum, t_crit / t_seq);
    printf("    Result: CORRECT, but serialized by global mutual exclusion lock!\n\n");

    // -------------------------------------------------------------
    // Snippet 3: Atomic Synchronization
    // -------------------------------------------------------------
    t0 = omp_get_wtime();
    #pragma omp parallel for default(none) shared(v_atom, indices, n) private(i)
    for (i = 0; i < n; i++) {
        #pragma omp atomic
        v_atom[indices[i]] += f(i);
    }
    double t_atom = omp_get_wtime() - t0;
    double atom_sum = 0.0;
    for (int j = 0; j < m; j++) atom_sum += v_atom[j];

    printf("[4] Snippet 3 (#pragma omp atomic):\n");
    printf("    Time: %f s | Total Sum: %.2f | Speedup vs Critical: %.2fx\n", 
           t_atom, atom_sum, t_crit / t_atom);
    printf("    Result: CORRECT, hardware-level memory atomicity without coarse locking.\n\n");

    printf("--------------------------------------------------------------------\n");
    printf("Summary Comparison:\n");
    printf("  Method          Correct?   Time (s)     Relative Speed\n");
    printf("  Sequential      YES        %8.5f     1.00x\n", t_seq);
    printf("  No Sync (Race)  NO         %8.5f     %.2fx (Unsafe)\n", t_race, t_seq / t_race);
    printf("  Critical        YES        %8.5f     %.2fx (Heavy lock contention)\n", t_crit, t_seq / t_crit);
    printf("  Atomic          YES        %8.5f     %.2fx (Fine-grained atomic)\n", t_atom, t_seq / t_atom);
    printf("====================================================================\n");

    free(indices);
    free(v_seq);
    free(v_race);
    free(v_crit);
    free(v_atom);
    return 0;
}
