import csv
import math

FILE = "data/results/combined_results.csv"

data = {}

with open(FILE, newline="") as f:
    for row in csv.DictReader(f):
        language = row["language"]
        n = int(row["n"])

        data.setdefault(language, []).append({
            "n": n,
            "time": float(row["median_seconds"]),
            "memory": float(row["max_rss_mib"])
        })

for language in data:
    data[language].sort(key=lambda x: x["n"])

print("=== TIME GROWTH ===")

for language, rows in data.items():
    first = rows[0]
    last = rows[-1]

    time_factor = last["time"] / first["time"]

    print(
        f"{language}: "
        f"{first['time']:.6f}s -> {last['time']:.6f}s "
        f"({time_factor:.2f}x)"
    )

print()
print("=== MEMORY GROWTH ===")

for language, rows in data.items():
    first = rows[0]
    last = rows[-1]

    memory_factor = last["memory"] / first["memory"]

    print(
        f"{language}: "
        f"{first['memory']:.2f} MiB -> {last['memory']:.2f} MiB "
        f"({memory_factor:.2f}x)"
    )

print()
print("=== TIME COMPARISON AT n=500 ===")

for language, rows in data.items():
    row = next(r for r in rows if r["n"] == 500)
    print(f"{language}: {row['time']:.6f}s")

print()
print("=== MEMORY COMPARISON AT n=500 ===")

for language, rows in data.items():
    row = next(r for r in rows if r["n"] == 500)
    print(f"{language}: {row['memory']:.2f} MiB")
