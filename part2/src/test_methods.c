#include "matrix_methods.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int equal(const double *A, const double *B, int n) {
    for (int i = 0; i < n*n; i++) {
        double diff = fabs(A[i] - B[i]);
        if (diff > 1e-9 * (1.0 + fabs(A[i]))) return 0;
    }
    return 1;
}

int main(void) {
    const int n = 4;
    double A[16] = {
        1,0,2,0,
        0,3,0,4,
        5,0,6,0,
        0,7,0,8
    };
    double B[16] = {
        1,2,0,0,
        0,3,4,0,
        5,0,6,0,
        0,7,0,8
    };
    double C0[16], C1[16], C2[16], C3[16];

    matmul_basic(A, B, C0, n);
    matmul_reordered(A, B, C1, n);
    matmul_blocked(A, B, C2, n, 2);

    if (!equal(C0, C1, n) || !equal(C0, C2, n)) {
        fprintf(stderr, "Dense methods: FAIL\n");
        return 1;
    }
    puts("Dense i-j-k vs i-k-j and blocked: PASS");

    CSRMatrix As = {0}, Bs = {0};
    if (!csr_from_dense(A, n, &As) || !csr_from_dense(B, n, &Bs)) {
        fprintf(stderr, "CSR allocation: FAIL\n");
        csr_free(&As);
        csr_free(&Bs);
        return 1;
    }

    matmul_csr(&As, &Bs, C3);
    if (!equal(C0, C3, n)) {
        fprintf(stderr, "Dense vs CSR: FAIL\n");
        csr_free(&As);
        csr_free(&Bs);
        return 1;
    }
    puts("Dense vs CSR: PASS");

    double Z[16] = {0}, I[16] = {0}, R[16];
    for (int i = 0; i < n; i++) I[i*n+i] = 1.0;
    matmul_basic(A, Z, R, n);
    if (!equal(R, Z, n)) {
        fprintf(stderr, "Zero matrix test: FAIL\n");
        csr_free(&As);
        csr_free(&Bs);
        return 1;
    }
    matmul_basic(A, I, R, n);
    if (!equal(R, A, n)) {
        fprintf(stderr, "Identity matrix test: FAIL\n");
        csr_free(&As);
        csr_free(&Bs);
        return 1;
    }
    puts("Zero and identity tests: PASS");

    csr_free(&As);
    csr_free(&Bs);
    return 0;
}
