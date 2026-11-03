# HPC-2026: Tutorial 5 – MPI Programming Tutorial

**Course:** High Performance Computing (HPC-2026)  
**Topic:** Message Passing Interface (MPI) – Blocking & Non-Blocking Communication, Topologies, and Performance Analysis  
**Environment:** Linux (Ubuntu 24.04 LTS / GitHub Codespaces), GCC & OpenMPI / MPICH  

---

## 1. Directory Structure

```text
Tutorial5/
├── bin/                             # Compiled MPI executables
│   ├── problem1_hello
│   ├── problem2_blocking
│   ├── problem3_nonblocking
│   ├── problem4_ring
│   ├── problem5_grid
│   └── problem5_cartesian
├── plots/                           # Python plotting script & generated graphs
│   ├── message_size_vs_time.png     # Required communication time vs. size plot
│   └── plot_blocking_time.py        # Matplotlib visualization script
├── results/                         # Actual execution terminal logs & CSV data
│   ├── blocking_results.csv         # Measured times for 1 MB, 10 MB, 50 MB
│   ├── nonblocking_comparison.csv   # Overlap comparison measurements
│   ├── problem1_output.txt          # Terminal output with 2 and 4 processes
│   ├── problem2_output.txt          # Terminal output for blocking transfers
│   ├── problem3_output.txt          # Terminal output for non-blocking overlap
│   ├── problem4_output.txt          # Terminal output for ring topology
│   ├── problem5_output.txt          # Terminal output for 2D grid
│   └── problem5_cartesian_output.txt# Terminal output for Cartesian topology
├── src/                             # Source code for all lab problems
│   ├── problem1_hello.c             # Problem 1: MPI Hello World
│   ├── problem2_blocking.c          # Problem 2: Blocking Point-to-Point
│   ├── problem3_nonblocking.c       # Problem 3: Non-Blocking Overlap
│   ├── problem4_ring.c              # Problem 4: Ring Topology (MPI_Sendrecv / Even-Odd)
│   ├── problem5_grid.c              # Problem 5: 2D Grid Topology
│   └── problem5_cartesian.c         # Problem 5 Extension: MPI_Cart_create / shift
├── Makefile                         # Easy compilation of all problems
└── README.md                        # Complete lab documentation and analysis
```

---

## 2. Compilation and Execution Guide

A `Makefile` is provided to compile all problems cleanly with `mpicc -O2 -Wall`.

### Compiling All Programs
```bash
make
```

### Running Individual Problems
```bash
# Problem 1: Run with 2 and 4 processes
mpirun -np 2 ./bin/problem1_hello
mpirun -np 4 ./bin/problem1_hello

# Problem 2: Run blocking point-to-point transfer (2 processes)
mpirun -np 2 ./bin/problem2_blocking

# Problem 3: Run non-blocking communication with computation overlap (2 processes)
mpirun -np 2 ./bin/problem3_nonblocking

# Problem 4: Run ring communication (4 processes)
mpirun -np 4 ./bin/problem4_ring

# Problem 5: Run 2D grid topology (4 processes for 2x2 grid)
mpirun -np 4 ./bin/problem5_grid

# Problem 5 Extension: Run Cartesian virtual topology
mpirun -np 4 ./bin/problem5_cartesian
```

### Cleaning Binaries
```bash
make clean
```

---

## 3. Problem 1 — Hello World with MPI

### Objective & Functions Used
The goal of Problem 1 is to understand the initialization, rank discovery, and finalization of an MPI execution environment:
- `MPI_Init(&argc, &argv)`: Initializes the MPI execution environment and sets up communication channels.
- `MPI_Comm_rank(MPI_COMM_WORLD, &rank)`: Obtains the unique integer rank (0 to size - 1) assigned to the calling process.
- `MPI_Comm_size(MPI_COMM_WORLD, &size)`: Retrieves the total count of processes cooperating in `MPI_COMM_WORLD`.
- `printf(...)`: Prints the greeting containing the rank and communicator size.
- `MPI_Finalize()`: Cleans up all MPI state, closes sockets/buffers, and terminates the MPI session.

### Execution Output
```text
=== Execution with 2 processes (mpirun -np 2 ./bin/problem1_hello) ===
Hello World from process 0 of 2
Hello World from process 1 of 2

=== Execution with 4 processes (mpirun -np 4 ./bin/problem1_hello) ===
Hello World from process 0 of 4
Hello World from process 1 of 4
Hello World from process 2 of 4
Hello World from process 3 of 4
```

### Why Does the Same Program Print Different Ranks?
When we compile `problem1_hello.c`, a single binary executable is created. When launched with `mpirun -np 4`, the MPI runtime spawns **four independent process instances**, each possessing its own private address space and memory.

During execution, each process calls `MPI_Comm_rank(MPI_COMM_WORLD, &rank)`. The MPI runtime assigns a distinct rank integer (`0`, `1`, `2`, `3`) to each instance based on the process launch table. Because each process queries its own rank into its local `rank` variable, the exact same `printf` statement outputs a different rank for every process. Furthermore, output lines may arrive in varying orders across different runs because the operating system schedules each process concurrently.

---

## 4. Problem 2 — Blocking Point-to-Point Communication

### Objective & Design
Send an integer array from Process 0 to Process 1 using `MPI_Send` and `MPI_Recv`, verifying receipt by printing the first and last elements, and accurately measuring communication time using `MPI_Wtime()`.

### Message Size Calculations (Decimal MB)
Assuming `sizeof(int) = 4` bytes and decimal MB ($1\text{ MB} = 1{,}000{,}000\text{ bytes}$):
- **1 MB:** $1{,}000{,}000\text{ bytes} \div 4 = 250{,}000$ integers
- **10 MB:** $10{,}000{,}000\text{ bytes} \div 4 = 2{,}500{,}000$ integers
- **50 MB:** $50{,}000{,}000\text{ bytes} \div 4 = 12{,}500{,}000$ integers

### Timing Methodology
`MPI_Wtime()` returns the local elapsed wall-clock time in seconds. Because process clocks can drift or be out of sync, timestamps from different processes should never be directly subtracted. Therefore:
1. `MPI_Barrier(MPI_COMM_WORLD)` synchronizes both processes before the transfer begins.
2. Process 0 records `start_time = MPI_Wtime()`.
3. Process 0 performs `MPI_Send`, and Process 1 receives with `MPI_Recv`.
4. A trailing `MPI_Barrier(MPI_COMM_WORLD)` ensures both sender and receiver have completed the message exchange.
5. `elapsed_time = MPI_Wtime() - start_time` is recorded on Process 0.

### Actual Measured Results Table
The following measurements were recorded over 3 trials for each message size:

| Message Size | Integers | Trial 1 (s) | Trial 2 (s) | Trial 3 (s) | Average (s) | Effective Bandwidth (GB/s) |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **1 MB** | 250,000 | 0.000593 | 0.000312 | 0.000094 | **0.000333** | 3.00 GB/s |
| **10 MB** | 2,500,000 | 0.005732 | 0.004594 | 0.001475 | **0.003933** | 2.54 GB/s |
| **50 MB** | 12,500,000 | 0.041450 | 0.019576 | 0.018103 | **0.026376** | 1.90 GB/s |

### Verification of Received Data (from Terminal Output)
- **1 MB:** Process 1 received: First element = `1`, Last element = `250000` (Verified correct)
- **10 MB:** Process 1 received: First element = `1`, Last element = `2500000` (Verified correct)
- **50 MB:** Process 1 received: First element = `1`, Last element = `12500000` (Verified correct)

### Generated Plot
The relationship between message size and average elapsed communication time is visualized below:

![Message Size vs Average Time](plots/message_size_vs_time.png)

---

### Answers to Problem 2 Analysis Questions

#### 1. Why does transfer time generally increase as the message grows?
The total transmission time in message passing follows the classic linear network model:
$$T = \alpha + \beta \times S$$
where $\alpha$ is latency (startup overhead), $\beta$ is the inverse bandwidth ($1 / \text{Bandwidth}$), and $S$ is the message size in bytes. As the message size $S$ increases from 1 MB to 50 MB, the transmission term $\beta \times S$ dominates. The hardware must copy and transfer 50 times more data through memory buffers, bus channels, or network interfaces, naturally requiring more time.

#### 2. Does doubling the message size always double the time? Why or why not?
**No, doubling the message size does not always strictly double the time.**
- For very small messages, latency $\alpha$ dominates, so doubling the size produces almost no change in total time.
- For intermediate messages, MPI switches transmission protocols (e.g., from eager protocol using small internal buffers to rendezvous protocol requiring handshakes).
- For very large messages (10 MB to 50 MB), data exceeds L2 and L3 CPU cache capacities, causing memory bus saturation and additional memory paging overhead. Hence, the scaling can be slightly non-linear depending on system memory bandwidth.

#### 3. What influence do latency, bandwidth, memory copying, and system load have?
- **Latency ($\alpha$):** The fixed time to initiate a message (software stack traversal, packet framing, and protocol handshakes). Latency sets the baseline floor for tiny messages.
- **Bandwidth ($1/\beta$):** The rate at which the interconnect (or shared memory bus) transfers data once transmission starts. Bandwidth dictates performance for large transfers (like our 10 MB and 50 MB messages).
- **Memory Copying:** In shared memory or host communication, data must often be copied from the sender application buffer to the MPI internal transport buffer, and then into the receiver application buffer. Memory copy speed directly impacts elapsed time.
- **System Load:** Competing OS processes, CPU scheduling interrupts, and cache thrashing introduce timing jitter between trials (as seen in Trial 1 being slightly higher than Trial 3 due to cache warmup).

#### 4. What does blocking mean for the calling process? Does `MPI_Send` returning always mean the receiver has processed the message?
- **Meaning of Blocking:** A blocking call (`MPI_Send` or `MPI_Recv`) means the calling process pauses execution and will **not return control** until the resources passed to the function are safe to reuse.
- **Does `MPI_Send` returning mean the receiver has processed the message?**
  **No.** `MPI_Send` returns as soon as the sender's buffer has been safely copied (either into an MPI system buffer or delivered to the transport layer). The receiving process may still be waiting to call `MPI_Recv` or may have only partially read the data. Returning only guarantees that the sender can safely modify or overwrite its buffer without corrupting the message in transit.

---

## 5. Problem 3 — Non-Blocking Communication and Overlap

### Objective & Correctness Rules
The objective is to replace `MPI_Send`/`MPI_Recv` with non-blocking primitives `MPI_Isend`, `MPI_Irecv`, and `MPI_Wait`, and investigate whether independent CPU computation can overlap with ongoing communication.

**Key Correctness Rules Adhered To:**
1. **Never modify or reuse the send buffer** while an `MPI_Isend` request is outstanding.
2. **Never read or process the receive buffer** before the matching `MPI_Wait(&request)` call completes.
3. **Independent Computation:** The computation performed during overlap must not depend on the incoming data.

### Experimental Comparison Table
We executed identical dummy computation workloads (50,000,000 arithmetic loop iterations) under both sequential blocking communication and non-blocking overlapped communication:

| Message Size | Blocking Time (s) | Non-Blocking Time (s) | Time Saved (s) | Observation |
| :--- | :--- | :--- | :--- | :--- |
| **1 MB** | 0.119610 | 0.119207 | +0.000403 | Minor overlap benefit (computation dominates) |
| **10 MB** | 0.109266 | 0.111917 | -0.002651 | Comparable time (within system scheduler noise) |
| **50 MB** | 0.135769 | 0.113964 | **+0.021805** | **Significant overlap observed (~21.8 ms saved)** |

### Discussion: Did Dummy Computation Overlap with Communication?
**Yes, meaningful overlap occurred, particularly at 50 MB.**
- Under blocking communication, Process 0 and 1 must finish transferring the entire 50 MB payload (taking ~0.026 s) before starting the computation loop (~0.110 s), leading to an additive total time of $0.026 + 0.110 \approx 0.136\text{ seconds}$.
- Under non-blocking communication, `MPI_Isend` and `MPI_Irecv` initiate the transfer asynchronously. While the memory copy / transfer proceeds in the background, the CPU concurrently executes the dummy math loop. When `MPI_Wait` is reached, the communication has already finished, reducing total runtime to approximately $\max(T_{\text{comm}}, T_{\text{comp}}) \approx 0.114\text{ seconds}$.
- **Influencing Factors:** On modern multi-core systems, hardware DMA or separate CPU cores allow memory copies to proceed in parallel with ALU operations. However, for small messages (1 MB), communication completes so fast that overlap is negligible compared to the computation time.

---

## 6. Problem 4 — Ring Communication Topology

### Objective & Topology Formulas
Processes are arranged in a logical 1D ring where each process sends a message to its next neighbor and receives a message from its previous neighbor:
$$\text{next} = (\text{rank} + 1) \pmod{\text{size}}$$
$$\text{previous} = (\text{rank} - 1 + \text{size}) \pmod{\text{size}}$$

### Neighbor Mapping for 4 Processes

| Process Rank | Previous Neighbor | Next Neighbor |
| :---: | :---: | :---: |
| **0** | 3 | 1 |
| **1** | 0 | 2 |
| **2** | 1 | 3 |
| **3** | 2 | 0 |

### How Send/Receive Ordering Causes Deadlock
If every process naively executes:
```c
MPI_Send(..., next, ...);
MPI_Recv(..., previous, ...);
```
when message sizes exceed small internal system buffers (or when synchronous mode is used), every process blocks waiting for its `next` neighbor to receive. But the `next` neighbor is also blocked trying to send to its own next neighbor. This creates a **circular wait condition** across all processes, causing the entire MPI job to hang indefinitely in a deadlock.

### Deadlock Solutions Implemented
We implemented and tested two safe solutions in `problem4_ring.c`:
1. **`MPI_Sendrecv`:** Combines send and receive into a single atomic MPI call. The MPI runtime internally manages buffering or non-blocking transfers to ensure neither process deadlocks.
2. **Even-Odd Ordering:**
   - **Even ranks (`rank % 2 == 0`)** send to `next` first, then receive from `previous`.
   - **Odd ranks (`rank % 2 == 1`)** receive from `previous` first, then send to `next`.
   This breaks the circular wait dependency cycle completely.

### Actual Execution Output
```text
=================================================================
  Problem 4: Ring Communication Topology (Size = 4 processes)   
=================================================================

[Process 0] Neighbors -> Previous: 3 | Next: 1
[Process 1] Neighbors -> Previous: 0 | Next: 2
[Process 2] Neighbors -> Previous: 1 | Next: 3
[Process 3] Neighbors -> Previous: 2 | Next: 0

--- Step 1: Communication using MPI_Sendrecv ---
[Process 0] Sent 100 to Next (Rank 1) | Received 130 from Previous (Rank 3)
[Process 1] Sent 110 to Next (Rank 2) | Received 100 from Previous (Rank 0)
[Process 2] Sent 120 to Next (Rank 3) | Received 110 from Previous (Rank 1)
[Process 3] Sent 130 to Next (Rank 0) | Received 120 from Previous (Rank 2)

--- Step 2: Communication using Even-Odd Send/Recv Ordering ---
[Process 0] (Even-Odd) Sent 10 to Rank 1 | Received 160 from Rank 3
[Process 1] (Even-Odd) Sent 60 to Rank 2 | Received 10 from Rank 0
[Process 2] (Even-Odd) Sent 110 to Rank 3 | Received 60 from Rank 1
[Process 3] (Even-Odd) Sent 160 to Rank 0 | Received 110 from Rank 2
```

---

## 7. Problem 5 — Two-Dimensional Grid Topology

### Objective & Layout
Arrange 4 processes into a $2 \times 2$ grid with non-periodic boundaries, exchanging messages in four cardinal directions (Up, Down, Left, Right).

```text
+---------------+---------------+
|   Process 0   |   Process 1   |
| (row=0,col=0) | (row=0,col=1) |
+---------------+---------------+
|   Process 2   |   Process 3   |
| (row=1,col=0) | (row=1,col=1) |
+---------------+---------------+
```

For a row-major layout with $C = 2$ columns:
$$\text{row} = \lfloor\text{rank} / C\rfloor, \quad \text{column} = \text{rank} \pmod{C}$$

### Boundary Neighbor Mapping

| Rank | Coordinates | Up Neighbor | Down Neighbor | Left Neighbor | Right Neighbor |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **0** | (0, 0) | `MPI_PROC_NULL` | Rank 2 | `MPI_PROC_NULL` | Rank 1 |
| **1** | (0, 1) | `MPI_PROC_NULL` | Rank 3 | Rank 0 | `MPI_PROC_NULL` |
| **2** | (1, 0) | Rank 0 | `MPI_PROC_NULL` | `MPI_PROC_NULL` | Rank 3 |
| **3** | (1, 1) | Rank 1 | `MPI_PROC_NULL` | Rank 2 | `MPI_PROC_NULL` |

### Boundary Handling with `MPI_PROC_NULL`
When a process lies on a grid boundary (e.g. Process 0 has no Up or Left neighbor), its neighbor rank is assigned `MPI_PROC_NULL`. In the MPI standard, sending to or receiving from `MPI_PROC_NULL` is a valid no-op that succeeds immediately without modifying memory or blocking. This allows uniform communication code without complex boundary conditional statements.

### Execution Output (`problem5_grid.c`)
```text
=================================================================
  Problem 5: Two-Dimensional Grid Topology (2x2 Grid)            
=================================================================

[Process 0] at Grid Position (row=0, col=0):
    Up:    NONE (Boundary)
    Down:  Rank 2
    Left:  NONE (Boundary)
    Right: Rank 1

[Process 1] at Grid Position (row=0, col=1):
    Up:    NONE (Boundary)
    Down:  Rank 3
    Left:  Rank 0
    Right: NONE (Boundary)

[Process 2] at Grid Position (row=1, col=0):
    Up:    Rank 0
    Down:  NONE (Boundary)
    Left:  NONE (Boundary)
    Right: Rank 3

[Process 3] at Grid Position (row=1, col=1):
    Up:    Rank 1
    Down:  NONE (Boundary)
    Left:  Rank 2
    Right: NONE (Boundary)

--- Message Exchange Verification ---
[Process 0] (Sent value 0):
    Received from DOWN  (Rank 2): 200
    Received from RIGHT (Rank 1): 100

[Process 1] (Sent value 100):
    Received from DOWN  (Rank 3): 300
    Received from LEFT  (Rank 0): 0

[Process 2] (Sent value 200):
    Received from UP    (Rank 0): 0
    Received from RIGHT (Rank 3): 300

[Process 3] (Sent value 300):
    Received from UP    (Rank 1): 100
    Received from LEFT  (Rank 2): 200
```

---

## 8. Problem 5 Extension — Cartesian Virtual Topology

### Objective & Functions Investigated
MPI provides native virtual topology constructors that automate grid coordinate math and neighbor discovery:
- `MPI_Cart_create(MPI_COMM_WORLD, ndims=2, dims=[2,2], periods=[0,0], reorder=0, &cart_comm)`: Builds a Cartesian communicator with non-wrapping edges.
- `MPI_Cart_coords(cart_comm, rank, maxdims=2, coords)`: Returns the `(row, col)` coordinate tuple.
- `MPI_Cart_shift(cart_comm, direction, disp=1, &source, &dest)`: Computes the neighbor ranks along dimension 0 (vertical shift for Up/Down) and dimension 1 (horizontal shift for Left/Right). It automatically substitutes `MPI_PROC_NULL` for boundary edges!

### Execution Output (`problem5_cartesian.c`)
```text
=================================================================
  Problem 5 Extension: MPI Cartesian Virtual Topology            
  Using MPI_Cart_create and MPI_Cart_shift                       
=================================================================

[Cartesian Process 0] Coordinates: row=0, col=0
    Up (Dim 0, -1):    NONE (MPI_PROC_NULL)
    Down (Dim 0, +1):  Rank 2
    Left (Dim 1, -1):  NONE (MPI_PROC_NULL)
    Right (Dim 1, +1): Rank 1

[Cartesian Process 1] Coordinates: row=0, col=1
    Up (Dim 0, -1):    NONE (MPI_PROC_NULL)
    Down (Dim 0, +1):  Rank 3
    Left (Dim 1, -1):  Rank 0
    Right (Dim 1, +1): NONE (MPI_PROC_NULL)

[Cartesian Process 2] Coordinates: row=1, col=0
    Up (Dim 0, -1):    Rank 0
    Down (Dim 0, +1):  NONE (MPI_PROC_NULL)
    Left (Dim 1, -1):  NONE (MPI_PROC_NULL)
    Right (Dim 1, +1): Rank 3

[Cartesian Process 3] Coordinates: row=1, col=1
    Up (Dim 0, -1):    Rank 1
    Down (Dim 0, +1):  NONE (MPI_PROC_NULL)
    Left (Dim 1, -1):  Rank 2
    Right (Dim 1, +1): NONE (MPI_PROC_NULL)

--- Cartesian Grid Exchange Verification ---
[Cartesian Process 0] (Sent value 25):
    Received from DOWN  (Rank 2): 75
    Received from RIGHT (Rank 1): 50

[Cartesian Process 1] (Sent value 50):
    Received from DOWN  (Rank 3): 100
    Received from LEFT  (Rank 0): 25

[Cartesian Process 2] (Sent value 75):
    Received from UP    (Rank 0): 25
    Received from RIGHT (Rank 3): 100

[Cartesian Process 3] (Sent value 100):
    Received from UP    (Rank 1): 50
    Received from LEFT  (Rank 2): 75
```

---

## 9. Conclusion

In this tutorial, all five core problems and extensions were successfully designed, implemented, compiled, and benchmarked:
1. **Basics:** Mastered the structure of MPI programs using `MPI_Init`, `MPI_Comm_rank`, `MPI_Comm_size`, and `MPI_Finalize`, observing how SPMD execution assigns distinct ranks to identical code copies.
2. **Blocking Communication:** Measured point-to-point transfer times across 1 MB, 10 MB, and 50 MB arrays, demonstrating how transmission time scales with message size and memory bandwidth.
3. **Non-Blocking Communication:** Proved that `MPI_Isend` and `MPI_Irecv` allow independent CPU computation to overlap with large data transfers, saving over 21 milliseconds on 50 MB messages.
4. **Ring Topology:** Explored communication deadlock stemming from symmetric blocking calls in circular topologies and verified two deadlock-free communication strategies (`MPI_Sendrecv` and Even-Odd rank scheduling).
5. **2D Grid & Cartesian Topologies:** Implemented 2D grid neighbor exchange using row-major index mapping and investigated MPI's built-in `MPI_Cart_create` and `MPI_Cart_shift` primitives for automated boundary management.
