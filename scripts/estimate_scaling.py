import csv
import math

FILE = "data/results/combined_results.csv"

data = {}

with open(FILE, newline="") as f:
    for row in csv.DictReader(f):
        language = row["language"]
        data.setdefault(language, []).append({
            "n": int(row["n"]),
            "time": float(row["median_seconds"]),
            "memory": float(row["max_rss_mib"])
        })

print("=== OBSERVED SCALING EXPONENTS ===")

for language, rows in data.items():
    rows.sort(key=lambda x: x["n"])

    first = rows[0]
    last = rows[-1]

    time_exponent = (
        math.log(last["time"] / first["time"])
        / math.log(last["n"] / first["n"])
    )

    memory_exponent = (
        math.log(last["memory"] / first["memory"])
        / math.log(last["n"] / first["n"])
    )

    print(
        f"{language}: "
        f"time exponent = {time_exponent:.3f}, "
        f"memory exponent = {memory_exponent:.3f}"
    )
