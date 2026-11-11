import csv
import statistics
from collections import defaultdict
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[2]
INPUT = ROOT / "part2/results/raw_benchmarks.csv"
OUT = ROOT / "part2/results"
FIG = ROOT / "part2/figures"
OUT.mkdir(parents=True, exist_ok=True)
FIG.mkdir(parents=True, exist_ok=True)

with INPUT.open(newline="") as f:
    rows = list(csv.DictReader(f))

if not rows:
    raise SystemExit("ERROR: el CSV no contiene mediciones.")

for r in rows:
    for k in ("n", "zero_pct", "rep", "nnz_A", "nnz_B", "nnz_C"):
        r[k] = int(r[k])
    for k in ("elapsed_seconds", "conversion_seconds",
              "resident_memory_bytes", "checksum"):
        r[k] = float(r[k])

groups = defaultdict(list)
for r in rows:
    groups[(r["method"], r["n"], r["zero_pct"])].append(r)

def median(key, data):
    return statistics.median(x[key] for x in data)

def quartile(values, fraction):
    values = sorted(values)
    if len(values) == 1:
        return values[0]
    pos = (len(values) - 1) * fraction
    lo = int(pos)
    hi = min(lo + 1, len(values) - 1)
    return values[lo] + (values[hi] - values[lo]) * (pos - lo)

summary = []
for (method, n, zero_pct), data in sorted(groups.items()):
    times = [x["elapsed_seconds"] for x in data]
    q1, q3 = quartile(times, .25), quartile(times, .75)
    conv = median("conversion_seconds", data)
    med = statistics.median(times)
    summary.append({
        "method": method,
        "n": n,
        "zero_pct": zero_pct,
        "repetitions": len(data),
        "median_seconds": med,
        "q1_seconds": q1,
        "q3_seconds": q3,
        "iqr_seconds": q3 - q1,
        "median_conversion_seconds": conv,
        "median_total_seconds": med + conv,
        "median_resident_memory_bytes": median("resident_memory_bytes", data),
        "nnz_A": data[0]["nnz_A"],
        "nnz_B": data[0]["nnz_B"],
        "median_nnz_C": median("nnz_C", data),
    })

summary_path = OUT / "summary.csv"
with summary_path.open("w", newline="") as f:
    writer = csv.DictWriter(f, fieldnames=summary[0].keys(), lineterminator="\n")
    writer.writeheader()
    writer.writerows(summary)

index = {(r["method"], r["n"], r["zero_pct"]): r for r in summary}

# Figure 1: dense methods, input matrices without deliberately introduced zeros.
dense_methods = ["basic", "reordered", "blocked"]
plt.figure(figsize=(8, 5))
for method in dense_methods:
    data = sorted(
        [r for r in summary if r["method"] == method and r["zero_pct"] == 0],
        key=lambda r: r["n"]
    )
    if data:
        plt.plot([r["n"] for r in data],
                 [r["median_seconds"] for r in data],
                 marker="o", label=method)
plt.xlabel("Matrix dimension n")
plt.ylabel("Median multiplication time (seconds)")
plt.title("Dense matrix multiplication performance")
plt.grid(True, alpha=.3)
plt.legend()
plt.tight_layout()
plt.savefig(FIG / "dense_runtime.png", dpi=180)
plt.close()

# Figure 2: CSR speedup over dense baseline, including CSR conversion.
plt.figure(figsize=(8, 5))
for zero_pct in (50, 90, 95, 99):
    xs, ys = [], []
    for n in sorted({r["n"] for r in summary}):
        dense = index.get(("basic", n, zero_pct))
        sparse = index.get(("csr", n, zero_pct))
        if dense and sparse and sparse["median_total_seconds"] > 0:
            xs.append(n)
            ys.append(dense["median_seconds"] /
                      sparse["median_total_seconds"])
    if xs:
        plt.plot(xs, ys, marker="o", label=f"{zero_pct}% zeros")
plt.axhline(1.0, linestyle="--", linewidth=1, color="black")
plt.xlabel("Matrix dimension n")
plt.ylabel("Speedup (dense baseline / CSR total time)")
plt.title("CSR performance including conversion cost")
plt.grid(True, alpha=.3)
plt.legend()
plt.tight_layout()
plt.savefig(FIG / "sparse_speedup.png", dpi=180)
plt.close()

# Figure 3: sampled resident memory for dense methods.
plt.figure(figsize=(8, 5))
for method in dense_methods:
    data = sorted(
        [r for r in summary if r["method"] == method and r["zero_pct"] == 0],
        key=lambda r: r["n"]
    )
    if data:
        plt.plot([r["n"] for r in data],
                 [r["median_resident_memory_bytes"] / (1024 * 1024)
                  for r in data],
                 marker="o", label=method)
plt.xlabel("Matrix dimension n")
plt.ylabel("Median sampled resident memory (MiB)")
plt.title("Sampled resident memory during dense benchmarks")
plt.grid(True, alpha=.3)
plt.legend()
plt.tight_layout()
plt.savefig(FIG / "dense_memory.png", dpi=180)
plt.close()

# Check that methods return matching checksums for the same input configuration.
checks = defaultdict(list)
for r in rows:
    checks[(r["n"], r["zero_pct"])].append(
        (r["method"], r["checksum"])
    )

notes = []
notes.append(f"Input measurements: {len(rows)}")
notes.append(f"Summary groups: {len(summary)}")
notes.append("")
notes.append("Dense medians (zero_pct=0):")
for n in sorted({r["n"] for r in summary}):
    for method in dense_methods:
        r = index.get((method, n, 0))
        if r:
            notes.append(
                f"n={n}, {method}: median={r['median_seconds']:.9f}s, "
                f"IQR={r['iqr_seconds']:.9f}s"
            )
notes.append("")
notes.append("CSR comparison (conversion included in total):")
for zero_pct in (50, 90, 95, 99):
    for n in sorted({r["n"] for r in summary}):
        dense = index.get(("basic", n, zero_pct))
        sparse = index.get(("csr", n, zero_pct))
        if dense and sparse:
            speedup = dense["median_seconds"] / sparse["median_total_seconds"]
            notes.append(
                f"n={n}, zeros={zero_pct}%: "
                f"dense={dense['median_seconds']:.9f}s, "
                f"CSR multiply={sparse['median_seconds']:.9f}s, "
                f"conversion={sparse['median_conversion_seconds']:.9f}s, "
                f"CSR total={sparse['median_total_seconds']:.9f}s, "
                f"speedup={speedup:.3f}x"
            )
notes.append("")
notes.append("Checksum comparisons (spread across methods/repetitions):")
for key, values in sorted(checks.items()):
    vals = [v for _, v in values]
    if vals and max(vals) - min(vals) > 1e-7:
        notes.append(f"WARNING n={key[0]}, zeros={key[1]}%: checksums differ "
                     f"(range {min(vals):.12g} to {max(vals):.12g})")
if not any(line.startswith("WARNING") for line in notes):
    notes.append("No checksum differences detected within tolerance 1e-7.")
notes.append("")
notes.append(
    "Memory values are sampled resident memory, not peak RSS. "
    "CSR conversion time is recorded separately and added to multiplication "
    "time for end-to-end comparisons."
)

notes_path = OUT / "analysis_notes.txt"
notes_path.write_text("\n".join(notes) + "\n")

print(f"Resumen: {summary_path}")
print(f"Notas: {notes_path}")
print("Gráficas:")
for name in ("dense_runtime.png", "sparse_speedup.png", "dense_memory.png"):
    print(f"  {FIG / name}")
print("\n" + "\n".join(notes))
