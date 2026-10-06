import csv
import statistics
import time
from pathlib import Path

from matrix_io import load_matrix
from matrix_multiplication import multiply


WARMUP_RUNS = 3
MEASURED_RUNS = 5

SIZES = [50, 100, 200, 300, 500]


def run_multiplication(A, B):
    C = multiply(A, B)

    checksum = sum(sum(row) for row in C)

    if checksum == float("inf"):
        raise RuntimeError("Invalid result.")

    return checksum


def benchmark(A, B):
    for _ in range(WARMUP_RUNS):
        run_multiplication(A, B)

    times = []

    for _ in range(MEASURED_RUNS):
        start = time.perf_counter()
        run_multiplication(A, B)
        end = time.perf_counter()

        times.append(end - start)

    return times


def main():
    output = Path("data/results/python_results.csv")
    output.parent.mkdir(parents=True, exist_ok=True)

    with open(output, "w", newline="") as f:
        writer = csv.writer(f)

        writer.writerow([
            "language",
            "n",
            "warmup_runs",
            "measured_runs",
            "median_seconds",
            "iqr_seconds"
        ])

        for n in SIZES:
            print(f"Running Python n={n}...")

            A = load_matrix(f"data/input/n{n}_A.csv")
            B = load_matrix(f"data/input/n{n}_B.csv")

            times = benchmark(A, B)

            median = statistics.median(times)
            q1 = statistics.quantiles(times, n=4)[0]
            q3 = statistics.quantiles(times, n=4)[2]
            iqr = q3 - q1

            writer.writerow([
                "Python",
                n,
                WARMUP_RUNS,
                MEASURED_RUNS,
                f"{median:.9f}",
                f"{iqr:.9f}"
            ])

            print(
                f"  median={median:.9f}s "
                f"IQR={iqr:.9f}s"
            )

    print(f"\nResults saved to {output}")


if __name__ == "__main__":
    main()
