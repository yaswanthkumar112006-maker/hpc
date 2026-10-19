/**
 * Tutorial-CUDA: Task 3 (Problem 2a) - 2D Grid-Block Thread Mapping
 * 
 * Maps each thread in a 2D grid-block configuration to one element of a 2D matrix
 * and prints its global position (row, column) along with its block and thread indices.
 *
 * Mathematical formulation (Emory CS 355 standard):
 * - Column Index: col = blockIdx.x * blockDim.x + threadIdx.x
 * - Row Index:    row = blockIdx.y * blockDim.y + threadIdx.y
 * - Linear Index: linear_idx = row * width + col
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
// 2D Matrix Index Mapping Kernel
// -------------------------------------------------------------
__global__ void map2DGridBlockKernel(int* d_matrix, int width, int height) {
    // Compute 2D coordinate for current thread
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    // Boundary check to guard against out-of-bounds access
    if (row < height && col < width) {
        int linear_idx = row * width + col;
        
        // Store calculated linear index into matrix for verification
        d_matrix[linear_idx] = linear_idx;

        printf("  Thread: (%d, %d) in Block: (%d, %d)  ==>  Matrix Element: [Row %d, Col %d]  |  Linear 1D Index: %2d\n",
               threadIdx.x, threadIdx.y, 
               blockIdx.x, blockIdx.y, 
               row, col, linear_idx);
    }
}

int main(int argc, char** argv) {
    printf("====================================================================\n");
    printf(" CUDA Tutorial - Task 3 (Problem 2a): 2D Grid-Block Matrix Mapping\n");
    printf(" Reference: Emory CS 355 (2D Grids and Thread Hierarchies)\n");
    printf("====================================================================\n\n");

    // Matrix dimensions (6 rows x 8 columns = 48 elements)
    int width = 8;
    int height = 6;
    int total_elements = width * height;
    size_t bytes = total_elements * sizeof(int);

    // 2D Block dimension: 4 threads in x, 2 threads in y = 8 threads per block
    dim3 block(4, 2);

    // 2D Grid dimension: calculate blocks needed in x and y (ceiling division)
    dim3 grid((width + block.x - 1) / block.x, 
              (height + block.y - 1) / block.y);

    printf("Matrix Configuration: %d rows x %d columns (%d total elements)\n", 
           height, width, total_elements);
    printf("Block Dimension:      dim3 block(%d, %d)  --> %d threads per block\n", 
           block.x, block.y, block.x * block.y);
    printf("Grid Dimension:       dim3 grid(%d, %d)   --> %d total blocks\n", 
           grid.x, grid.y, grid.x * grid.y);
    printf("Total GPU Threads:    %d threads allocated\n\n", 
           grid.x * grid.y * block.x * block.y);

    int* h_matrix = (int*)malloc(bytes);
    int* d_matrix = NULL;
    CHECK_CUDA_ERROR(cudaMalloc((void**)&d_matrix, bytes));

    printf("Executing 2D Mapping Kernel and printing thread-to-matrix assignments:\n");
    printf("--------------------------------------------------------------------------------\n");
    map2DGridBlockKernel<<<grid, block>>>(d_matrix, width, height);
    CHECK_CUDA_ERROR(cudaGetLastError());
    CHECK_CUDA_ERROR(cudaDeviceSynchronize());
    printf("--------------------------------------------------------------------------------\n\n");

    // Copy result back to host and verify
    CHECK_CUDA_ERROR(cudaMemcpy(h_matrix, d_matrix, bytes, cudaMemcpyDeviceToHost));

    bool verified = true;
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            int expected_idx = r * width + c;
            if (h_matrix[expected_idx] != expected_idx) {
                verified = false;
            }
        }
    }

    printf("Host Verification of 2D Matrix Mappings:\n");
    printf("  Verification Status: %s\n", verified ? "PASSED (100% of matrix elements uniquely mapped)" : "FAILED");
    
    // Print 2D representation of the mapped matrix
    printf("\nVisual 2D Matrix Array on Host:\n");
    for (int r = 0; r < height; r++) {
        printf("  Row %d: [ ", r);
        for (int c = 0; c < width; c++) {
            printf("%2d ", h_matrix[r * width + c]);
        }
        printf("]\n");
    }

    CHECK_CUDA_ERROR(cudaFree(d_matrix));
    free(h_matrix);

    printf("\nExecution completed successfully.\n");
    printf("====================================================================\n");
    return 0;
}
