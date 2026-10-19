# Screenshots Directory

This directory stores **genuine screenshot captures** from actual runs of the CUDA tutorial programs executed in an authentic NVIDIA GPU environment (such as Google Colab).

---

## Status & Guidelines

- **Integrity Policy:** In strict adherence to the project guidelines, no simulated or fabricated screenshot files are placed here.
- **Pending Actual Colab Run:** Screenshots will be added once the interactive notebook [`../colab/CUDA_Tutorial_Colab_Runner.ipynb`](../colab/CUDA_Tutorial_Colab_Runner.ipynb) is executed under a Google Colab GPU runtime.

---

## Recommended Screenshots to Capture

When running in Google Colab:
1. `00_nvidia_smi_colab.png`: Screenshot showing the output of `!nvidia-smi` confirming an active Tesla T4 or A100 GPU allocated by Colab.
2. `01_device_query_colab.png`: Screenshot of Task 1 displaying the queried GPU properties (Compute capability, SM count, warp size, global memory).
3. `02_threads_blocks_colab.png`: Screenshot of Task 2 showing the thread and block hierarchy outputs.
4. `03_2d_indexing_colab.png`: Screenshot of Task 3 demonstrating the 2D row/column matrix mapping.
5. `04_image_brightness_colab.png`: Screenshot of Task 4 displaying the $512 \times 512$ kernel execution time and verification result.
6. `05_matrix_addition_colab.png`: Screenshot of Task 5 displaying the 2D matrix addition throughput, memory bandwidth (GB/s), and verification status.
