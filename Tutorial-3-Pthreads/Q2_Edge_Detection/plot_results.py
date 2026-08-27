import matplotlib.pyplot as plt

threads = [1, 2, 4, 8, 16]

times = [0.039217, 0.033559, 0.031248, 0.029337, 0.040461]

speedup = [0.925450, 1.081457, 1.161432, 1.237096, 0.896995]

efficiency = [92.55, 54.07, 29.04, 15.46, 5.61]

plt.figure()
plt.plot(threads, times, marker="o")
plt.xlabel("Number of Threads")
plt.ylabel("Execution Time (seconds)")
plt.title("Threads vs Execution Time")
plt.xticks(threads)
plt.grid()
plt.savefig("graphs/threads_vs_execution_time.png")
plt.close()

plt.figure()
plt.plot(threads, speedup, marker="o")
plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title("Threads vs Speedup")
plt.xticks(threads)
plt.grid()
plt.savefig("graphs/threads_vs_speedup.png")
plt.close()

plt.figure()
plt.plot(threads, efficiency, marker="o")
plt.xlabel("Number of Threads")
plt.ylabel("Efficiency (%)")
plt.title("Threads vs Efficiency")
plt.xticks(threads)
plt.grid()
plt.savefig("graphs/threads_vs_efficiency.png")
plt.close()

print("Q2 graphs created successfully!")
