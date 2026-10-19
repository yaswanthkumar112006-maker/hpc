/**
 * Tutorial-CUDA: Task 5 (Problem 2i) - 2D Element-Wise Matrix Addition
 * 
 * Performs element-wise addition of two matrices A and B (C = A + B)
 * using a 2D grid-block thread mapping where each thread computes
 * one element of the resulting matrix C.
 *
 * Mapping Formula:
 * - col = blockIdx.x * blockDim.x + threadIdx.x
 * - row = blockIdx.y * blockDim.y + threadIdx.y
 * - index = row * width + col
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>

#define DEFAULT_WIDTH 1024
#define DEFAULT_HEIGHT 1024

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
// CUDA Kernel: 2D Matrix Addition (C = A + B)
// -------------------------------------------------------------
__global__ void matrixAdd2DKernel(const float* __restrict__ A, 
                                  const float* __restrict__ B, 
                                  float* __restrict__ C, 
                                  int width, int height) {
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    if (row < height && col < width) {
        int idx = row * width + col;
        C[idx] = A[idx] + B[idx];
    }
}

// CPU baseline function for numerical verification
void matrixAddCPU(const float* A, const float* B, float* C, int width, int height) {
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            int idx = r * width + c;
            C[idx] = A[idx] + B[idx];
        }
    }
}

int main(int argc, char** argv) {
    int width = DEFAULT_WIDTH;
    int height = DEFAULT_HEIGHT;

    if (argc > 1) width = atoi(argv[1]);
    if (argc > 2) height = atoi(argv[2]);

    int total_elements = width * height;
    size_t bytes = total_elements * sizeof(float);

    printf("====================================================================\n");
    printf(" CUDA Tutorial - Task 5 (Problem 2i): 2D Matrix Addition (C = A + B)\n");
    printf("====================================================================\n\n");

    printf("Matrix Configuration:\n");
    printf("  - Dimensions:             %d rows x %d columns\n", height, width);
    printf("  - Total Elements:         %d floating-point elements\n", total_elements);
    printf("  - Memory per Matrix:      %.2f MB (Total 3 matrices = %.2f MB)\n", 
           (float)bytes / (1024.0f * 1024.0f), 3.0f * (float)bytes / (1024.0f * 1024.0f));

    // 2D Block and Grid Launch Configuration
    dim3 block(16, 16); // 256 threads per block
    dim3 grid((width + block.x - 1) / block.x, 
              (height + block.y - 1) / block.y);

    printf("\nCUDA Launch Geometry:\n");
    printf("  - Block Dimension:        dim3 block(%d, %d) -> %d threads/block\n", 
           block.x, block.y, block.x * block.y);
    printf("  - Grid Dimension:         dim3 grid(%d, %d)  -> %d blocks\n", 
           grid.x, grid.y, grid.x * grid.y);
    printf("  - Total Threads Launched: %d threads\n\n", 
           grid.x * grid.y * block.x * block.y);

    // Host memory allocation
    float* h_A = (float*)malloc(bytes);
    float* h_B = (float*)malloc(bytes);
    float* h_C_gpu = (float*)malloc(bytes);
    float* h_C_cpu = (float*)malloc(bytes);

    // Initialize matrices with test values
    for (int i = 0; i < total_elements; i++) {
        h_A[i] = (float)(i % 100) * 0.5f;
        h_B[i] = (float)((total_elements - i) % 100) * 0.25f;
    }

    // 1. CPU Sequential Baseline Run
    printf("Executing CPU baseline sequential matrix addition... ");
    matrixAddCPU(h_A, h_B, h_C_cpu, width, height);
    printf("Done.\n");

    // 2. Device Memory Allocation
    float *d_A = NULL, *d_B = NULL, *d_C = NULL;
    CHECK_CUDA_ERROR(cudaMalloc((void**)&d_A, bytes));
    CHECK_CUDA_ERROR(cudaMalloc((void**)&d_B, bytes));
    CHECK_CUDA_ERROR(cudaMalloc((void**)&d_C, bytes));

    // Copy matrices from Host to Device
    CHECK_CUDA_ERROR(cudaMemcpy(d_A, h_A, bytes, cudaMemcpyHostToDevice));
    CHECK_CUDA_ERROR(cudaMemcpy(d_B, h_B, bytes, cudaMemcpyHostToDevice));

    // Timing with CUDA events
    cudaEvent_t start_event, stop_event;
    CHECK_CUDA_ERROR(cudaEventCreate(&start_event));
    CHECK_CUDA_ERROR(cudaEventCreate(&stop_event));

    // Launch CUDA Kernel
    CHECK_CUDA_ERROR(cudaEventRecord(start_event, 0));
    matrixAdd2DKernel<<<grid, block>>>(d_A, d_B, d_C, width, height);
    CHECK_CUDA_ERROR(cudaGetLastError());
    CHECK_CUDA_ERROR(cudaEventRecord(stop_event, 0));
    CHECK_CUDA_ERROR(cudaEventSynchronize(stop_event));

    float kernel_time_ms = 0.0f;
    CHECK_CUDA_ERROR(cudaEventElapsedTime(&kernel_time_ms, start_event, stop_event));

    // Copy result back from Device to Host
    CHECK_CUDA_ERROR(cudaMemcpy(h_C_gpu, d_C, bytes, cudaMemcpyDeviceToHost));

    // Numerical Verification
    float max_error = 0.0f;
    int error_count = 0;
    for (int i = 0; i < total_elements; i++) {
        float diff = fabsf(h_C_gpu[i] - h_C_cpu[i]);
        if (diff > max_error) max_error = diff;
        if (diff > 1e-4f) error_count++;
    }

    // Effective Bandwidth Calculation: 2 reads (A, B) + 1 write (C)
    double effective_bytes = 3.0 * (double)bytes;
    double effective_bandwidth_GBs = (effective_bytes / (kernel_time_ms * 1e-3)) / 1e9;

    printf("\nExecution Results & Verification:\n");
    printf("--------------------------------------------------------------------\n");
    printf("  - GPU Kernel Execution Time:   %.4f ms (%.6f s)\n", 
           kernel_time_ms, kernel_time_ms / 1000.0f);
    printf("  - Effective Memory Bandwidth:  %.2f GB/s\n", effective_bandwidth_GBs);
    printf("  - Maximum Numerical Error:     %e\n", max_error);
    printf("  - Verification Status:         %s (Errors: %d / %d)\n", 
           (error_count == 0) ? "PASSED (100% Numerical Match with CPU)" : "FAILED", 
           error_count, total_elements);
    printf("  - Sample Values [Index (row 0, col 0)]:\n");
    printf("      A = %.2f, B = %.2f  -->  C_CPU = %.2f, C_GPU = %.2f\n",
           h_A[0], h_B[0], h_C_cpu[0], h_C_gpu[0]);
    printf("--------------------------------------------------------------------\n\n");

    // Clean up
    CHECK_CUDA_ERROR(cudaEventDestroy(start_event));
    CHECK_CUDA_ERROR(cudaEventDestroy(stop_event));
    CHECK_CUDA_ERROR(cudaFree(d_A));
    CHECK_CUDA_ERROR(cudaFree(d_B));
    CHECK_CUDA_ERROR(cudaFree(d_C));
    free(h_A);
    free(h_B);
    free(h_C_gpu);
    free(h_C_cpu);

    printf("Execution completed successfully.\n");
    printf("====================================================================\n");
    return 0;
}
