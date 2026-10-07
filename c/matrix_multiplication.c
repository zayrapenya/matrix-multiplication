#include <stdlib.h>

double *multiply(const double *A, const double *B, int n) {
    double *C = calloc((size_t)n * n, sizeof(double));

    if (C == NULL) {
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                C[i * n + j] +=
                    A[i * n + k] * B[k * n + j];
            }
        }
    }

    return C;
}
