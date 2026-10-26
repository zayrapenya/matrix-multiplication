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
| 100 | 0.001754 | 0.000285 | 0.000433 |
| 200 | 0.007914 | 0.001299 | 0.001616 |
| 500 | 0.133325 | 0.019522 | 0.023363 |
| 800 | 0.586206 | 0.081109 | 0.094841 |

The reordered implementation was the fastest dense method at every tested size. At dimension 800, its median time was approximately 7.23 times lower than the basic implementation. Blocking was also faster than the basic method, but it did not outperform loop reordering in these experiments.

The results are consistent with loop order having a substantial effect on memory access patterns. The measurements do not establish that the same ordering will be fastest on every processor or for every matrix size.

## 5. Sparse results

The following table compares the dense baseline with CSR. CSR total time includes the measured conversion time.

| Dimension | Zeros | Dense median (s) | CSR multiply (s) | Conversion (s) | CSR total (s) | Speedup |
|---:|---:|---:|---:|---:|---:|---:|
| 100 | 50% | 0.000705 | 0.000137 | 0.000097 | 0.000234 | 3.01x |
| 200 | 50% | 0.006162 | 0.001095 | 0.000382 | 0.001477 | 4.17x |
| 500 | 50% | 0.132568 | 0.015077 | 0.002637 | 0.017714 | 7.48x |
| 800 | 50% | 0.587858 | 0.059881 | 0.006192 | 0.066073 | 8.90x |
| 100 | 90% | 0.000740 | 0.000009 | 0.000030 | 0.000039 | 18.97x |
| 200 | 90% | 0.006147 | 0.000063 | 0.000115 | 0.000178 | 34.53x |
| 500 | 90% | 0.132613 | 0.000853 | 0.000736 | 0.001589 | 83.46x |
| 800 | 90% | 0.589976 | 0.003284 | 0.001808 | 0.005092 | 115.86x |
| 100 | 95% | 0.000739 | 0.000004 | 0.000021 | 0.000025 | 29.56x |
| 200 | 95% | 0.006148 | 0.000022 | 0.000079 | 0.000101 | 60.87x |
| 500 | 95% | 0.132692 | 0.000266 | 0.000496 | 0.000762 | 174.14x |
| 800 | 95% | 0.588215 | 0.001018 | 0.001272 | 0.002290 | 256.86x |
| 100 | 99% | 0.000746 | 0.000001 | 0.000016 | 0.000017 | 43.88x |
| 200 | 99% | 0.006132 | 0.000004 | 0.000053 | 0.000057 | 107.58x |
| 500 | 99% | 0.133522 | 0.000040 | 0.000316 | 0.000356 | 375.06x |
| 800 | 99% | 0.588431 | 0.000115 | 0.000827 | 0.000942 | 624.66x |

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
