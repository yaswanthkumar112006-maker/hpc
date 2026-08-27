import matplotlib.pyplot as plt
import math
import os

os.makedirs("graphs", exist_ok=True)

# Serial results
points = [10, 100, 10000, 1000000, 100000000]
pi_values = [4.000000, 3.120000, 3.151200, 3.1415312, 3.14168628]

# Parallel results
threads = [1, 2, 4, 8, 16]
times = [0.019816, 0.020858, 0.011716, 0.017171, 0.011534]

serial_time = 0.023569

errors = [abs(x - math.pi) for x in pi_values]
speedup = [serial_time / t for t in times]
efficiency = [
    speedup[i] / threads[i] * 100
    for i in range(len(threads))
]

# 1. Points vs Estimated Pi
plt.figure()
plt.plot(points, pi_values, marker="o")
plt.axhline(math.pi, linestyle="--", label="Actual Pi")
plt.xscale("log")
plt.xlabel("Number of Points")
plt.ylabel("Estimated Pi")
plt.title("Number of Points vs Estimated Pi")
plt.legend()
plt.grid()
plt.savefig("graphs/points_vs_pi.png")
plt.close()

# 2. Points vs Error
plt.figure()
plt.plot(points, errors, marker="o")
plt.xscale("log")
plt.yscale("log")
plt.xlabel("Number of Points")
plt.ylabel("Absolute Error")
plt.title("Number of Points vs Error")
plt.grid()
plt.savefig("graphs/points_vs_error.png")
plt.close()

# 3. Threads vs Execution Time
plt.figure()
plt.plot(threads, times, marker="o")
plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (seconds)")
plt.title("Threads vs Execution Time")
plt.xticks(threads)
plt.grid()
plt.savefig("graphs/threads_vs_execution_time.png")
plt.close()

# 4. Threads vs Speedup
plt.figure()
plt.plot(threads, speedup, marker="o")
plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title("Threads vs Speedup")
plt.xticks(threads)
plt.grid()
plt.savefig("graphs/threads_vs_speedup.png")
plt.close()

# 5. Threads vs Efficiency
plt.figure()
plt.plot(threads, efficiency, marker="o")
plt.xlabel("Number of Threads")
plt.ylabel("Efficiency (%)")
plt.title("Threads vs Efficiency")
plt.xticks(threads)
plt.grid()
plt.savefig("graphs/threads_vs_efficiency.png")
plt.close()

print("All 5 graphs created successfully!")