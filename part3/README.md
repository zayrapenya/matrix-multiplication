# Assignment 3 – Parallel matrix multiplication (Big Data, ULPGC 2026-27)

Copy this folder into the repo as `part3/` (branch `assignment-3`).

## What is compared
- **Sequential baseline** `seq`: i–k–j dense kernel (best dense method of Assignment 2), `double`, row-major.
- **Parallel** `par`: *the same row kernel* (`rows_ikj`), rows of C split into `p` contiguous blocks, one POSIX thread per block (pthreads; no OpenMP/libomp needed on macOS). Each thread writes only its own rows of C -> no shared writes, no locks.
- `par` with p = 1 still spawns one worker thread, to expose framework overhead.
- No numerical library is used, so no hidden library threads.

## Timing / memory protocol
- Timed region: the multiplication kernel only (includes zeroing C; for `par` includes thread create + join). Inputs generated outside the timer.
- Monotonic clock (`CLOCK_MONOTONIC`), 2 warm-up runs (excluded), 7 measured repetitions per configuration, result consumed through a checksum.
- Inputs: `splitmix64`, values in [0,1), seed_A = 42 + 17n, seed_B = seed_A + 1000003 (same logical matrices for every method/thread count). NOTE: this generator is not guaranteed identical to the one in Assignment 2 – say so in the report.
- Memory: **peak RSS** (`getrusage ru_maxrss`), one process per configuration, so each value is the peak of that configuration only.
- Budget: each configuration is killed after `BUDGET_S` seconds (default 120); kills are logged in `results/failures.csv` (report them as results, no invented times).
- Statistics: median and IQR; S(p) = median Tseq / median Tp; E(p) = S(p)/p.

## Run (macOS / Linux, from the repo root)
```bash
cd part3
bash scripts/run_benchmarks.sh          # builds, runs correctness tests, then all benchmarks
python3 scripts/analyze_results.py      # summary.csv, speedup.csv, one_thread_overhead.csv, figures/
```
Quick smoke test: `SIZES="200 500" THREADS="1 2 4" REPS=5 bash scripts/run_benchmarks.sh`

Optional vectorisation study (Clang only): `VEC_STUDY=1 bash scripts/run_benchmarks.sh`
builds a second binary with `-fno-vectorize -fno-slp-vectorize` and saves the compiler's
vectorisation report in `results/vectorization_report.txt` (evidence).

## Before running (for a clean measurement)
Plug in the charger, close other apps, do not touch the machine during the run. The M1 has 4 performance + 4 efficiency cores and macOS offers no thread pinning, so: 1–4 threads run mostly on P-cores, >4 start using slower E-cores, and 12/16 threads are oversubscription (8 hardware contexts). Report this when you interpret the curves.

## Files
- `src/matrix_parallel.{c,h}` kernels · `src/benchmark.c` one-config benchmark · `src/test_parallel.c` correctness tests
- `scripts/run_benchmarks.sh`, `scripts/analyze_results.py`, `scripts/collect_environment.sh`
- Outputs: `results/raw_benchmarks.csv` (every measurement), `summary.csv`, `speedup.csv`, `environment.txt`, `test_output.txt`, `failures.csv`; `figures/`.
