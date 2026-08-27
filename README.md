# Tutorial 3 - Pthreads

## Q1 - Monte Carlo Pi

Implemented Monte Carlo estimation of Pi using:

- Serial C program
- Parallel Pthreads program
- 1, 2, 4, 8 and 16 threads
- Different numbers of random points
- Execution time measurement
- Speedup and efficiency calculation

### Graphs

- Points vs Estimated Pi
- Points vs Error
- Threads vs Execution Time
- Threads vs Speedup
- Threads vs Efficiency

### Files

- `serial.c` - Serial implementation
- `parallel.c` - Pthreads implementation
- `plot_results.py` - Generates graphs
- `graphs/` - Generated graphs
- `screenshots/` - Execution screenshots


## Q2 - Edge Detection

Implemented 3x3 kernel-based edge detection using:

- Serial C program
- Parallel Pthreads program
- Row-wise division of image among threads
- 1, 2, 4, 8 and 16 threads
- Execution time measurement
- Speedup and efficiency calculation

### Graphs

- Threads vs Execution Time
- Threads vs Speedup
- Threads vs Efficiency

### Files

- `serial.c` - Serial implementation
- `parallel.c` - Pthreads implementation
- `plot_results.py` - Generates graphs
- `graphs/` - Generated graphs
- `screenshots/` - Execution screenshots


## Q3 - Dining Philosophers

Implemented the Dining Philosophers problem using Pthreads.

### Solutions

1. Left chopstick followed by right chopstick
2. Resource ordering
3. Waiter / Arbitrator approach

### Files

- `DP_D.c` - Solution 1
- `DP_O.c` - Solution 2
- `DP_W.c` - Solution 3
- `screenshots/` - Execution screenshots


## Conclusion

This tutorial demonstrates parallel programming using POSIX threads (Pthreads), including workload distribution, synchronization, deadlock avoidance, execution-time comparison, speedup and efficiency analysis.