#include <stdio.h>
#include <stdlib.h>

double *load_matrix(const char *path, int n);

double *multiply(const double *A, const double *B, int n);

static void run_multiplication(const double *A, const double *B, int n) {
    double *C = multiply(A, B, n);

    if (C == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    double checksum = 0.0;

    for (int i = 0; i < n * n; i++) {
        checksum += C[i];
    }

    if (checksum == 1.0 / 0.0) {
        fprintf(stderr, "Invalid result.\n");
        free(C);
        exit(EXIT_FAILURE);
    }

    free(C);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);

    if (n <= 0) {
        fprintf(stderr, "Invalid matrix size.\n");
        return EXIT_FAILURE;
    }

    char path_A[256];
    char path_B[256];

    snprintf(path_A, sizeof(path_A), "data/input/n%d_A.csv", n);
    snprintf(path_B, sizeof(path_B), "data/input/n%d_B.csv", n);

    double *A = load_matrix(path_A, n);
    double *B = load_matrix(path_B, n);

    if (A == NULL || B == NULL) {
        fprintf(stderr, "Could not load matrices.\n");
        free(A);
        free(B);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < 3; i++) {
        run_multiplication(A, B, n);
    }

    run_multiplication(A, B, n);

    free(A);
    free(B);

    return EXIT_SUCCESS;
}
