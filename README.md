# Matrix Multiplication — Assignment 1

**BIG DATA — Academic Year 2026–2027**
**Degree in Data Science and Engineering**
**Universidad de Las Palmas de Gran Canaria**

## Overview

This project studies how matrix size affects the **execution time** and **memory usage** of a basic dense matrix multiplication implemented in **Python, Java and C**.

The same algorithm and experimental protocol are used across the three languages to allow a fair comparison.

## Research Question

> How does matrix size affect the execution time and memory usage of the basic triple-loop matrix multiplication implemented in Python, Java and C?

## Algorithm

All implementations use the standard triple-loop matrix multiplication:

```text
for i
    for j
        for k
            C[i][j] += A[i][k] * B[k][j]
```

For two `n × n` matrices:

* **Time complexity:** O(n³)
* **Matrix storage:** O(n²)

No optimized matrix multiplication libraries are used.

## Implementations

The project contains equivalent implementations in:

* **Python**
* **Java**
* **C**

All implementations use double-precision floating-point values and the same `i-j-k` loop order.

The C implementation is compiled with `clang -O2`.

## Experimental Setup

The experiments were performed on:

* macOS 14.6.1
* Apple M1
* 8 GB RAM
* Python 3.14.7
* OpenJDK 25.0.2
* Apple Clang 15.0.0

### Input data

Matrices are generated using a fixed random seed (`42`) with values in `[-1, 1]`.

The tested matrix sizes are:

```text
n = 50, 100, 200, 300, 500
```

The same generated input matrices are used by all three implementations.

### Timing protocol

For each matrix size:

* 3 warm-up runs
* 5 measured runs
* High-resolution monotonic timing
* Matrix generation/loading excluded from the measured interval
* Median execution time reported
* IQR reported as a measure of variability

The Java implementation uses warm-up runs to reduce the effect of JVM JIT compilation.

### Memory protocol

Memory usage is measured using macOS:

```text
/usr/bin/time -l
```

The reported metric is the **maximum resident set size (RSS)** of the complete process.

Therefore, the memory measurements include runtime overhead from the Python interpreter, Java Virtual Machine and C process, rather than measuring only the matrices themselves.

## Results

### Execution time

|   n | Python (s) | Java (s) |    C (s) |
| --: | ---------: | -------: | -------: |
|  50 |   0.006658 | 0.000470 | 0.000160 |
| 100 |   0.051679 | 0.003637 | 0.001756 |
| 200 |   0.419857 | 0.007103 | 0.006710 |
| 300 |   1.463930 | 0.023874 | 0.025850 |
| 500 |   7.216606 | 0.114524 | 0.136088 |

At `n = 500`, Python is substantially slower than Java and C. Java and C have similar performance, with Java being slightly faster in this particular experimental environment.

### Memory usage

|   n | Python (MiB) | Java (MiB) | C (MiB) |
| --: | -----------: | ---------: | ------: |
|  50 |        11.97 |      45.38 |    1.02 |
| 100 |        13.05 |      56.11 |    1.45 |
| 200 |        18.19 |      81.27 |    2.36 |
| 300 |        24.73 |      98.50 |    4.55 |
| 500 |        45.52 |     129.44 |   10.75 |

The RSS measurements show substantial differences between language runtimes. Java has the highest process RSS because the JVM introduces significant runtime overhead, while C has the lowest.

These measurements should not be interpreted as the memory required exclusively by the matrices.

## Scaling Analysis

The observed time exponents between `n = 50` and `n = 500` are:

| Language | Observed time exponent |
| -------- | ---------------------: |
| Python   |                  3.035 |
| Java     |                  2.387 |
| C        |                  2.930 |

Python and C are very close to the theoretical cubic behaviour expected from the triple-loop algorithm.

The Java exponent is lower, which can be explained by practical effects such as JIT compilation, runtime behaviour and the relatively small number of tested matrix sizes.

The measured RSS exponents are lower than the theoretical O(n²) matrix-storage exponent because RSS includes a substantial fixed runtime overhead. Therefore, the theoretical O(n²) storage complexity remains the appropriate model for the matrices themselves.

## Correctness

Correctness was tested using:

* Known small matrices
* Identity matrices
* Zero matrices
* Real generated input data

All three implementations produced the expected results.

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
│
├── java/
│   ├── MatrixMultiplication.java
│   ├── MatrixIO.java
│   ├── CorrectnessTest.java
│   ├── TestRealData.java
│   ├── Benchmark.java
│   └── MemoryBenchmark.java
│
├── c/
│   ├── matrix_multiplication.c
│   ├── matrix_io.c
│   ├── matrix_io.h
│   ├── correctness.c
│   ├── benchmark.c
│   ├── test_real_data.c
│   └── memory_benchmark.c
│
├── data/
│   ├── input/
│   └── results/
│
├── scripts/
│   ├── generate_input.py
│   ├── analyze_results.py
│   ├── analyze_numbers.py
│   └── estimate_scaling.py
│
└── report/
    └── figures/
```

## Figures

The generated figures are available in `report/figures/`:

* `execution_time.png`
* `execution_time_log.png`
* `memory_usage.png`
* `memory_usage_log.png`

## Reproducibility

Input data can be regenerated with:

```bash
python scripts/generate_input.py
```

The benchmark and analysis scripts can then be executed independently for each implementation.

The experimental results are stored in:

```text
data/results/
```

and the combined dataset is:

```text
data/results/combined_results.csv
```

## Conclusion

The experiment confirms the expected cubic growth of execution time for the basic matrix multiplication algorithm. Python shows the highest execution times, while Java and C provide considerably better performance under the tested conditions.

The memory experiment shows that process-level RSS is strongly affected by language runtime overhead. C has the lowest measured RSS, Python is intermediate, and Java has the highest RSS because of the JVM.

Overall, the results demonstrate how the same algorithm can exhibit significantly different practical performance characteristics depending on the programming language and runtime environment.
