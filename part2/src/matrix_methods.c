#include "matrix_methods.h"
#include <stdlib.h>

void matmul_basic(const double *A, const double *B, double *C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++)
                sum += A[i*n+k] * B[k*n+j];
            C[i*n+j] = sum;
        }
}

void matmul_reordered(const double *A, const double *B, double *C, int n) {
    for (int i = 0; i < n*n; i++) C[i] = 0.0;
    for (int i = 0; i < n; i++)
        for (int k = 0; k < n; k++) {
            double a = A[i*n+k];
            for (int j = 0; j < n; j++)
                C[i*n+j] += a * B[k*n+j];
        }
}

void matmul_blocked(const double *A, const double *B, double *C,
                    int n, int block) {
    if (block < 1) block = 1;
    for (int i = 0; i < n*n; i++) C[i] = 0.0;

    for (int ii = 0; ii < n; ii += block)
        for (int kk = 0; kk < n; kk += block)
            for (int jj = 0; jj < n; jj += block) {
                int imax = ii + block < n ? ii + block : n;
                int kmax = kk + block < n ? kk + block : n;
                int jmax = jj + block < n ? jj + block : n;

                for (int i = ii; i < imax; i++)
                    for (int k = kk; k < kmax; k++) {
                        double a = A[i*n+k];
                        for (int j = jj; j < jmax; j++)
                            C[i*n+j] += a * B[k*n+j];
                    }
            }
}

int csr_from_dense(const double *A, int n, CSRMatrix *out) {
    if (!A || !out || n < 1) return 0;

    int count = 0;
    for (int i = 0; i < n*n; i++)
        if (A[i] != 0.0) count++;

    out->n = n;
    out->nnz = count;
    out->values = count ? malloc((size_t)count * sizeof(double)) : NULL;
    out->col_idx = count ? malloc((size_t)count * sizeof(int)) : NULL;
    out->row_ptr = malloc((size_t)(n+1) * sizeof(int));

    if (!out->row_ptr || (count && (!out->values || !out->col_idx))) {
        csr_free(out);
        return 0;
    }

    int p = 0;
    for (int i = 0; i < n; i++) {
        out->row_ptr[i] = p;
        for (int j = 0; j < n; j++) {
            double v = A[i*n+j];
            if (v != 0.0) {
                out->values[p] = v;
                out->col_idx[p] = j;
                p++;
            }
        }
    }
    out->row_ptr[n] = p;
    return 1;
}

void csr_free(CSRMatrix *A) {
    if (!A) return;
    free(A->values);
    free(A->col_idx);
    free(A->row_ptr);
    A->values = NULL;
    A->col_idx = NULL;
    A->row_ptr = NULL;
    A->n = 0;
    A->nnz = 0;
}

void matmul_csr(const CSRMatrix *A, const CSRMatrix *B, double *C) {
    int n = A->n;
    for (int i = 0; i < n*n; i++) C[i] = 0.0;

    for (int i = 0; i < n; i++)
        for (int ap = A->row_ptr[i]; ap < A->row_ptr[i+1]; ap++) {
            int k = A->col_idx[ap];
            double a = A->values[ap];
            for (int bp = B->row_ptr[k]; bp < B->row_ptr[k+1]; bp++) {
                int j = B->col_idx[bp];
                C[i*n+j] += a * B->values[bp];
            }
        }
}
