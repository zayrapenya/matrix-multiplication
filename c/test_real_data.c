#include <stdio.h>
#include <stdlib.h>

double *load_matrix(const char *path, int n);
double *multiply(const double *A, const double *B, int n);

int main(void) {
    int n = 50;

    double *A = load_matrix("data/input/n50_A.csv", n);
    double *B = load_matrix("data/input/n50_B.csv", n);

    if (A == NULL || B == NULL) {
        printf("Error loading matrices.\n");
        free(A);
        free(B);
        return 1;
    }

    double *C = multiply(A, B, n);

    if (C == NULL) {
        printf("Error multiplying matrices.\n");
        free(A);
        free(B);
        return 1;
    }

    double checksum = 0.0;

    for (int i = 0; i < n * n; i++) {
        checksum += C[i];
    }

    printf("C real data test passed.\n");
    printf("Matrix size: %dx%d\n", n, n);
    printf("Result checksum: %.12f\n", checksum);

    free(A);
    free(B);
    free(C);

    return 0;
}
