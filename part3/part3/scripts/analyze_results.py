#!/usr/bin/env python3
"""Reads results/raw_benchmarks.csv -> results/summary.csv, results/speedup.csv and figures/."""
import os
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.join(os.path.dirname(__file__), "..")
raw = pd.read_csv(os.path.join(ROOT, "results", "raw_benchmarks.csv"))
os.makedirs(os.path.join(ROOT, "figures"), exist_ok=True)

def iqr(s): return s.quantile(0.75) - s.quantile(0.25)

g = raw.groupby(["build", "method", "n", "threads"])
summary = g.agg(reps=("time_s", "size"), median_s=("time_s", "median"), iqr_s=("time_s", iqr),
                min_s=("time_s", "min"), max_s=("time_s", "max"),
                peak_rss_mib=("peak_rss_kib", lambda s: s.max() / 1024)).reset_index()
summary.to_csv(os.path.join(ROOT, "results", "summary.csv"), index=False)

main = summary[summary.build == "autovec"]
seq = main[main.method == "seq"].set_index("n")["median_s"]
par = main[main.method == "par"].copy()
par["t_seq_s"] = par["n"].map(seq)
par["speedup"] = par["t_seq_s"] / par["median_s"]           # S(p) = Tseq / Tp (medians)
par["efficiency"] = par["speedup"] / par["threads"]          # E(p) = S(p) / p
par.to_csv(os.path.join(ROOT, "results", "speedup.csv"), index=False)

def lineplot(col, ylabel, fname, ideal=False, logy=False):
    plt.figure(figsize=(7, 4.5))
    for n, d in par.groupby("n"):
        plt.plot(d.threads, d[col], marker="o", label=f"n={n}")
    if ideal:
        ps = sorted(par.threads.unique()); plt.plot(ps, ps, "k--", label="ideal")
    plt.xlabel("threads p"); plt.ylabel(ylabel); plt.grid(alpha=.3); plt.legend()
    if logy: plt.yscale("log")
    plt.tight_layout(); plt.savefig(os.path.join(ROOT, "figures", fname), dpi=150); plt.close()

lineplot("speedup", "speedup S(p)", "speedup.png", ideal=True)
lineplot("efficiency", "efficiency E(p)", "efficiency.png")
lineplot("median_s", "median time (s)", "runtime.png", logy=True)

# overhead of the parallel code path with 1 thread vs. the sequential baseline
ov = par[par.threads == 1][["n", "median_s", "t_seq_s"]].copy()
ov["overhead_pct"] = (ov.median_s / ov.t_seq_s - 1) * 100
ov.to_csv(os.path.join(ROOT, "results", "one_thread_overhead.csv"), index=False)

# optional vectorisation study
if (summary.build == "novec").any():
    v = summary[summary.build.isin(["novec", "autovec", "autovec_vecstudy"])].copy()
    v["label"] = v.build.replace({"autovec_vecstudy": "autovec"})
    v = v.drop_duplicates(["label", "method", "n", "threads"], keep="last")
    v = v[v.n.isin(summary[summary.build == "novec"].n.unique())]
    piv = v.pivot_table(index=["n", "method", "threads"], columns="label", values="median_s")
    piv["vec_speedup"] = piv["novec"] / piv["autovec"]
    piv.reset_index().to_csv(os.path.join(ROOT, "results", "vectorization_summary.csv"), index=False)

print(par[["n", "threads", "median_s", "speedup", "efficiency"]].round(4).to_string(index=False))
