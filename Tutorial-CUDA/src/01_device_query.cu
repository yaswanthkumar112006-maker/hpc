/**
 * Tutorial-CUDA: Task 1 - Device Properties Query
 * 
 * Queries and displays comprehensive hardware and architectural properties
 * for all available CUDA-capable devices on the system using cudaGetDeviceProperties.
 *
 * Properties queried:
 * a. Device name
 * b. Total global memory
 * c. Shared memory per block
 * d. Number of multiprocessors (SMs)
 * e. Maximum threads per block
 * f. Maximum grid size (x, y, z)
 * g. Compute capability (Major.Minor)
 * h. Warp size
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

int main(int argc, char **argv) {
    printf("====================================================================\n");
    printf(" CUDA Tutorial - Task 1: CUDA Device Query & Capabilities\n");
    printf("====================================================================\n\n");

    int deviceCount = 0;
    cudaError_t error_id = cudaGetDeviceCount(&deviceCount);

    if (error_id != cudaSuccess) {
        printf("cudaGetDeviceCount returned %d -> %s\n", 
               (int)error_id, cudaGetErrorString(error_id));
        printf("Result: NO CUDA-capable devices detected or NVIDIA drivers not installed.\n");
        return 1;
    }

    if (deviceCount == 0) {
        printf("There are no available CUDA-capable devices detected on this host.\n");
        return 0;
    }

    printf("Detected %d CUDA-capable device(s) on this system.\n\n", deviceCount);

    for (int dev = 0; dev < deviceCount; ++dev) {
        cudaSetDevice(dev);
        cudaDeviceProp deviceProp;
        CHECK_CUDA_ERROR(cudaGetDeviceProperties(&deviceProp, dev));

        printf("--------------------------------------------------------------------\n");
        printf(" Device %d: \"%s\"\n", dev, deviceProp.name);
        printf("--------------------------------------------------------------------\n");

        // g. Compute Capability
        printf("  g. Compute Capability:                     %d.%d\n", 
               deviceProp.major, deviceProp.minor);

        // b. Total Global Memory
        printf("  b. Total Global Memory:                    %.2f GB (%llu bytes)\n",
               (float)deviceProp.totalGlobalMem / (1024.0f * 1024.0f * 1024.0f),
               (unsigned long long)deviceProp.totalGlobalMem);

        // c. Shared Memory per Block
        printf("  c. Shared Memory per Block:                %.2f KB (%zu bytes)\n",
               (float)deviceProp.sharedMemPerBlock / 1024.0f,
               deviceProp.sharedMemPerBlock);

        // d. Number of Multiprocessors (SMs)
        printf("  d. Multiprocessor (SM) Count:              %d\n",
               deviceProp.multiProcessorCount);

        // e. Maximum Threads per Block
        printf("  e. Maximum Threads per Block:              %d\n",
               deviceProp.maxThreadsPerBlock);

        // Maximum Block Dimensions
        printf("     Maximum Block Dimensions (x, y, z):     [%d, %d, %d]\n",
               deviceProp.maxThreadsDim[0], deviceProp.maxThreadsDim[1], deviceProp.maxThreadsDim[2]);

        // f. Maximum Grid Dimensions
        printf("  f. Maximum Grid Dimensions (x, y, z):      [%d, %d, %d]\n",
               deviceProp.maxGridSize[0], deviceProp.maxGridSize[1], deviceProp.maxGridSize[2]);

        // h. Warp Size
        printf("  h. Warp Size:                              %d threads\n",
               deviceProp.warpSize);

        // Additional valuable architectural details (CUDA 12/13+ compliant)
        printf("  - Total Constant Memory:                   %zu bytes\n", deviceProp.totalConstMem);
        printf("  - L2 Cache Size:                           %d KB\n", deviceProp.l2CacheSize / 1024);
        printf("  - Memory Bus Width:                        %d-bit\n", deviceProp.memoryBusWidth);
        printf("  - Maximum Memory Pitch:                    %zu bytes\n", deviceProp.memPitch);
        printf("  - Concurrent Copy and Execution:           %s (Async Engines: %d)\n", 
               deviceProp.asyncEngineCount > 0 ? "Yes" : "No", deviceProp.asyncEngineCount);
        printf("  - Integrated / Discrete GPU:               %s\n", deviceProp.integrated ? "Integrated" : "Discrete");
        printf("--------------------------------------------------------------------\n\n");
    }

    printf("Device query completed successfully.\n");
    printf("====================================================================\n");
    return 0;
}
