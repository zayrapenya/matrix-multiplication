# BIG DATA — Basic Matrix Multiplication in Different Languages

**Academic Year:** 2026–2027  
**Degree:** Data Science and Engineering  
**University:** Universidad de Las Palmas de Gran Canaria

## Overview

This project studies how matrix size affects the execution time and memory usage of basic dense matrix multiplication implemented in Python, Java and C.

The same algorithm, input data and experimental protocol are used across the three languages to support a fair comparison.

## Research Question

How does matrix size affect the execution time and memory usage of basic triple-loop matrix multiplication implemented in Python, Java and C?

## Algorithm

All implementations use the standard triple-loop matrix multiplication algorithm:

```text
for i
    for j
        for k
            C[i][j] += A[i][k] * B[k][j]
```

For two \(n \times n\) matrices:

- **Time complexity:** \(O(n^3)\)
- **Matrix storage complexity:** \(O(n^2)\)

No optimized matrix multiplication libraries are used.

## Implementations

The project contains equivalent implementations in Python, Java and C. All use double-precision floating-point values and the same `i-j-k` loop order.

The implementations use different storage mechanisms:

- **Python:** nested lists.
- **Java:** two-dimensional `double` arrays.
- **C:** contiguous row-major memory allocation.

These language and storage differences may affect execution time and process memory usage.

The C implementation is compiled with Apple Clang using the `-O2` optimization flag.

## Experimental Setup

The experiments were performed on the following system:

| Component | Configuration |
|---|---|
| Operating system | macOS 14.6.1 |
| Processor | Apple M1 |
| RAM | 8 GB |
| Python | 3.14.7 |
| Java runtime | OpenJDK 25.0.2 |
| C compiler | Apple Clang 15.0.0 |
| C compilation flags | `-O2 -Wall -Wextra -std=c17` |

## Input Data

The input matrices are generated using a fixed random seed (`42`) and values uniformly distributed in the interval \([-1,1]\).

The tested matrix sizes are:

`n = 50, 100, 200, 300, 500`

The same generated input files are used by all three implementations. Input generation and loading are excluded from the measured execution interval.

Input data can be regenerated using:

```bash
python3 scripts/generate_input.py
```

## Timing Protocol

For each language and matrix size, the protocol uses:

- 3 warm-up runs.
- 5 measured runs.
- High-resolution monotonic timing.
- Median execution time as the central estimate.
- Interquartile range (IQR) as a measure of variability.

The measured interval includes the multiplication, output allocation and initialization, and checksum consumption. Input generation and loading are excluded.

The Java implementation uses warm-up runs to reduce the influence of JVM just-in-time (JIT) compilation on the measured results.

Raw timing measurements are preserved in `data/results/raw/`.

## Memory Measurement Protocol

Memory usage is measured on macOS using:

```bash
/usr/bin/time -l
```

The reported metric is the maximum resident set size (RSS) of the complete process. Values are converted from bytes to mebibytes (MiB).

These measurements include runtime overhead from the Python interpreter, the Java Virtual Machine and the C process. They do not represent only the memory occupied by the input and output matrices.

Consequently, process-level RSS should not be interpreted as a direct measurement of the algorithm's theoretical matrix-storage complexity.

## Results

### Execution Time

The following table reports median execution time in seconds.

| Matrix size | Python (s) | Java (s) | C (s) |
|---:|---:|---:|---:|
| 50 | 0.007852 | 0.000470 | 0.000249 |
| 100 | 0.042107 | 0.003656 | 0.001739 |
| 200 | 0.339358 | 0.007212 | 0.006816 |
| 300 | 1.251765 | 0.023826 | 0.025437 |
| 500 | 6.572772 | 0.116233 | 0.135725 |

Python has substantially higher execution times than Java and C for all tested sizes. Java and C are much closer to each other, and their relative performance varies with matrix size. At \(n=500\), Java is slightly faster than C in this experimental environment.

Execution time increases considerably as matrix size grows, consistent with the computational cost of triple-loop matrix multiplication.

### Memory Usage

The following table reports maximum process RSS in MiB.

| Matrix size | Python (MiB) | Java (MiB) | C (MiB) |
|---:|---:|---:|---:|
| 50 | 11.97 | 45.38 | 1.02 |
| 100 | 13.05 | 56.11 | 1.45 |
| 200 | 18.19 | 81.27 | 2.36 |
| 300 | 24.73 | 98.50 | 4.55 |
| 500 | 45.52 | 129.44 | 10.75 |

Java has the highest measured process RSS, reflecting the memory overhead of the JVM. Python has intermediate RSS, while C has the lowest RSS for all tested matrix sizes.

These values include runtime and process overhead and therefore do not measure the memory required exclusively by the matrices.

## Scaling Analysis

The observed scaling exponents are estimated from the measurements at \(n=50\) and \(n=500\), using:

\[
p = \frac{\log(T_{500}/T_{50})}
{\log(500/50)}
\]

where \(T_n\) denotes the median execution time for matrix size \(n\). The same calculation is applied to RSS values for the memory exponent.

### Execution-time exponents

| Language | Observed time exponent |
|---|---:|
| Python | 2.923 |
| Java | 2.393 |
| C | 2.736 |

Python and C show observed time exponents relatively close to the theoretical cubic complexity, \(O(n^3)\). Java exhibits a lower exponent over the tested range.

These values are empirical estimates, not proofs of asymptotic complexity. Runtime effects, measurement variability and the limited range of tested matrix sizes can influence the estimates.

### Memory exponents

| Language | Observed RSS exponent |
|---|---:|
| Python | 0.580 |
| Java | 0.455 |
| C | 1.025 |

The observed RSS exponents are lower than the theoretical \(O(n^2)\) matrix-storage exponent because RSS includes runtime and process overhead that does not scale directly with matrix size.

The theoretical \(O(n^2)\) storage complexity remains the appropriate model for storing the matrices themselves.

## Correctness

Correctness was checked using:

- Known small matrices with expected results.
- Identity matrices.
- Zero matrices.
- Generated input data.

The implementations were tested against expected results using a numerical tolerance of \(10^{-9}\) for absolute and relative comparisons.

All three implementations passed the correctness checks.

## Project Structure

```text
matrix-multiplication/
├── python/
│   ├── matrix_multiplication.py
│   ├── correctness.py
│   ├── matrix_io.py
│   ├── test_real_data.py
│   ├── benchmark.py
│   ├── run_benchmark.py
│   └── memory_benchmark.py
├── java/
│   ├── MatrixMultiplication.java
│   ├── MatrixIO.java
│   ├── CorrectnessTest.java
│   ├── TestRealData.java
│   ├── Benchmark.java
│   └── MemoryBenchmark.java
├── c/
│   ├── matrix_multiplication.c
│   ├── matrix_io.c
│   ├── matrix_io.h
│   ├── correctness.c
│   ├── benchmark.c
│   ├── test_real_data.c
│   └── memory_benchmark.c
├── data/
│   ├── input/
│   └── results/
│       ├── raw/
│       └── memory/
├── scripts/
│   ├── generate_input.py
│   ├── analyze_results.py
│   ├── analyze_numbers.py
│   └── estimate_scaling.py
└── report/
    └── figures/
```

## Figures

The generated figures are available in `report/figures/`:

- `execution_time.png`
- `execution_time_log.png`
- `memory_usage.png`
- `memory_usage_log.png`

## Reproducibility

The repository preserves the source code, generated input data, raw measurements, processed results, analysis scripts and figures.

To regenerate the input data:

```bash
python3 scripts/generate_input.py
```

The processed result files are stored in `data/results/`:

- `python_results.csv`
- `java_results.csv`
- `c_results.csv`
- `combined_results.csv`

Raw timing measurements are stored in `data/results/raw/`, while memory measurements are stored in `data/results/memory/`.

The scaling exponents can be recalculated using:

```bash
python3 scripts/estimate_scaling.py
```

The repository is available at:

https://github.com/zayrapenya/matrix-multiplication

The reported benchmark results correspond to the data currently stored in `data/results/combined_results.csv`.

## Limitations

The experiments cover five matrix sizes, from \(n=50\) to \(n=500\), on a single computer and operating system. The conclusions should therefore be restricted to the tested environment.

The measurements do not establish that one language will always be faster or use less memory on other systems, runtime versions or workloads. Process-level RSS includes runtime overhead and should not be treated as the exclusive memory consumption of the matrices.

The observed scaling exponents are estimates over a limited range and may be affected by runtime behaviour, cache effects, measurement variability and fixed process overhead.

## Conclusion

The experiment shows that matrix size strongly affects the execution time of basic triple-loop matrix multiplication. Python has the highest execution times, while Java and C perform considerably better in the tested environment.

Process-level RSS also differs substantially between implementations. C has the lowest measured RSS, Python is intermediate, and Java has the highest RSS due to JVM overhead.

Overall, the results illustrate how programming language, runtime and memory representation can influence the practical performance of the same algorithm. The conclusions apply to the tested implementations and experimental environment.
