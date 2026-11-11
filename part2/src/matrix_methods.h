#ifndef MATRIX_METHODS_H
#define MATRIX_METHODS_H

typedef struct {
    int n;
    int nnz;
    double *values;
    int *col_idx;
    int *row_ptr;
} CSRMatrix;

void matmul_basic(const double *A, const double *B, double *C, int n);
void matmul_reordered(const double *A, const double *B, double *C, int n);
void matmul_blocked(const double *A, const double *B, double *C, int n, int block);
int csr_from_dense(const double *A, int n, CSRMatrix *out);
void csr_free(CSRMatrix *A);
void matmul_csr(const CSRMatrix *A, const CSRMatrix *B, double *C);

#endif
