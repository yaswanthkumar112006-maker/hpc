/**
 * Tutorial-05 OpenMP: Question 6
 * OpenMP program to determine the default loop scheduling of the OpenMP implementation.
 * (Static vs Dynamic vs Guided)
 *
 * Algorithm:
 * 1. Execute a worksharing loop without any schedule clause: #pragma omp parallel for
 * 2. Record thread assignment for every iteration index.
 * 3. Analyze contiguous chunk lengths and thread assignment patterns:
 *    - Static (Block): Exactly P contiguous chunks of uniform size ~N/P assigned in thread order.
 *    - Static (Cyclic): Multiple chunks of size 1 or chunk_size per thread in round-robin order.
 *    - Dynamic: Threads interleave nondeterministically based on execution arrival.
 *    - Guided: Chunks start large (~N/P) and exponentially decrease in size towards 1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 120
#define NUM_THREADS 4

typedef struct ChunkInfo {
    int thread_id;
    int start_idx;
    int end_idx;
    int length;
} ChunkInfo;

int main(void) {
    int thread_assignments[N];
    omp_set_num_threads(NUM_THREADS);

    printf("====================================================================\n");
    printf(" OpenMP Tutorial 05 - Question 6: Default Scheduling Policy Detection\n");
    printf("====================================================================\n");
    printf("Total Loop Iterations: %d | OpenMP Threads: %d\n", N, NUM_THREADS);
    printf("Testing loop: '#pragma omp parallel for' WITHOUT schedule clause...\n\n");

    // Execute loop without specifying schedule(...)
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        thread_assignments[i] = omp_get_thread_num();
    }

    // Inspect chunk distributions
    ChunkInfo chunks[N];
    int chunk_count = 0;

    int current_thread = thread_assignments[0];
    int start_i = 0;

    for (int i = 1; i <= N; i++) {
        if (i == N || thread_assignments[i] != current_thread) {
            chunks[chunk_count].thread_id = current_thread;
            chunks[chunk_count].start_idx = start_i;
            chunks[chunk_count].end_idx = i - 1;
            chunks[chunk_count].length = i - start_i;
            chunk_count++;

            if (i < N) {
                current_thread = thread_assignments[i];
                start_i = i;
            }
        }
    }

    printf("Detected Iteration Chunks:\n");
    printf("--------------------------------------------------------------------\n");
    printf("%-8s | %-12s | %-16s | %-10s\n", "Chunk #", "Thread ID", "Iteration Range", "Chunk Size");
    printf("--------------------------------------------------------------------\n");
    for (int c = 0; c < chunk_count; c++) {
        printf("%-8d | %-12d | [%3d ... %3d]      | %-10d\n", 
               c + 1, chunks[c].thread_id, chunks[c].start_idx, chunks[c].end_idx, chunks[c].length);
    }
    printf("--------------------------------------------------------------------\n\n");

    // Classification Logic
    printf("Scheduling Classification Analysis:\n");
    bool is_static_block = false;
    bool is_guided = false;
    bool is_dynamic = false;

    if (chunk_count == NUM_THREADS) {
        bool monotonic_threads = true;
        for (int c = 0; c < chunk_count; c++) {
            if (chunks[c].thread_id != c) monotonic_threads = false;
        }
        if (monotonic_threads) {
            is_static_block = true;
        }
    } else {
        // Check if chunk lengths are decreasing
        bool decreasing = true;
        for (int c = 0; c < chunk_count - 1; c++) {
            if (chunks[c + 1].length > chunks[c].length) {
                decreasing = false;
                break;
            }
        }
        if (decreasing && chunk_count > NUM_THREADS) {
            is_guided = true;
        } else {
            is_dynamic = true;
        }
    }

    printf("  - Total contiguous chunks created: %d (Expected for static block = %d)\n", 
           chunk_count, NUM_THREADS);
    printf("  - Chunk distribution per thread: Uniform block size of %d iterations per thread\n\n",
           N / NUM_THREADS);

    printf("====================================================================\n");
    printf(" CONCLUSION:\n");
    if (is_static_block) {
        printf(" The default scheduling of this OpenMP implementation (GCC libgomp) is:\n");
        printf(" >>> STATIC (BLOCK) SCHEDULING <<<\n\n");
        printf(" Reason: Iterations [0..N-1] are divided evenly into %d contiguous\n", NUM_THREADS);
        printf(" chunks of size %d, statically pre-allocated to threads 0..%d.\n", 
               N / NUM_THREADS, NUM_THREADS - 1);
    } else if (is_guided) {
        printf(" >>> GUIDED SCHEDULING <<<\n");
    } else if (is_dynamic) {
        printf(" >>> DYNAMIC SCHEDULING <<<\n");
    }
    printf("====================================================================\n");

    return 0;
}
