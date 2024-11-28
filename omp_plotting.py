# Plot speedup and efficiency graphs for OpenMP results

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

n = input("Enter the value of n to plot: ")

# Load the data
data = pd.read_csv("results/results_omp.csv")

# Sort by code and n
data = data.sort_values(["code", "n"])

# Filter the data
filter = pow(2, int(n))
data = data[data["n"] == filter]


custom_code_order = ["O", "OR", "OB", "OBT", "OB_S", "OB_D", "OBf"]
xticks_values = [1, 2, 4, 8, 16, 32, 64]
color_map = {code: plt.cm.tab10(i) for i, code in enumerate(data["code"].unique())}

# Plot the speedup for symmetry
fig, ax = plt.subplots()
for code in custom_code_order:
    if code in data["code"].values:
        code_data = data[data["code"] == code]
        color = color_map[code]
        ax.plot(
            [1] + code_data["threads"].tolist(),
            [1] + code_data["speedup1"].tolist(),
            color=color,
            marker="o",
            linestyle="-",
            label=(code),
        )

ax.set_xlabel("Number of threads")
ax.set_ylabel("Speedup")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()

# Plot the efficiency for symmetry
fig, ax = plt.subplots()
for code in custom_code_order:
    if code in data["code"].values:
        code_data = data[data["code"] == code]
        color = color_map[code]
        ax.plot(
            [1] + code_data["threads"].tolist(),
            [100] + code_data["efficiency1"].tolist(),
            color=color,
            marker="o",
            linestyle="-",
            label=(code),
        )

ax.set_xlabel("Number of threads")
ax.set_ylabel("Efficiency")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()

# Plot the speedup for transpose
fig, ax = plt.subplots()
for code in custom_code_order:
    if code in data["code"].values:
        code_data = data[data["code"] == code]
        color = color_map[code]
        ax.plot(
            [1] + code_data["threads"].tolist(),
            [1] + code_data["speedup2"].tolist(),
            color=color,
            marker="o",
            linestyle="-",
            label=(code),
        )

ax.set_xlabel("Number of threads")
ax.set_ylabel("Speedup")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()

# Plot the efficiency for transpose
fig, ax = plt.subplots()
for code in custom_code_order:
    if code in data["code"].values:
        code_data = data[data["code"] == code]
        color = color_map[code]
        ax.plot(
            [1] + code_data["threads"].tolist(),
            [100] + code_data["efficiency2"].tolist(),
            color=color,
            marker="o",
            linestyle="-",
            label=(code),
        )

ax.set_xlabel("Number of threads")
ax.set_ylabel("Efficiency")
ax.set_xscale("log")
ax.legend()
ax.set_xticks(xticks_values)
ax.set_xticklabels([str(x) for x in xticks_values])
plt.grid(True, linestyle="--", alpha=0.7)
plt.tight_layout()
plt.show()