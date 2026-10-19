# CUDA Tutorial: High-Performance Computing with NVIDIA CUDA

This directory contains the complete implementation, documentation, runner notebook, and evaluation materials for the **CUDA Tutorial** assignment within the `hpc` project.

---

## 1. Project Directory Structure

```text
Tutorial-CUDA/
├── colab/
│   └── CUDA_Tutorial_Colab_Runner.ipynb  # Interactive Colab notebook for GPU compilation & execution
├── report/
│   ├── REPORT.md                         # Detailed academic report (formulas, design, analysis)
│   └── CUDA_Tutorial_Report.pdf          # Polished, submission-ready PDF report
├── results/
│   ├── README.md                         # Details on execution logs and integrity notes
│   └── ...                               # Populated when executed on NVIDIA GPU
├── screenshots/
│   └── README.md                         # Details on genuine screenshots from Colab runs
├── src/
│   ├── 01_device_query.cu                # Task 1: Query CUDA device architecture & properties
│   ├── 02_threads_and_blocks.cu          # Task 2: Threads, blocks, and built-in variables
│   ├── 03_2d_global_indexing.cu          # Task 3: 2D grid-block coordinate mapping (row, col)
│   ├── 04_image_brightness.cu            # Task 4: 512x512 image brightness adjustment (16x16, 32x32)
│   └── 05_matrix_addition.cu             # Task 5: 2D element-wise matrix addition (C = A + B)
└── README.md                             # This documentation file
```

---

## 2. Hardware Context & Execution Strategy

### Host Machine Specifications
- **GPU:** Intel(R) Arc(TM) Graphics (Intel Xe Architecture).
- **Limitation:** The host laptop does not have an NVIDIA GPU. NVIDIA's CUDA compiler (`nvcc`) and runtime are proprietary to NVIDIA hardware and cannot execute natively on Intel GPUs.
- **Academic Integrity Policy:** In accordance with the project guidelines, **no fake or synthetic GPU outputs are generated locally**. Programs that are written and checked locally are kept distinct from actual GPU executions.

### Solution: Google Colab Remote GPU Execution
To obtain **genuine, authentic CUDA execution logs and metrics**, an automated Google Colab notebook is provided:
[`colab/CUDA_Tutorial_Colab_Runner.ipynb`](colab/CUDA_Tutorial_Colab_Runner.ipynb).
Google Colab provides free access to NVIDIA GPUs (e.g., Tesla T4, A100, or L4), allowing the exact `.cu` source code in this repository to compile with `nvcc` and execute on real NVIDIA CUDA cores.

---

## 3. Tutorial Tasks & Source Implementations

### Task 1: Query CUDA Device Properties (`src/01_device_query.cu`)
- **Objective:** Query and display comprehensive architectural properties of the target GPU using `cudaGetDeviceProperties`.
- **Properties Queried:**
  1. **Device name** (`prop.name`)
  2. **Total global memory** (`prop.totalGlobalMem` in GB and bytes)
  3. **Shared memory per block** (`prop.sharedMemPerBlock` in KB)
  4. **Number of Streaming Multiprocessors (SMs)** (`prop.multiProcessorCount`)
  5. **Maximum threads per block** (`prop.maxThreadsPerBlock`)
  6. **Maximum grid dimensions** (`prop.maxGridSize[0, 1, 2]`)
  7. **Compute capability** (`prop.major.prop.minor`)
  8. **Warp size** (`prop.warpSize`)
  9. Additional metrics: L2 cache size, memory bus width, async copy engines, constant memory.

### Task 2: Exploring Threads, Blocks, and Built-In Variables (`src/02_threads_and_blocks.cu`)
- **Objective:** Demonstrate the hierarchical organization of CUDA execution: Grids $\to$ Blocks $\to$ Warps $\to$ Threads.
- **Built-In Variables Demonstrated:**
  - `threadIdx.x`, `threadIdx.y`: Local thread index within a block.
  - `blockIdx.x`, `blockIdx.y`: Block index within the grid.
  - `blockDim.x`, `blockDim.y`: Block dimensions (threads per block).
  - `gridDim.x`, `gridDim.y`: Grid dimensions (blocks per grid).
- **Sub-Demonstrations:**
  - **Part A (1D Hierarchy):** Launching $3$ blocks of $4$ threads ($12$ total threads) to show linear indexing $\text{global\_id} = \text{blockIdx.x} \times \text{blockDim.x} + \text{threadIdx.x}$.
  - **Part B (2D Hierarchy):** Launching a $2 \times 2$ grid of $2 \times 2$ blocks ($16$ threads) mapping onto a $4 \times 4$ matrix, with local rank and warp ID calculations.

### Task 3: 2D Grid-Block Thread Mapping (`src/03_2d_global_indexing.cu`)
- **Objective:** Map each thread in a 2D grid-block structure to a specific element of a 2D matrix $(row, col)$ and print its global coordinates, block indices, and thread indices.
- **Reference Standard:** Emory University CS 355 curriculum.
- **Mapping Formula:**
  $$\text{col} = \text{blockIdx.x} \times \text{blockDim.x} + \text{threadIdx.x}$$
  $$\text{row} = \text{blockIdx.y} \times \text{blockDim.y} + \text{threadIdx.y}$$
  $$\text{linear\_index} = \text{row} \times \text{width} + \text{col}$$
- **Features:** Boundary check `if (row < height && col < width)` and full host verification table.

### Task 4: 2D Image Brightness Adjustment (`src/04_image_brightness.cu`)
- **Objective:** Process a $512 \times 512$ image ($262,144$ pixels), increasing brightness by $+50$ units per pixel.
- **Assignment Geometry Constraints:**
  - **Image Size:** $512 \times 512$ pixels.
  - **Block Dimension:** `dim3 block(16, 16)` ($256$ threads/block).
  - **Grid Dimension:** `dim3 grid(32, 32)` ($1,024$ blocks).
  - **Total Threads:** $16 \times 16 \times 32 \times 32 = 262,144$ threads (exact 1:1 thread-to-pixel mapping).
- **Features:**
  - Value clamping to range $[0, 255]$: $\min(\max(\text{pixel} + \delta, 0), 255)$.
  - CPU baseline execution with bitwise validation ($100\%$ match).
  - High-precision CUDA event timing (`cudaEventRecord`, `cudaEventElapsedTime`).
  - Throughput computation in MegaPixels/sec.
  - Image export to standard grayscale PGM (`input_512x512.pgm`, `output_brightened_512x512.pgm`).

### Task 5: 2D Element-Wise Matrix Addition (`src/05_matrix_addition.cu`)
- **Objective:** Perform element-wise addition of two 2D matrices $C = A + B$.
- **Mapping:** 2D grid and block configuration with bounds checking.
- **Features:**
  - Default size: $1024 \times 1024$ ($1,048,576$ floating-point elements, $12 \text{ MB}$ total working set).
  - Configurable dimensions via command-line arguments.
  - Sequential CPU baseline validation checking max absolute error ($< 10^{-4}$).
  - Effective memory bandwidth computation:
    $$\text{Effective Bandwidth} = \frac{3 \times N \times \text{sizeof(float)}}{\text{Kernel Time}} \text{ [GB/s]}$$
    (Accounting for 2 reads from $A$ and $B$, and 1 write to $C$).

---

## 4. How to Run on Google Colab (Recommended for Genuine GPU Runs)

Follow these step-by-step instructions to compile and run all programs on a free NVIDIA GPU in Google Colab:

### Step 1: Open Google Colab
1. Open your web browser and go to [https://colab.research.google.com/](https://colab.research.google.com/).
2. Click **Upload** and select `Tutorial-CUDA/colab/CUDA_Tutorial_Colab_Runner.ipynb` from this repository.
   *(Alternatively, open via GitHub tab by pasting the repository URL: `https://github.com/yaswanthkumar112006-maker/hpc.git`)*.

### Step 2: Enable GPU Hardware Accelerator
1. In Colab's menu bar, click **Runtime** > **Change runtime type**.
2. Under **Hardware accelerator**, select **T4 GPU** (Standard free tier).
3. Click **Save**.

### Step 3: Run the Cells
1. Run **Cell 1 & 2** to inspect the allocated GPU (`nvidia-smi`, `nvcc --version`) and clone the repository.
2. Run **Cells 3 through 7** sequentially to compile with `nvcc -O3` and execute Tasks 1 through 5.
   Each cell automatically displays the output in the notebook and writes a log to `results/<task>_output.txt`.
3. Run **Cell 8** to inspect the generated result files, pack them into `cuda_gpu_results.zip`, and automatically trigger a download to your local machine.

### Step 4: Save Genuine Results to Repository
1. Extract the downloaded `cuda_gpu_results.zip` into your local `Tutorial-CUDA/results/` directory.
2. Take screenshots of the Colab execution cells (or `nvidia-smi`) and place them in `Tutorial-CUDA/screenshots/`.

---

## 5. Local Compilation & Execution (For Systems with NVIDIA GPU)

If you are running on an NVIDIA GPU workstation with the CUDA Toolkit installed:

```bash
# 1. Navigate to Tutorial-CUDA directory
cd Tutorial-CUDA

# 2. Create build and results directories
mkdir -p bin results screenshots

# 3. Compile all source files
nvcc -O3 src/01_device_query.cu -o bin/01_device_query
nvcc -O3 src/02_threads_and_blocks.cu -o bin/02_threads_and_blocks
nvcc -O3 src/03_2d_global_indexing.cu -o bin/03_2d_global_indexing
nvcc -O3 src/04_image_brightness.cu -o bin/04_image_brightness
nvcc -O3 src/05_matrix_addition.cu -o bin/05_matrix_addition

# 4. Execute each task and save outputs
./bin/01_device_query | tee results/01_device_query_output.txt
./bin/02_threads_and_blocks | tee results/02_threads_and_blocks_output.txt
./bin/03_2d_global_indexing | tee results/03_2d_global_indexing_output.txt
./bin/04_image_brightness | tee results/04_image_brightness_output.txt
./bin/05_matrix_addition | tee results/05_matrix_addition_output.txt
```

---

## 6. Verification and Error Handling

All source files implement the standard CUDA error checking macro:
```cpp
#define CHECK_CUDA_ERROR(val) check((val), #val, __FILE__, __LINE__)
void check(cudaError_t result, char const *const func, const char *const file, int const line) {
    if (result != cudaSuccess) {
        fprintf(stderr, "CUDA error at %s:%d code=%d(%s) \"%s\" \n",
                file, line, (unsigned int)result, 
                cudaGetErrorString(result), func);
                exit(EXIT_FAILURE);
    }
}
```
Every kernel launch is followed by:
1. `CHECK_CUDA_ERROR(cudaGetLastError());` to catch syntax, invalid configuration, or launch parameter errors.
2. `CHECK_CUDA_ERROR(cudaDeviceSynchronize());` to catch asynchronous runtime errors occurring during kernel execution.
3. Host baseline comparison verifying $100\%$ numerical equivalence with sequential CPU implementations.
