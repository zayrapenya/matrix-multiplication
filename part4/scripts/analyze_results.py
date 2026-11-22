#!/usr/bin/env python3
"""results/raw_benchmarks.csv + memory.csv -> summary.csv, scaling.csv, figures/."""
import os
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.join(os.path.dirname(__file__), "..")
R = lambda *p: os.path.join(ROOT, *p)
raw = pd.read_csv(R("results", "raw_benchmarks.csv"))
os.makedirs(R("figures"), exist_ok=True)

def iqr(s): return s.quantile(0.75) - s.quantile(0.25)

cols = ["end_to_end_s", "upload_s", "compute_s", "assemble_s", "max_fetch_s", "max_kernel_s", "mean_kernel_s"]
agg = {"reps": ("end_to_end_s", "size"), "iqr_end_to_end_s": ("end_to_end_s", iqr)}
for c in cols: agg["median_" + c] = (c, "median")
s = raw.groupby(["mode", "n", "workers"]).agg(**agg).reset_index()
if os.path.exists(R("results", "memory.csv")):
    s = s.merge(pd.read_csv(R("results", "memory.csv")), on=["mode", "n", "workers"], how="left")
s.to_csv(R("results", "summary.csv"), index=False)

seq = s[s["mode"] == "seq"].set_index("n")["median_end_to_end_s"]
par = s[s["mode"] == "par"]
dist = s[s["mode"] == "dist"].copy()
dist["t_seq_s"] = dist["n"].map(seq)
d1 = dist[dist.workers == 1].set_index("n")["median_end_to_end_s"]
dist["speedup_vs_seq_end_to_end"] = dist["t_seq_s"] / dist["median_end_to_end_s"]
dist["speedup_vs_seq_compute_phase"] = dist["t_seq_s"] / dist["median_compute_s"]
dist["speedup_vs_1worker"] = dist["n"].map(d1) / dist["median_end_to_end_s"]      # strong scaling, same framework
dist["efficiency_vs_1worker"] = dist["speedup_vs_1worker"] / dist["workers"]
# analytic payload estimate (computed from sizes, NOT measured): B upload + A out + C back + B read by every task
dist["payload_mib_estimate"] = (3 + dist["workers"]) * 8 * dist["n"] ** 2 / 2 ** 20
dist.to_csv(R("results", "scaling.csv"), index=False)

# cross-language check against Assignment 3 (C): same inputs -> same checksum up to rounding
p3 = os.path.join(ROOT, "..", "part3", "results", "raw_benchmarks.csv")
if os.path.exists(p3):
    c = pd.read_csv(p3); c = c[c.method == "seq"].groupby("n").checksum.first()
    j = raw[raw["mode"] == "seq"].groupby("n").checksum.first()
    chk = pd.DataFrame({"java": j, "c_part3": c}).dropna()
    chk["rel_diff"] = (chk.java - chk.c_part3).abs() / chk.c_part3.abs()
    chk.to_csv(R("results", "checksum_crosscheck_vs_part3.csv"))
    print("checksum cross-check vs Assignment 3 (relative difference):\n", chk.rel_diff.to_string())

def save(name): plt.tight_layout(); plt.savefig(R("figures", name), dpi=150); plt.close()

# 1 end-to-end time vs workers
plt.figure(figsize=(7, 4.5))
for n, g in dist.groupby("n"):
    plt.plot(g.workers, g.median_end_to_end_s, marker="o", label=f"n={n}, distributed end-to-end")
    if n in seq.index: plt.axhline(seq[n], ls=":", lw=1, color=plt.gca().lines[-1].get_color())
plt.xscale("log", base=2); plt.yscale("log"); plt.xlabel("workers (local member JVMs)"); plt.ylabel("median time (s)")
plt.grid(alpha=.3); plt.legend(fontsize=8); plt.title("Distributed end-to-end time (dotted: Java sequential kernel)")
save("dist_time.png")

# 2 speedup vs 1 worker (strong scaling)
plt.figure(figsize=(7, 4.5))
for n, g in dist.groupby("n"): plt.plot(g.workers, g.speedup_vs_1worker, marker="o", label=f"n={n}")
ws = sorted(dist.workers.unique()); plt.plot(ws, ws, "k--", label="ideal")
plt.xlabel("workers"); plt.ylabel("speedup vs 1 worker (end-to-end)"); plt.grid(alpha=.3); plt.legend(); save("dist_speedup.png")

# 3 phases at the driver (consecutive, so stacking is valid) for the largest n
nmax = dist.n.max(); g = dist[dist.n == nmax].sort_values("workers")
plt.figure(figsize=(7, 4.5)); x = np.arange(len(g))
plt.bar(x, g.median_upload_s, label="upload B"); 
plt.bar(x, g.median_compute_s, bottom=g.median_upload_s, label="compute phase (A out, B read, kernel, C back)")
plt.bar(x, g.median_assemble_s, bottom=g.median_upload_s + g.median_compute_s, label="assemble C")
plt.plot(x, g.median_max_kernel_s, "k_", markersize=18, mew=2, label="slowest in-task kernel (overlaps compute phase)")
plt.xticks(x, g.workers); plt.xlabel("workers"); plt.ylabel("median time (s)"); plt.title(f"Driver-side phases, n={nmax}")
plt.legend(fontsize=7); save("dist_phases.png")

print(dist[["n", "workers", "median_end_to_end_s", "iqr_end_to_end_s", "speedup_vs_seq_end_to_end",
            "speedup_vs_1worker", "efficiency_vs_1worker"]].round(4).to_string(index=False))
