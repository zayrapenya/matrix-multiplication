# Assignment 2 — Optimised Matrix Multiplication and Sparse Matrices

## 1. Objective

This experiment compares three sequential dense matrix multiplication implementations and a sparse implementation using Compressed Sparse Row (CSR). The goals are to measure the impact of loop reordering and blocking, and to determine how input sparsity affects the performance of CSR multiplication.

## 2. Implementation

All implementations are written in C and use `double` values.

- **Basic:** conventional `i-j-k` loop order.
- **Reordered:** `i-k-j` loop order, which accesses consecutive elements of a row of B and C.
- **Blocked:** tiled multiplication with a block size of 32.
- **CSR:** sparse matrices represented by values, column indices and row pointers. The result is stored as a dense matrix.

The implementations run sequentially. The benchmark uses a deterministic seed for each matrix size and zero percentage. Dense methods and CSR therefore receive the same generated logical input for each matching configuration.

## 3. Experimental protocol

Dense performance was tested at dimensions 100, 200, 500 and 800, with no deliberately introduced zeros. Sparse comparisons used the same dimensions and input zero percentages of 50%, 90%, 95% and 99%.

Each configuration has five measured repetitions. The summary reports the median and interquartile range (IQR). The benchmark times the multiplication separately from CSR conversion. For an end-to-end CSR comparison, conversion time is added to multiplication time.

Correctness tests passed for comparisons between the dense implementations, dense versus CSR, and zero/identity cases. The benchmark also reported matching checksums within a tolerance of 1e-7. A checksum is a useful consistency check but does not replace element-by-element comparison.

## 4. Dense results

Median multiplication time, in seconds:

| Dimension | Basic | Reordered | Blocked |
|---:|---:|---:|---:|
| 100 | 0.000815 | 0.000164 | 0.000289 |
| 200 | 0.009376 | 0.001404 | 0.001648 |
| 500 | 0.132658 | 0.018953 | 0.023457 |
| 800 | 0.592104 | 0.079646 | 0.095121 |

The reordered implementation was the fastest dense method at every tested size. At dimension 800, its median time was approximately 7.43 times lower than the basic implementation. Blocking was also faster than the basic method, but it did not outperform loop reordering in these experiments.

The results are consistent with loop order having a substantial effect on memory access patterns. The measurements do not establish that the same ordering will be fastest on every processor or for every matrix size.

## 5. Sparse results

The following table compares the dense baseline with CSR. CSR total time includes the measured conversion time.

| Dimension | Zeros | Dense median (s) | CSR multiply (s) | Conversion (s) | CSR total (s) | Speedup |
|---:|---:|---:|---:|---:|---:|---:|
| 100 | 50% | 0.000745 | 0.000137 | 0.000096 | 0.000233 | 3.20x |
| 200 | 50% | 0.006167 | 0.001119 | 0.000386 | 0.001505 | 4.10x |
| 500 | 50% | 0.132278 | 0.015084 | 0.002453 | 0.017537 | 7.54x |
| 800 | 50% | 0.586540 | 0.059211 | 0.006337 | 0.065548 | 8.95x |
| 100 | 90% | 0.000727 | 0.000008 | 0.000029 | 0.000037 | 19.65x |
| 200 | 90% | 0.006178 | 0.000064 | 0.000110 | 0.000174 | 35.51x |
| 500 | 90% | 0.132201 | 0.000856 | 0.000780 | 0.001636 | 80.81x |
| 800 | 90% | 0.584820 | 0.003305 | 0.001823 | 0.005128 | 114.04x |
| 100 | 95% | 0.000750 | 0.000003 | 0.000020 | 0.000023 | 32.61x |
| 200 | 95% | 0.006148 | 0.000023 | 0.000079 | 0.000102 | 60.27x |
| 500 | 95% | 0.132255 | 0.000263 | 0.000491 | 0.000754 | 175.40x |
| 800 | 95% | 0.586663 | 0.000985 | 0.001296 | 0.002281 | 257.20x |
| 100 | 99% | 0.000728 | 0.000001 | 0.000013 | 0.000014 | 52.00x |
| 200 | 99% | 0.006194 | 0.000004 | 0.000054 | 0.000058 | 106.79x |
| 500 | 99% | 0.132681 | 0.000047 | 0.000320 | 0.000367 | 361.53x |
| 800 | 99% | 0.590133 | 0.000117 | 0.000819 | 0.000936 | 630.48x |

CSR becomes increasingly advantageous as the proportion of zeros rises. At low sparsity, conversion overhead is a more significant fraction of total time. At high sparsity, skipping zero entries greatly reduces the work required by the sparse multiplication algorithm.

These speedups apply to the tested implementation, input generator, compiler and hardware. They should not be assumed to generalise to all sparse matrices.

## 6. Memory and limitations

The benchmark records a sample of the process's resident memory. It is not a peak-memory measurement and can include memory unrelated to the matrix data. Therefore, the memory graph should be interpreted as an approximate sampled comparison, not as proof of each method's maximum memory requirement.

The tested sizes stop at 800. Larger sizes were not included in this dataset, so no conclusions about performance beyond that size are claimed. Five repetitions provide an initial estimate of variability, but additional repetitions could improve confidence, especially for very short sparse operations.

The CSR conversion is timed separately. The reported end-to-end time adds the median conversion time to the median multiplication time; this is an estimate based on the two medians rather than the median of paired total-time measurements.

## 7. Conclusion

For these experiments, changing the dense loop order to `i-k-j` gave the best dense performance. Blocking improved on the basic implementation but was slower than reordering at every tested dimension. CSR was especially effective for highly sparse inputs, even when conversion time was included. The results illustrate why the choice between dense and sparse storage should depend on the number of nonzero entries and on whether conversion is part of the actual workload.

## 8. Reproducibility and artifacts

Raw measurements are in `results/raw_benchmarks.csv`, aggregated statistics in `results/summary.csv`, and additional calculated comparisons in `results/analysis_notes.txt`. The generated figures are in `figures/`.

Run the benchmark and analysis instructions in `README.md` from the repository root. Record the exact compiler version and machine details when preparing the final submission.

## 9. Hardware, environment and measurement scope

The experiments were run on a MacBook Air with an Apple M1 chip (8 CPU cores: 4 performance and 4 efficiency cores) and 8 GB of RAM. The operating system was macOS 14.6.1, the C compiler was Apple Clang 15.0.0, and the Python version was 3.14.7. The benchmark uses C double-precision values and sequential execution.

### Sparsity, density and nonzero counts

The input generator's zero percentage (`zero_pct`) is the percentage of matrix entries deliberately set to zero. Density is the percentage of nonzero entries, calculated as `100 - zero_pct`. For example, an input with 90% zeros has a nominal density of 10%.

The raw CSV records `nnz_A`, `nnz_B` and `nnz_C` for each measurement, where NNZ means the number of nonzero entries in the corresponding matrix. These fields allow the actual generated input and output counts to be inspected for every size and sparsity configuration. The CSR representation stores values, column indices and row pointers; the multiplication result is dense.

### Experimental limits and memory caveat

The largest tested dimension was 800 x 800. The largest median time among the dense baseline cases was approximately 0.586 seconds. These values describe the tested workload, not a guaranteed time limit for other machines or inputs. No hard execution timeout was enforced, and no strict peak-memory budget was verified.

The benchmark records sampled resident memory, not peak memory. Consequently, the memory results must not be interpreted as proof that a method stays below a specified memory limit. The machine has 8 GB of physical RAM, but this experiment did not establish the peak memory consumed by each method.

### Reproducibility details

The benchmark input generator uses the deterministic seed `42 + n * 17 + zero_pct`. The benchmark script runs five repetitions for each configured method, dimension and zero percentage. To reproduce the measurements from the repository root, compile the benchmark as described in `README.md`, run `bash part2/scripts/run_benchmarks.sh`, and then run `python part2/scripts/analyze_results.py`. Running the benchmark script replaces `results/raw_benchmarks.csv`; preserve a copy first if the existing measurements need to be retained.
