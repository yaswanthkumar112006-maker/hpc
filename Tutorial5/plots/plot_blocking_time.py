import matplotlib.pyplot as plt
import numpy as np
import os

# Set clean academic style
plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')

csv_path = os.path.join(os.path.dirname(__file__), '..', 'results', 'blocking_results.csv')
output_path = os.path.join(os.path.dirname(__file__), 'message_size_vs_time.png')

sizes = []
trial1 = []
trial2 = []
trial3 = []
averages = []

with open(csv_path, 'r') as f:
    lines = [line.strip() for line in f if line.strip()]
    header = lines[0].split(',')
    for line in lines[1:]:
        parts = line.split(',')
        sizes.append(float(parts[0]))
        trial1.append(float(parts[2]))
        trial2.append(float(parts[3]))
        trial3.append(float(parts[4]))
        averages.append(float(parts[5]))

fig, ax = plt.subplots(figsize=(8, 5), dpi=300)

# Plot trials as scattered points
for s, t1, t2, t3 in zip(sizes, trial1, trial2, trial3):
    ax.scatter([s, s, s], [t1, t2, t3], color='gray', alpha=0.6, s=30, zorder=3)

# Plot average line
ax.plot(sizes, averages, color='#1f77b4', marker='o', linewidth=2.2, markersize=7, label='Average Time', zorder=4)

# Annotate each average point
for s, avg in zip(sizes, averages):
    ax.annotate(f"{avg:.6f} s\n({s:.0f} MB)",
                xy=(s, avg),
                xytext=(0, 12),
                textcoords="offset points",
                ha='center',
                fontsize=9,
                fontweight='semibold',
                color='#0d3b66',
                bbox=dict(boxstyle="round,pad=0.2", facecolor='#f4f7fb', edgecolor='#b0c4de', alpha=0.9))

ax.set_title("MPI Blocking Communication: Message Size vs. Elapsed Time\n(Point-to-Point MPI_Send / MPI_Recv)", fontsize=13, fontweight='bold', pad=15)
ax.set_xlabel("Message Size (MB)", fontsize=11, labelpad=10)
ax.set_ylabel("Average Communication Time (seconds)", fontsize=11, labelpad=10)
ax.set_xticks(sizes)
ax.set_xticklabels([f"{int(s)} MB" for s in sizes])

# Add grid
ax.grid(True, linestyle='--', alpha=0.6)

# Legend
ax.scatter([], [], color='gray', alpha=0.6, s=30, label='Individual Trials (1-3)')
ax.legend(frameon=True, facecolor='white', framealpha=0.9, loc='upper left')

plt.tight_layout()
plt.savefig(output_path)
print(f"Graph saved successfully to: {output_path}")
