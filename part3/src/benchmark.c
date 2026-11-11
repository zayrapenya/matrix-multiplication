#ifndef __APPLE__
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include "matrix_parallel.h"

/* Usage: benchmark <seq|par> <n> <threads> <reps> <warmup> <budget_s> <label>
   One process = one configuration, so ru_maxrss is the peak RSS of that
   configuration alone. Timed region = kernel only (includes zeroing of C and,
   for "par", thread creation/join). Inputs are generated outside the timer. */
static double now_s(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char **argv) {
    if (argc != 8) { fprintf(stderr, "usage: %s seq|par n threads reps warmup budget_s label\n", argv[0]); return 1; }
    const char *method = argv[1];
    int n = atoi(argv[2]), p = atoi(argv[3]), reps = atoi(argv[4]), warm = atoi(argv[5]);
    unsigned budget = (unsigned)atoi(argv[6]);
    const char *label = argv[7];
    int is_par = strcmp(method, "par") == 0;
    if (!is_par && strcmp(method, "seq") != 0) { fprintf(stderr, "method must be seq or par\n"); return 1; }
    if (!is_par) p = 1;
    if (budget) alarm(budget);              /* SIGALRM kills the run if the budget is exceeded */

    size_t bytes = (size_t)n * n * sizeof(double);
    double *A = malloc(bytes), *B = malloc(bytes), *C = malloc(bytes);
    if (!A || !B || !C) { fprintf(stderr, "allocation failed\n"); return 2; }
    fill_random(A, n, 42 + 17 * (uint64_t)n);
    fill_random(B, n, 42 + 17 * (uint64_t)n + 1000003);
    memset(C, 0, bytes);                     /* touch pages before timing */

    double *times = malloc((size_t)reps * sizeof(double));
    double checksum = 0.0;
    for (int r = -warm; r < reps; r++) {
        double t0 = now_s();
        if (is_par) mm_par_ikj(A, B, C, n, p); else mm_seq_ikj(A, B, C, n);
        double t1 = now_s();
        double cs = 0.0;
        for (long i = 0; i < (long)n * n; i++) cs += C[i];   /* consume the result */
        checksum = cs;
        if (r >= 0) times[r] = t1 - t0;
    }
    struct rusage ru; getrusage(RUSAGE_SELF, &ru);
#ifdef __APPLE__
    long rss_kib = ru.ru_maxrss / 1024;      /* bytes on macOS */
#else
    long rss_kib = ru.ru_maxrss;             /* KiB on Linux */
#endif
    for (int r = 0; r < reps; r++)
        printf("%s,%d,%d,%d,%.9f,%.10e,%ld,%s\n", method, n, p, r + 1, times[r], checksum, rss_kib, label);
    free(A); free(B); free(C); free(times);
    return 0;
}
