#define _POSIX_C_SOURCE 200809L
#include "matrix_methods.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mach/mach.h>

static double now_seconds(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static double checksum(const double *C, int n) {
    double s = 0.0;
    for (int i = 0; i < n*n; i++) s += C[i];
    return s;
}

static int count_nnz(const double *A, int n) {
    int count = 0;
    for (int i = 0; i < n*n; i++)
        if (A[i] != 0.0) count++;
    return count;
}

static void generate(double *A, double *B, int n, int zero_pct) {
    for (int i = 0; i < n*n; i++) {
        A[i] = rand() % 100 < zero_pct ? 0.0
             : (double)(rand() % 2001 - 1000) / 1000.0;
        B[i] = rand() % 100 < zero_pct ? 0.0
             : (double)(rand() % 2001 - 1000) / 1000.0;
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s n zero_percent repetitions\n", argv[0]);
        return 1;
    }

    int n = atoi(argv[1]);
    int zero_pct = atoi(argv[2]);
    int reps = atoi(argv[3]);

    if (n < 1 || zero_pct < 0 || zero_pct > 99 || reps < 1) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }

    size_t count = (size_t)n * (size_t)n;
    if (count > (size_t)-1 / sizeof(double)) {
        fprintf(stderr, "Matrix too large\n");
        return 1;
    }

    double *A = malloc(count * sizeof(double));
    double *B = malloc(count * sizeof(double));
    double *C = malloc(count * sizeof(double));
    if (!A || !B || !C) {
        fprintf(stderr, "Memory allocation failed for n=%d\n", n);
        free(A); free(B); free(C);
        return 1;
    }

    srand(42 + n * 17 + zero_pct);
    generate(A, B, n, zero_pct);

    int nnz_A = count_nnz(A, n);
    int nnz_B = count_nnz(B, n);
    double conversion_seconds = 0.0;
    CSRMatrix As = {0}, Bs = {0};

    const char *method = getenv("METHOD");
    if (!method) method = "basic";

    if (strcmp(method, "csr") == 0) {
        double start = now_seconds();
        if (!csr_from_dense(A, n, &As) ||
            !csr_from_dense(B, n, &Bs)) {
            fprintf(stderr, "CSR conversion failed\n");
            csr_free(&As); csr_free(&Bs);
            free(A); free(B); free(C);
            return 1;
        }
        conversion_seconds = now_seconds() - start;
        free(A); A = NULL;
        free(B); B = NULL;
    }

    /* One warm-up run, excluded from reported samples. */
    if (strcmp(method, "basic") == 0)
        matmul_basic(A, B, C, n);
    else if (strcmp(method, "reordered") == 0)
        matmul_reordered(A, B, C, n);
    else if (strcmp(method, "blocked") == 0)
        matmul_blocked(A, B, C, n, 32);
    else if (strcmp(method, "csr") == 0)
        matmul_csr(&As, &Bs, C);
    else {
        fprintf(stderr, "Unknown method: %s\n", method);
        csr_free(&As); csr_free(&Bs);
        free(A); free(B); free(C);
        return 1;
    }

    /* Touch the result so the computation is consumed. */
    volatile double warmup_check = checksum(C, n);
    (void)warmup_check;

    for (int r = 1; r <= reps; r++) {
        double start = now_seconds();

        if (strcmp(method, "basic") == 0)
            matmul_basic(A, B, C, n);
        else if (strcmp(method, "reordered") == 0)
            matmul_reordered(A, B, C, n);
        else if (strcmp(method, "blocked") == 0)
            matmul_blocked(A, B, C, n, 32);
        else
            matmul_csr(&As, &Bs, C);

        double elapsed = now_seconds() - start;
        double sum = checksum(C, n);
        int nnz_C = count_nnz(C, n);

        mach_task_basic_info_data_t info;
        mach_msg_type_number_t info_count = MACH_TASK_BASIC_INFO_COUNT;

        kern_return_t kr = task_info(
            mach_task_self(),
            MACH_TASK_BASIC_INFO,
            (task_info_t)&info,
            &info_count
        );

        if (kr != KERN_SUCCESS) {
            fprintf(stderr, "Error al medir la memoria\n");
            return 1;
        }

        printf("%s,%d,%d,%d,%.9f,%.9f,%llu,%d,%d,%d,%.12f\n",
               method, n, zero_pct, r, elapsed,
               conversion_seconds,
               (unsigned long long)info.resident_size,
               nnz_A, nnz_B, nnz_C, sum);
        fflush(stdout);
    }

    csr_free(&As);
    csr_free(&Bs);
    free(A); free(B); free(C);
    return 0;
}
