# HPC Tutorial 5 - MPI Programming

This folder contains my solutions and experimental results for HPC Tutorial 5 on MPI (Message Passing Interface).

## Folder Structure
- `src/`: C source code for Problems 1 to 5
- `bin/`: Compiled executables
- `results/`: Terminal output text files and CSV data
- `plots/`: Graph comparing message size and time
- `Makefile`: Script to build all programs

## How to Compile and Run

To compile all programs:
```bash
make
```

To run each problem:
```bash
# Problem 1 (test with 2 and 4 processes)
mpirun -np 2 ./bin/problem1_hello
mpirun -np 4 ./bin/problem1_hello

# Problem 2 (blocking communication)
mpirun -np 2 ./bin/problem2_blocking

# Problem 3 (non-blocking communication)
mpirun -np 2 ./bin/problem3_nonblocking

# Problem 4 (ring communication)
mpirun -np 4 ./bin/problem4_ring

# Problem 5 (2D grid communication)
mpirun -np 4 ./bin/problem5_grid
mpirun -np 4 ./bin/problem5_cartesian
```

---

## Problem 1: Hello World with MPI

### What it does:
Initializes MPI, gets the rank of each process and the total number of processes, prints a message, and finalizes MPI.

### Output:
```text
=== 2 Processes ===
Hello World from process 0 of 2
Hello World from process 1 of 2

=== 4 Processes ===
Hello World from process 0 of 4
Hello World from process 1 of 4
Hello World from process 2 of 4
Hello World from process 3 of 4
```

### Why different ranks are printed:
When we run `mpirun -np 4`, four separate copies of the program run at the same time. The MPI runtime assigns each copy a different ID number (rank from 0 to 3). Each process queries its own rank and prints it.

---

## Problem 2: Blocking Communication

### What it does:
Process 0 sends an array of integers to Process 1 using `MPI_Send`, and Process 1 receives it using `MPI_Recv`. We test three sizes: 1 MB (250,000 ints), 10 MB (2,500,000 ints), and 50 MB (12,500,000 ints). We run 3 trials for each size and measure time using `MPI_Wtime()`.

### Timing Results:

| Message Size | Trial 1 (s) | Trial 2 (s) | Trial 3 (s) | Average Time (s) |
| :--- | :--- | :--- | :--- | :--- |
| **1 MB** | 0.000593 | 0.000312 | 0.000094 | **0.000333** |
| **10 MB** | 0.005732 | 0.004594 | 0.001475 | **0.003933** |
| **50 MB** | 0.041450 | 0.019576 | 0.018103 | **0.026376** |

### Graph:
![Message Size vs Time](plots/message_size_vs_time.png)

### Answers to Questions:
1. **Why does transfer time increase as message size grows?**  
   Larger messages have more bytes to copy into memory and send, so it naturally takes more time.

2. **Does doubling the message size always double the time?**  
   Not always. For small messages, the fixed startup time (latency) is what takes most of the time. For big messages, bandwidth and memory copying matter much more.

3. **What influence do latency, bandwidth, and system load have?**  
   - Latency is the startup delay to begin sending.  
   - Bandwidth controls how fast data moves once started.  
   - Memory copying adds CPU time to move data between buffers.  
   - System load (other running background tasks) causes small timing variations between trials.

4. **What does blocking mean? Does MPI_Send returning mean the receiver got it?**  
   Blocking means the function waits until its buffer is safe to reuse. `MPI_Send` returning does *not* mean the receiver has already processed the message; it only means the data has left the sender's buffer.

---

## Problem 3: Non-Blocking Communication and Overlap

### What it does:
We use `MPI_Isend` and `MPI_Irecv` so communication starts in the background while both processes do some dummy calculations. Then `MPI_Wait` is called to make sure the data transfer finished before reading or freeing the data.

### Comparison Table:

| Message Size | Blocking Time (s) | Non-Blocking Time (s) | Difference / Observation |
| :--- | :--- | :--- | :--- |
| **1 MB** | 0.119610 | 0.119207 | Almost same (computation took most of the time) |
| **10 MB** | 0.109266 | 0.111917 | Similar time due to system scheduling noise |
| **50 MB** | 0.135769 | 0.113964 | **Saved ~0.022 seconds** due to overlap |

### Discussion:
At 50 MB, the non-blocking version was noticeably faster because the data transfer happened in the background while the CPU was executing the dummy math loop.

---

## Problem 4: Ring Topology

### What it does:
Processes form a circle (ring). Each process sends data to its next neighbor and receives from its previous neighbor:
- `next = (rank + 1) % size`
- `previous = (rank - 1 + size) % size`

### Output (4 processes):
```text
[Process 0] Neighbors: previous = 3, next = 1
[Process 1] Neighbors: previous = 0, next = 2
[Process 2] Neighbors: previous = 1, next = 3
[Process 3] Neighbors: previous = 2, next = 0

--- Method 1: MPI_Sendrecv ---
[Process 0] Sent 100 to Rank 1 | Received 130 from Rank 3
[Process 1] Sent 110 to Rank 2 | Received 100 from Rank 0
[Process 2] Sent 120 to Rank 3 | Received 110 from Rank 1
[Process 3] Sent 130 to Rank 0 | Received 120 from Rank 2
```

### Why deadlock happens and how we solved it:
If all processes call `MPI_Send` at the same time, everyone is waiting for someone else to receive, which creates a deadlock.  
We solved this in two ways:
1. `MPI_Sendrecv`: Sends and receives at the same time safely.
2. Even-Odd ordering: Even ranks send first then receive; odd ranks receive first then send.

---

## Problem 5: 2D Grid Topology

### What it does:
Arranges 4 processes in a 2x2 grid. Coordinates are:
- `row = rank / 2`
- `col = rank % 2`

Each process finds its Up, Down, Left, and Right neighbors. If a neighbor is across the boundary, we use `MPI_PROC_NULL`.

### Grid Neighbors:
- Process 0 at (0, 0): Down = 2, Right = 1 (Up/Left = none)
- Process 1 at (0, 1): Down = 3, Left = 0 (Up/Right = none)
- Process 2 at (1, 0): Up = 0, Right = 3 (Down/Left = none)
- Process 3 at (1, 1): Up = 1, Left = 2 (Down/Right = none)

### Output:
```text
[Process 0] Grid (0, 0) -> Up: NONE, Down: Rank 2, Left: NONE, Right: Rank 1
[Process 1] Grid (0, 1) -> Up: NONE, Down: Rank 3, Left: Rank 0, Right: NONE
[Process 2] Grid (1, 0) -> Up: Rank 0, Down: NONE, Left: NONE, Right: Rank 3
[Process 3] Grid (1, 1) -> Up: Rank 1, Down: NONE, Left: Rank 2, Right: NONE
```

### Extension:
We also tested `problem5_cartesian.c` using MPI's built-in `MPI_Cart_create` and `MPI_Cart_shift`, which automatically finds the neighbors and handles boundaries cleanly.

---

## Conclusion
In this tutorial, I learned how to create basic MPI programs, measure transfer speeds for different message sizes, overlap communication and computation with non-blocking calls, and design deadlock-free ring and 2D grid topologies.
