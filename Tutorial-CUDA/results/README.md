# Execution Results Directory

This directory is designated for **genuine execution outputs** obtained from running the CUDA programs on an NVIDIA GPU environment (e.g., Google Colab with NVIDIA Tesla T4/A100 or a dedicated CUDA workstation).

---

## Status & Environment Notice

- **Host Machine Hardware:** Intel(R) Arc(TM) Graphics (Integrated/Discrete Intel GPU).
- **CUDA Availability on Host:** `nvcc` and `nvidia-smi` are not installed locally because CUDA is proprietary to NVIDIA hardware.
- **Integrity Rule:** In accordance with the academic requirements and project guidelines, **no fake, synthetic, or fabricated GPU output files are committed**. Programs that have been authored and syntax-checked are distinguished from runs executed on physical hardware.

---

## Running on Google Colab to Populate Results

To generate genuine execution logs:
1. Open the runner notebook: [`../colab/CUDA_Tutorial_Colab_Runner.ipynb`](../colab/CUDA_Tutorial_Colab_Runner.ipynb) in [Google Colab](https://colab.research.google.com/).
2. Select **Runtime** > **Change runtime type** > **T4 GPU**.
3. Run all cells sequentially. The notebook executes each binary and pipes output directly to files in `results/`:
   - `01_device_query_output.txt`
   - `02_threads_and_blocks_output.txt`
   - `03_2d_global_indexing_output.txt`
   - `04_image_brightness_output.txt`
   - `05_matrix_addition_output.txt`
4. The notebook automatically bundles all generated results into `cuda_gpu_results.zip` for instant download back into this directory.
