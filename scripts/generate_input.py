import csv
import random
import sys
from pathlib import Path


SEED = 42
LOW = -1.0
HIGH = 1.0


def generate_matrix(n, rng):
    return [[rng.uniform(LOW, HIGH) for _ in range(n)] for _ in range(n)]


def save_matrix(matrix, path):
    with open(path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerows(matrix)


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 scripts/generate_input.py <n>")
        sys.exit(1)

    n = int(sys.argv[1])

    if n <= 0:
        print("Matrix size must be positive.")
        sys.exit(1)

    output_dir = Path("data/input")
    output_dir.mkdir(parents=True, exist_ok=True)

    rng = random.Random(SEED)

    A = generate_matrix(n, rng)
    B = generate_matrix(n, rng)

    save_matrix(A, output_dir / f"n{n}_A.csv")
    save_matrix(B, output_dir / f"n{n}_B.csv")

    print(f"Generated matrices of size {n}x{n}")
    print(f"Seed: {SEED}")
    print(f"Range: [{LOW}, {HIGH}]")
    print(f"Output: {output_dir}/n{n}_A.csv")
    print(f"Output: {output_dir}/n{n}_B.csv")


if __name__ == "__main__":
    main()
