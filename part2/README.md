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

## Detailed experimental protocol

### Input generation

The benchmark uses the C standard library pseudo-random number generator
(`rand()`), initialised with the deterministic seed
`42 + n * 17 + zero_pct`, where `n` is the matrix dimension and
`zero_pct` is the nominal percentage of zero entries.

Each matrix element is set to zero when `rand() % 100 < zero_pct`.
Otherwise, it is generated as `(rand() % 2001 - 1000) / 1000.0`,
giving nonzero values in the range [-1.000, 1.000] in increments of
0.001. The requested zero percentage is nominal; the actual number of
nonzero entries is recorded in the raw measurements.

### Timing and warm-up

Elapsed time is measured with `clock_gettime(CLOCK_MONOTONIC)`.
Matrix generation and allocation take place outside the timed
multiplication region. CSR conversion is timed separately. Each
configuration performs one warm-up multiplication, excluded from the
reported measurements, followed by five measured repetitions.
The output is consumed by calculating a checksum.

### Memory measurement

On macOS, the benchmark calls `task_info` with `MACH_TASK_BASIC_INFO`
and records `resident_size` in bytes in each repetition. This is a
sample of the process resident memory at the time of the call, not a
peak resident-set-size measurement. It may include memory beyond the
matrix arrays and does not establish the maximum memory used during
execution.

### Execution limits

The original benchmark script does not enforce a hard timeout or a
predefined peak-memory limit. Therefore, the existing results must not
be described as having passed such limits. The largest tested dimension
was 800. No claim is made that this is the largest dimension that would
meet a predefined time or peak-memory budget.

### Post-run resource-budget check

The existing benchmark results were inspected after execution against a
10-second per-multiplication timing budget. All 220 recorded multiplication
measurements were below 10 seconds. The maximum was 0.608907 seconds for
the basic method at dimension 800, with a nominal zero percentage of 0%.

This was a post-run check, not an enforced timeout during the original
benchmark runs. Therefore, the original measurements must not be described
as having been executed under a hard timeout. Likewise, resident-memory
values are samples recorded during execution, not enforced memory limits
or guaranteed peak-memory measurements.
