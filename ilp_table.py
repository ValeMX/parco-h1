import pandas as pd
from tabulate import tabulate

file_path = "results/results_ilp.csv"
data = pd.read_csv(file_path)

data["flops"] = data["flops"] * 1e-9
data["bandwidth"] = data["bandwidth"] * 1e-9

data["flops"] = data["flops"].apply(lambda x: round(x, 3))
data["bandwidth"] = data["bandwidth"].apply(lambda x: round(x, 3))

code_order = ["S", "V", "B", "BP", "BO1", "BO2", "BO3", "BOf"]
data["code"] = pd.Categorical(data["code"], categories=code_order, ordered=True)
data = data.sort_values(by=["code", "n"])

pivot_flops = data.pivot(index="code", columns="n", values="flops")
pivot_bandwidth = data.pivot(index="code", columns="n", values="bandwidth")

print("FLOPS (GFLOPS) FROM EXPERIMENTS")
print(tabulate(pivot_flops, headers="keys", tablefmt="grid", floatfmt=".3f"))

print("\nBANDWIDTH (GB/s) FROM EXPERIMENTS")
print(tabulate(pivot_bandwidth, headers="keys", tablefmt="grid", floatfmt=".3f"))