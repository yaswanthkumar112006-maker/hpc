/**
 * Tutorial-CUDA: Task 2 (Problem 1) - Exploring Threads and Blocks
 * 
 * Demonstrates how CUDA organizes computation into Grids, Blocks, and Threads,
 * and how each thread uniquely identifies itself using built-in variables:
 * - threadIdx: Index of thread within its block (x, y, z)
 * - blockIdx:  Index of block within the grid (x, y, z)
 * - blockDim:  Dimensions of each block in threads (x, y, z)
 * - gridDim:   Dimensions of the grid in blocks (x, y, z)
 */

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>

#define CHECK_CUDA_ERROR(val) check((val), #val, __FILE__, __LINE__)
void check(cudaError_t result, char const *const func, const char *const file, int const line) {
    if (result != cudaSuccess) {
        fprintf(stderr, "CUDA error at %s:%d code=%d(%s) \"%s\" \n",
                file, line, (unsigned int)result, 
                cudaGetErrorString(result), func);
        exit(EXIT_FAILURE);
    }
}

// -------------------------------------------------------------
// Kernel 1: 1D Thread and Block Organization
// -------------------------------------------------------------
__global__ void print1DHierarchyKernel(int* d_output) {
    // Unique global thread ID in 1D
    int global_id = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Store in output array for host verification
    d_output[global_id] = global_id;

    // Each thread prints its exact coordinates in the hierarchy
    printf("  [1D Kernel] Global ID: %2d | Block ID: %2d (out of %2d) | Thread ID in Block: %2d (out of %2d)\n",
           global_id, blockIdx.x, gridDim.x, threadIdx.x, blockDim.x);
}

// -------------------------------------------------------------
// Kernel 2: 2D Thread and Block Organization
// -------------------------------------------------------------
__global__ void print2DHierarchyKernel(int width, int height) {
    // Global 2D coordinates
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    // Linear global ID
    int global_id = row * width + col;

    // Calculate Warp ID within the block (32 threads per warp)
    int thread_id_in_block = threadIdx.y * blockDim.x + threadIdx.x;
    int warp_id = thread_id_in_block / 32;

    if (row < height && col < width) {
        printf("  [2D Kernel] Pos (Row=%d, Col=%d) -> Global Index: %2d | Block: (%d,%d) | Thread in Block: (%d,%d) | Local Rank: %d | Warp: %d\n",
               row, col, global_id, blockIdx.x, blockIdx.y, threadIdx.x, threadIdx.y, thread_id_in_block, warp_id);
    }
}

int main(int argc, char** argv) {
    printf("====================================================================\n");
    printf(" CUDA Tutorial - Task 2 (Problem 1): Threads and Blocks Exploration\n");
    printf("====================================================================\n\n");

    // -------------------------------------------------------------
    // Part A: 1D Thread Hierarchy Demonstration
    // -------------------------------------------------------------
    int num_blocks_1d = 3;
    int threads_per_block_1d = 4;
    int total_threads_1d = num_blocks_1d * threads_per_block_1d; // 12 threads total

    printf("--- Part A: 1D Grid of 1D Blocks ---\n");
    printf("Configuration: Grid Dimension = %d blocks, Block Dimension = %d threads\n", 
           num_blocks_1d, threads_per_block_1d);
    printf("Total threads executing: %d\n\n", total_threads_1d);

    int* d_output_1d = NULL;
    int* h_output_1d = (int*)malloc(total_threads_1d * sizeof(int));
    CHECK_CUDA_ERROR(cudaMalloc((void**)&d_output_1d, total_threads_1d * sizeof(int)));

    // Launch 1D kernel
    print1DHierarchyKernel<<<num_blocks_1d, threads_per_block_1d>>>(d_output_1d);
    CHECK_CUDA_ERROR(cudaGetLastError());
    CHECK_CUDA_ERROR(cudaDeviceSynchronize());

    // Copy back output
    CHECK_CUDA_ERROR(cudaMemcpy(h_output_1d, d_output_1d, total_threads_1d * sizeof(int), cudaMemcpyDeviceToHost));
    
    // Verify on host
    bool correct_1d = true;
    for (int i = 0; i < total_threads_1d; i++) {
        if (h_output_1d[i] != i) correct_1d = false;
    }
    printf("\nHost Verification (1D): %s\n\n", correct_1d ? "SUCCESS (All IDs match linear sequence)" : "FAILED");

    CHECK_CUDA_ERROR(cudaFree(d_output_1d));
    free(h_output_1d);

    // -------------------------------------------------------------
    // Part B: 2D Grid of 2D Blocks Hierarchy
    // -------------------------------------------------------------
    int matrix_w = 4;
    int matrix_h = 4;

    dim3 block2D(2, 2); // 4 threads per block (2x2)
    dim3 grid2D((matrix_w + block2D.x - 1) / block2D.x, 
                (matrix_h + block2D.y - 1) / block2D.y); // 2x2 = 4 blocks

    printf("--- Part B: 2D Grid of 2D Blocks ---\n");
    printf("Matrix Size: %d x %d (Total %d elements)\n", matrix_w, matrix_h, matrix_w * matrix_h);
    printf("Block Dimensions: dim3 block(%d, %d)\n", block2D.x, block2D.y);
    printf("Grid Dimensions:  dim3 grid(%d, %d)\n", grid2D.x, grid2D.y);
    printf("Total blocks: %d | Total threads: %d\n\n", 
           grid2D.x * grid2D.y, grid2D.x * grid2D.y * block2D.x * block2D.y);

    print2DHierarchyKernel<<<grid2D, block2D>>>(matrix_w, matrix_h);
    CHECK_CUDA_ERROR(cudaGetLastError());
    CHECK_CUDA_ERROR(cudaDeviceSynchronize());

    printf("\nExecution completed successfully.\n");
    printf("====================================================================\n");
    return 0;
}
