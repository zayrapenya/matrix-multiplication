# Assignment 2 — Matrix Multiplication

## Contents

- `src/matrix_methods.c`: dense and CSR implementations.
- `src/matrix_methods.h`: declarations and CSR data structure.
- `src/test_methods.c`: correctness tests.
- `src/benchmark.c`: benchmark executable source.
- `scripts/run_benchmarks.sh`: benchmark suite.
- `scripts/analyze_results.py`: statistics and figure generation.
- `results/raw_benchmarks.csv`: raw timing measurements.
- `results/summary.csv`: medians, quartiles and IQR.
- `results/analysis_notes.txt`: generated comparison notes.
- `figures/`: generated plots.
- `report.md`: experiment report.

## Requirements

- Clang or another compatible C compiler.
- Python 3.
- Matplotlib for generating plots.

## Setup

From the repository root:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install matplotlib
```

## Compile and test

Compile the correctness tests:

```bash
clang -O3 -Wall -Wextra part2/src/matrix_methods.c part2/src/test_methods.c -o part2/test_methods
./part2/test_methods
```

Compile the benchmark:

```bash
clang -O3 -Wall -Wextra part2/src/matrix_methods.c part2/src/benchmark.c -o part2/benchmark
```

## Run benchmarks

```bash
bash part2/scripts/run_benchmarks.sh
```

## Analyze results and generate figures

```bash
python part2/scripts/analyze_results.py
```

## Experimental notes

The implementations use double-precision values and sequential execution. Dense implementations are compared using the same generated inputs. The sparse implementation uses CSR (Compressed Sparse Row) representation.

Benchmark results include repeated measurements. The analysis reports medians and interquartile ranges (IQR). Sparse conversion time is measured separately and included when calculating total sparse time.

The memory measurements are samples of resident memory, not guaranteed peak-memory measurements. Results depend on the machine, operating system, compiler and execution environment.

See `report.md` for the experimental results, comparisons, limitations and conclusions.
