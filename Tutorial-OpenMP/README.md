# OpenMP Tutorial 05 Assessment

Comprehensive parallel programming implementation and empirical evaluation using OpenMP in C for High-Performance Computing (HPC).

---

## 📁 Repository Structure

```
Tutorial-OpenMP/
├── src/                               # C source code files
│   ├── test_omp.c                     # Verification test for OpenMP setup
│   ├── q1_race_condition.c            # Q1: Race conditions, critical vs atomic
│   ├── q2_scheduling_nowait.c         # Q2: Loop scheduling, barrier wait & nowait
│   ├── q3_monte_carlo_pi.c            # Q3: Monte Carlo Pi estimation & thread scaling
│   ├── q4_binary_tree_sum.c           # Q4: Task-based parallel binary tree sum
│   ├── q5_bfs_dfs.c                   # Q5: Thread-safe parallel BFS & DFS traversals
│   ├── q6_default_schedule.c          # Q6: Detection of default OpenMP loop schedule
│   └── optional_heat_conduction.c     # Optional: 2D steady-state heat diffusion solver
├── bin/                               # Compiled executable binaries
├── results/                           # Raw terminal outputs and benchmark CSV data
│   ├── pi_results.csv                 # Monte Carlo error vs sample size (N=10 to 10^9)
│   ├── pi_scaling.csv                 # Thread scaling data (1, 2, 4, 8, 16 threads)
│   └── q*.output.txt                  # Full captured execution logs
├── graphs/                            # High-resolution generated plots (Matplotlib)
│   ├── points_vs_pi.png               # Sample size vs Estimated Pi
│   ├── points_vs_error.png            # Sample size vs Error (Empirical vs O(1/sqrt(N)))
│   ├── threads_vs_execution_time.png  # Multi-thread execution time
│   ├── threads_vs_speedup.png         # Speedup vs Ideal linear speedup
│   └── threads_vs_efficiency.png      # Parallel efficiency (%)
├── screenshots/                       # High-fidelity terminal output captures
│   ├── q1_race_condition.png
│   ├── q2_scheduling_nowait.png
│   ├── q3_monte_carlo_pi.png
│   ├── q4_binary_tree_sum.png
│   ├── q5_bfs_dfs.png
│   ├── q6_default_schedule.png
│   └── optional_heat_conduction.png
├── build_and_run.ps1                  # Complete automated build and test runner
├── plot_pi.py                         # Graph generation script (Python 3.12)
├── render_screenshots.py              # Terminal screenshot rendering script (Pillow)
├── README.md                          # Build, run, and usage instructions
└── REPORT.md                          # Full academic report and analytical solutions
```

---

## 🛠️ Environment Prerequisites

- **Operating System:** Windows 11 x64
- **C Compiler:** GCC 16.2.0 (MSYS2 UCRT64 toolchain)
  - Full path: `C:\msys64\ucrt64\bin\gcc.exe`
  - OpenMP library: `mingw-w64-ucrt-x86_64-libgomp`
- **Python Runtime:** Python 3.12 (with `matplotlib` and `pillow`)
  - Full path: `C:\Users\admin h\AppData\Local\Programs\Python\Python312\python.exe`

---

## 🚀 Quick Start: Build and Run All

To compile all 7 programs, execute the benchmark suite, record terminal logs, generate all plots, and render screenshots in a single command:

```powershell
# Open PowerShell inside Tutorial-OpenMP directory
cd "c:\Users\admin h\hpc\Tutorial-OpenMP"

# Run automated build and test suite
powershell -ExecutionPolicy Bypass -File .\build_and_run.ps1
```

---

## 🔨 Manual Compilation & Execution Instructions

If compiling programs individually, ensure the MSYS2 UCRT64 `bin` folder is added to your PowerShell session path:

```powershell
$env:PATH = "C:\msys64\ucrt64\bin;" + $env:PATH
```

### Question 1: Race Conditions & Synchronization
```powershell
gcc -O3 -fopenmp -Wall -Wextra src/q1_race_condition.c -o bin/q1_race_condition.exe
.\bin\q1_race_condition.exe [iterations=1000000] [array_size=50] [threads=4]
```

### Question 2: Scheduling, Barriers & Nowait
```powershell
gcc -O3 -fopenmp -Wall -Wextra src/q2_scheduling_nowait.c -o bin/q2_scheduling_nowait.exe
.\bin\q2_scheduling_nowait.exe [n=100] [scale_us=20.0]
```

### Question 3: Monte Carlo Pi Estimation
```powershell
gcc -O3 -fopenmp -Wall -Wextra src/q3_monte_carlo_pi.c -o bin/q3_monte_carlo_pi.exe -lm
.\bin\q3_monte_carlo_pi.exe
```

### Question 4: Parallel Binary Tree Sum Traversal
```powershell
gcc -O3 -fopenmp -Wall -Wextra src/q4_binary_tree_sum.c -o bin/q4_binary_tree_sum.exe
.\bin\q4_binary_tree_sum.exe [depth=22] [cutoff=10]
```

### Question 5: Parallel BFS & DFS Traversals
```powershell
gcc -O3 -fopenmp -Wall -Wextra -Wl,--stack,67108864 src/q5_bfs_dfs.c -o bin/q5_bfs_dfs.exe
.\bin\q5_bfs_dfs.exe [num_vertices=30000] [avg_degree=4]
```

### Question 6: OpenMP Default Loop Scheduling Detection
```powershell
gcc -O3 -fopenmp -Wall -Wextra src/q6_default_schedule.c -o bin/q6_default_schedule.exe
.\bin\q6_default_schedule.exe
```

### Optional: Steady-State Heat Conduction Solver
```powershell
gcc -O3 -fopenmp -Wall -Wextra src/optional_heat_conduction.c -o bin/optional_heat_conduction.exe -lm
.\bin\optional_heat_conduction.exe [grid_size=256] [max_iter=2500]
```

---

## 📊 Generating Visual Plots & Screenshots

To regenerate graphs and screenshots from existing benchmark results:

```powershell
# Generate Monte Carlo plots in graphs/
& "C:\Users\admin h\AppData\Local\Programs\Python\Python312\python.exe" plot_pi.py

# Render terminal screenshots in screenshots/
& "C:\Users\admin h\AppData\Local\Programs\Python\Python312\python.exe" render_screenshots.py
```

---

## 📖 Detailed Analysis & Report

For the in-depth theoretical proofs, mathematical derivations, analysis of race conditions, and complete benchmark interpretation, see [REPORT.md](REPORT.md).
