import sys
import time

from matrix_io import load_matrix
from matrix_multiplication import multiply

WARMUP_RUNS = 3

def run_multiplication(A, B):
    C = multiply(A, B)

    checksum = sum(sum(row) for row in C)

    if checksum == float("inf"):
        raise RuntimeError("Invalid result.")

def main():
    if len(sys.argv) != 2:
        print("Usage: python3 python/memory_benchmark.py <n>")
        sys.exit(1)

    n = int(sys.argv[1])

    A = load_matrix(f"data/input/n{n}_A.csv")
    B = load_matrix(f"data/input/n{n}_B.csv")

    for _ in range(WARMUP_RUNS):
        run_multiplication(A, B)

    start = time.perf_counter()
    run_multiplication(A, B)
    end = time.perf_counter()

    print(f"n={n}")
    print(f"time_seconds={end - start:.9f}")

if __name__ == "__main__":
    main()
