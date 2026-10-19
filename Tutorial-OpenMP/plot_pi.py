import os
import math
import csv
import matplotlib.pyplot as plt

os.makedirs("graphs", exist_ok=True)

# -------------------------------------------------------------
# 1. Points vs Estimated Pi & Points vs Error
# -------------------------------------------------------------
results_file = "results/pi_results.csv"
if os.path.exists(results_file):
    points = []
    pi_est = []
    errors = []
    with open(results_file, mode='r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            points.append(int(row["points"]))
            pi_est.append(float(row["estimated_pi"]))
            errors.append(float(row["error"]))

    # Plot 1: Number of Points vs Estimated Pi
    plt.figure(figsize=(8, 5), dpi=300)
    plt.plot(points, pi_est, marker='o', color='#1f77b4', linewidth=2, label='Estimated $\\pi$')
    plt.axhline(math.pi, color='crimson', linestyle='--', linewidth=2, label=f'True $\\pi$ ({math.pi:.6f})')
    plt.xscale('log')
    plt.xlabel('Number of Points ($N$)', fontsize=12, fontweight='bold')
    plt.ylabel('Estimated $\\pi$', fontsize=12, fontweight='bold')
    plt.title('Monte Carlo Estimation: Number of Points vs Estimated $\\pi$', fontsize=14, fontweight='bold')
    plt.grid(True, which="both", ls="--", alpha=0.6)
    plt.legend(fontsize=11)
    plt.tight_layout()
    plt.savefig('graphs/points_vs_pi.png')
    plt.close()
    print("Generated graphs/points_vs_pi.png")

    # Plot 2: Error vs Sample Size
    plt.figure(figsize=(8, 5), dpi=300)
    plt.plot(points, errors, marker='s', color='#d62728', linewidth=2, label='Empirical Absolute Error $|\\hat{\\pi} - \\pi|$')
    
    # Theoretical O(1 / sqrt(N)) curve for comparison
    ref_errors = [1.0 / math.sqrt(p) for p in points]
    scale_factor = errors[2] / ref_errors[2] if len(errors) > 2 else 1.0
    plt.plot(points, [r * scale_factor for r in ref_errors], linestyle=':', color='#555555', label=r'Theoretical $O(1/\sqrt{N})$ Trend')

    plt.xscale('log')
    plt.yscale('log')
    plt.xlabel('Sample Size ($N$)', fontsize=12, fontweight='bold')
    plt.ylabel('Absolute Error $|\\hat{\\pi} - \\pi|$', fontsize=12, fontweight='bold')
    plt.title('Monte Carlo Error Convergence vs Sample Size', fontsize=14, fontweight='bold')
    plt.grid(True, which="both", ls="--", alpha=0.6)
    plt.legend(fontsize=11)
    plt.tight_layout()
    plt.savefig('graphs/points_vs_error.png')
    plt.close()
    print("Generated graphs/points_vs_error.png")

# -------------------------------------------------------------
# 2. Thread Scalability Graphs (Execution Time, Speedup, Efficiency)
# -------------------------------------------------------------
scaling_file = "results/pi_scaling.csv"
if os.path.exists(scaling_file):
    threads = []
    times = []
    speedup = []
    efficiency = []
    with open(scaling_file, mode='r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            threads.append(int(row["threads"]))
            times.append(float(row["time_sec"]))
            speedup.append(float(row["speedup"]))
            efficiency.append(float(row["efficiency"]))

    # Plot 3: Threads vs Execution Time
    plt.figure(figsize=(8, 5), dpi=300)
    plt.plot(threads, times, marker='o', color='#2ca02c', linewidth=2)
    plt.xlabel('Number of Threads', fontsize=12, fontweight='bold')
    plt.ylabel('Execution Time (seconds)', fontsize=12, fontweight='bold')
    plt.title('Monte Carlo: Threads vs Execution Time ($N = 10^8$)', fontsize=14, fontweight='bold')
    plt.xticks(threads)
    plt.grid(True, ls="--", alpha=0.6)
    plt.tight_layout()
    plt.savefig('graphs/threads_vs_execution_time.png')
    plt.close()
    print("Generated graphs/threads_vs_execution_time.png")

    # Plot 4: Threads vs Speedup
    plt.figure(figsize=(8, 5), dpi=300)
    plt.plot(threads, speedup, marker='^', color='#9467bd', linewidth=2, label='Measured Speedup')
    plt.plot(threads, threads, linestyle='--', color='gray', label='Ideal Linear Speedup')
    plt.xlabel('Number of Threads', fontsize=12, fontweight='bold')
    plt.ylabel('Speedup ($T_1 / T_p$)', fontsize=12, fontweight='bold')
    plt.title('Monte Carlo: Threads vs Speedup ($N = 10^8$)', fontsize=14, fontweight='bold')
    plt.xticks(threads)
    plt.legend(fontsize=11)
    plt.grid(True, ls="--", alpha=0.6)
    plt.tight_layout()
    plt.savefig('graphs/threads_vs_speedup.png')
    plt.close()
    print("Generated graphs/threads_vs_speedup.png")

    # Plot 5: Threads vs Efficiency
    plt.figure(figsize=(8, 5), dpi=300)
    plt.plot(threads, efficiency, marker='D', color='#ff7f0e', linewidth=2)
    plt.axhline(100.0, linestyle='--', color='gray', label='100% Ideal Efficiency')
    plt.xlabel('Number of Threads', fontsize=12, fontweight='bold')
    plt.ylabel('Parallel Efficiency (%)', fontsize=12, fontweight='bold')
    plt.title('Monte Carlo: Threads vs Parallel Efficiency', fontsize=14, fontweight='bold')
    plt.xticks(threads)
    plt.ylim(0, 115)
    plt.grid(True, ls="--", alpha=0.6)
    plt.tight_layout()
    plt.savefig('graphs/threads_vs_efficiency.png')
    plt.close()
    print("Generated graphs/threads_vs_efficiency.png")

print("All Monte Carlo graphs successfully generated!")
