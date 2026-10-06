import statistics
import sys
import time

from matrix_io import load_matrix
from matrix_multiplication import multiply


WARMUP_RUNS = 3
MEASURED_RUNS = 5


def run_multiplication(A, B):
    C = multiply(A, B)

    # Consume the result.
    checksum = sum(sum(row) for row in C)

    if checksum == float("inf"):
        raise RuntimeError("Invalid result.")

    return checksum


def benchmark(A, B):
    # Warm-up runs are not included in the measurements.
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
    if len(sys.argv) != 2:
        print("Usage: python3 python/benchmark.py <n>")
        sys.exit(1)

    n = int(sys.argv[1])

    A = load_matrix(f"data/input/n{n}_A.csv")
    B = load_matrix(f"data/input/n{n}_B.csv")

    times = benchmark(A, B)

    median = statistics.median(times)
    q1 = statistics.quantiles(times, n=4)[0]
    q3 = statistics.quantiles(times, n=4)[2]
    iqr = q3 - q1

    print(f"n={n}")
    print(f"warmup_runs={WARMUP_RUNS}")
    print(f"measured_runs={MEASURED_RUNS}")
    print("times_seconds=" + ",".join(f"{t:.9f}" for t in times))
    print(f"median_seconds={median:.9f}")
    print(f"iqr_seconds={iqr:.9f}")


if __name__ == "__main__":
    main()
