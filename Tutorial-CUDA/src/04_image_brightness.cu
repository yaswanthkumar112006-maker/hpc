/**
 * Tutorial-CUDA: Task 4 (Problem 2b) - 2D Image Brightness Adjustment
 * 
 * Each thread processes one pixel of a 2D image using a 2D grid-block mapping
 * and increases the brightness/color intensity of each pixel by a fixed value.
 *
 * Problem Specification:
 * - Image Dimension: 512 x 512 = 262,144 pixels
 * - Block Configuration: dim3 block(16, 16)  --> 256 threads per block
 * - Grid Configuration:  dim3 grid(32, 32)   --> 1,024 blocks
 * - Total Threads: 256 x 1024 = 262,144 threads (1:1 mapping per pixel)
 * - Clamping: Pixel values clamped to [0, 255]
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>

#define WIDTH 512
#define HEIGHT 512
#define TOTAL_PIXELS (WIDTH * HEIGHT)
#define BRIGHTNESS_DELTA 50

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
// CUDA Kernel: 2D Image Brightness Adjustment
// -------------------------------------------------------------
__global__ void adjustBrightnessKernel(const unsigned char* __restrict__ d_in, 
                                       unsigned char* __restrict__ d_out, 
                                       int width, int height, int delta) {
    // 2D coordinate calculation
    int col = blockIdx.x * blockDim.x + threadIdx.x;
    int row = blockIdx.y * blockDim.y + threadIdx.y;

    if (row < height && col < width) {
        int idx = row * width + col;
        int new_val = (int)d_in[idx] + delta;
        
        // Clamping between 0 and 255
        if (new_val > 255) new_val = 255;
        if (new_val < 0)   new_val = 0;

        d_out[idx] = (unsigned char)new_val;
    }
}

// CPU baseline function for verification and performance comparison
void adjustBrightnessCPU(const unsigned char* h_in, unsigned char* h_out, int width, int height, int delta) {
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            int idx = r * width + c;
            int new_val = (int)h_in[idx] + delta;
            if (new_val > 255) new_val = 255;
            if (new_val < 0)   new_val = 0;
            h_out[idx] = (unsigned char)new_val;
        }
    }
}

// Helper to save image as Portable Graymap (PGM/PPM) format
void savePGM(const char* filename, const unsigned char* image, int width, int height) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) return;
    fprintf(fp, "P5\n%d %d\n255\n", width, height);
    fwrite(image, sizeof(unsigned char), width * height, fp);
    fclose(fp);
}

int main(int argc, char** argv) {
    printf("====================================================================\n");
    printf(" CUDA Tutorial - Task 4 (Problem 2b): Image Brightness Adjustment\n");
    printf("====================================================================\n\n");

    printf("Image Specifications:\n");
    printf("  - Dimensions:             %d x %d pixels (Total: %d pixels)\n", 
           WIDTH, HEIGHT, TOTAL_PIXELS);
    printf("  - Brightness Adjustment:  %+d intensity units\n", BRIGHTNESS_DELTA);

    // Exact block and grid dimensions required by assignment
    dim3 block(16, 16);
    dim3 grid(32, 32);

    printf("\nCUDA Launch Geometry:\n");
    printf("  - Block Dimensions:       dim3 block(%d, %d) -> %d threads/block\n", 
           block.x, block.y, block.x * block.y);
    printf("  - Grid Dimensions:        dim3 grid(%d, %d)  -> %d blocks\n", 
           grid.x, grid.y, grid.x * grid.y);
    printf("  - Total Active Threads:   %d threads (1:1 pixel coverage)\n\n",
           block.x * block.y * grid.x * grid.y);

    size_t img_bytes = TOTAL_PIXELS * sizeof(unsigned char);

    // Host memory allocation
    unsigned char* h_input = (unsigned char*)malloc(img_bytes);
    unsigned char* h_output_gpu = (unsigned char*)malloc(img_bytes);
    unsigned char* h_output_cpu = (unsigned char*)malloc(img_bytes);

    // Generate synthetic gradient and circular image pattern
    for (int r = 0; r < HEIGHT; r++) {
        for (int c = 0; c < WIDTH; c++) {
            int dx = c - WIDTH / 2;
            int dy = r - HEIGHT / 2;
            int dist = (int)sqrtf((float)(dx * dx + dy * dy));
            int val = (r * 128 / HEIGHT) + (dist < 150 ? 80 : 20);
            if (val > 255) val = 255;
            h_input[r * WIDTH + c] = (unsigned char)val;
        }
    }

    // 1. CPU Sequential Processing
    printf("Executing CPU baseline sequential processing... ");
    adjustBrightnessCPU(h_input, h_output_cpu, WIDTH, HEIGHT, BRIGHTNESS_DELTA);
    printf("Done.\n");

    // 2. CUDA Device Memory Allocation
    unsigned char* d_input = NULL;
    unsigned char* d_output = NULL;
    CHECK_CUDA_ERROR(cudaMalloc((void**)&d_input, img_bytes));
    CHECK_CUDA_ERROR(cudaMalloc((void**)&d_output, img_bytes));

    // Copy input to GPU
    CHECK_CUDA_ERROR(cudaMemcpy(d_input, h_input, img_bytes, cudaMemcpyHostToDevice));

    // CUDA Events for high-precision kernel timing
    cudaEvent_t start_event, stop_event;
    CHECK_CUDA_ERROR(cudaEventCreate(&start_event));
    CHECK_CUDA_ERROR(cudaEventCreate(&stop_event));

    // Launch CUDA Kernel
    CHECK_CUDA_ERROR(cudaEventRecord(start_event, 0));
    adjustBrightnessKernel<<<grid, block>>>(d_input, d_output, WIDTH, HEIGHT, BRIGHTNESS_DELTA);
    CHECK_CUDA_ERROR(cudaGetLastError());
    CHECK_CUDA_ERROR(cudaEventRecord(stop_event, 0));
    CHECK_CUDA_ERROR(cudaEventSynchronize(stop_event));

    float kernel_time_ms = 0.0f;
    CHECK_CUDA_ERROR(cudaEventElapsedTime(&kernel_time_ms, start_event, stop_event));

    // Copy result back to host
    CHECK_CUDA_ERROR(cudaMemcpy(h_output_gpu, d_output, img_bytes, cudaMemcpyDeviceToHost));

    // Verification
    int error_count = 0;
    for (int i = 0; i < TOTAL_PIXELS; i++) {
        if (h_output_gpu[i] != h_output_cpu[i]) {
            error_count++;
        }
    }

    printf("\nExecution Results & Verification:\n");
    printf("--------------------------------------------------------------------\n");
    printf("  - GPU Kernel Execution Time:   %.4f ms (%.6f seconds)\n", 
           kernel_time_ms, kernel_time_ms / 1000.0f);
    printf("  - Total Pixel Processing Rate: %.2f MegaPixels/sec\n", 
           ((float)TOTAL_PIXELS / 1e6f) / (kernel_time_ms / 1000.0f));
    printf("  - Verification Status:         %s (Errors: %d / %d)\n", 
           (error_count == 0) ? "PASSED (100% Bitwise Match with CPU)" : "FAILED", 
           error_count, TOTAL_PIXELS);
    printf("  - Sample Pixel Comparison [Pos (256, 256)]:\n");
    printf("      Input: %3d  -->  CPU Output: %3d  |  GPU Output: %3d\n",
           h_input[256 * WIDTH + 256], 
           h_output_cpu[256 * WIDTH + 256], 
           h_output_gpu[256 * WIDTH + 256]);
    printf("--------------------------------------------------------------------\n\n");

    // Save sample output image files
    savePGM("input_512x512.pgm", h_input, WIDTH, HEIGHT);
    savePGM("output_brightened_512x512.pgm", h_output_gpu, WIDTH, HEIGHT);
    printf("Saved sample test images:\n");
    printf("  - input_512x512.pgm\n");
    printf("  - output_brightened_512x512.pgm\n\n");

    // Clean up
    CHECK_CUDA_ERROR(cudaEventDestroy(start_event));
    CHECK_CUDA_ERROR(cudaEventDestroy(stop_event));
    CHECK_CUDA_ERROR(cudaFree(d_input));
    CHECK_CUDA_ERROR(cudaFree(d_output));
    free(h_input);
    free(h_output_gpu);
    free(h_output_cpu);

    printf("Execution completed successfully.\n");
    printf("====================================================================\n");
    return 0;
}
