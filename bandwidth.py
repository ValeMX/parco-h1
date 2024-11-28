import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

n = input("Enter the value of n to plot: ")
data = pd.read_csv("results/results_omp.csv")

data = data.sort_values(["code", "n"])

filter = pow(2, int(n))
data = data[data["n"] == filter]

custom_code_order = ["O", "OR", "OB", "OBT", "OB_S", "OB_D", "OBf"]
xticks_values = [1, 2, 4, 8, 16, 32, 64]
color_map = {code: plt.cm.tab10(i) for i, code in enumerate(data["code"].unique())}

fig, ax = plt.subplots()

data["bandwidth"] = data["bandwidth"] * 1e-9

max_bandwidth = 0
max_point = (0, 0)

for code in custom_code_order:
    if code in data["code"].values:
        code_data = data[data["code"] == code]
        color = color_map[code]

        max_code_bandwidth = code_data["bandwidth"].max()
        if max_code_bandwidth > max_bandwidth:
            max_bandwidth = max_code_bandwidth
            max_threads = code_data.loc[code_data["bandwidth"].idxmax(), "threads"]
            max_point = (max_threads, max_bandwidth)

        ax.plot(
            code_data["threads"].tolist(),
            code_data["bandwidth"].tolist(),
            color=color,
            marker="o",
            linestyle="-",
            label=(code + " - S"),
        )

ax.text(
    max_point[0],
    max_point[1] + 10,
    f"{max_point[1]:.2f} GB/s",
    color="black",
    fontsize=10,
    ha="center",
)

peak_bandwidth = 281.568
ax.axhline(y=peak_bandwidth, color="red", linestyle="--")

ax.text(
    xticks_values[1],
    peak_bandwidth - 10,
    f"{peak_bandwidth} GB/s",
    color="red",
    fontsize=10,
    va="top",
    ha="center",
    fontweight="bold",
)

ax.set_xlabel("Number of threads")
ax.set_ylabel("Bandwidth (GB/s)")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()
