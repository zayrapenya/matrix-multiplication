#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix_parallel.h"

static uint64_t splitmix64(uint64_t *s) {
    uint64_t z = (*s += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

void fill_random(double *M, int n, uint64_t seed) {
    uint64_t s = seed;
    for (long i = 0; i < (long)n * n; i++)
        M[i] = (double)(splitmix64(&s) >> 11) * (1.0 / 9007199254740992.0); /* [0,1) */
}

/* Shared row kernel: computes rows [i0, i1) of C = A*B (i-k-j order). */
static void rows_ikj(const double *restrict A, const double *restrict B,
                     double *restrict C, int n, int i0, int i1) {
    for (int i = i0; i < i1; i++) {
        double *restrict Ci = C + (long)i * n;
        memset(Ci, 0, (size_t)n * sizeof(double));
        for (int k = 0; k < n; k++) {
            const double aik = A[(long)i * n + k];
            const double *restrict Bk = B + (long)k * n;
            for (int j = 0; j < n; j++) Ci[j] += aik * Bk[j];
        }
    }
}

void mm_seq_ikj(const double *A, const double *B, double *C, int n) {
    rows_ikj(A, B, C, n, 0, n);
}

typedef struct { const double *A, *B; double *C; int n, i0, i1; } job_t;

static void *worker(void *arg) {
    job_t *j = (job_t *)arg;
    rows_ikj(j->A, j->B, j->C, j->n, j->i0, j->i1);
    return NULL;
}

void mm_par_ikj(const double *A, const double *B, double *C, int n, int p) {
    if (p < 1) p = 1;
    if (p > n) p = n;                       /* never more threads than rows */
    pthread_t *th = malloc((size_t)p * sizeof *th);
    job_t *jobs = malloc((size_t)p * sizeof *jobs);
    if (!th || !jobs) { fprintf(stderr, "alloc failed\n"); exit(2); }
    for (int t = 0; t < p; t++) {
        jobs[t] = (job_t){A, B, C, n, (int)((long)n * t / p), (int)((long)n * (t + 1) / p)};
        if (pthread_create(&th[t], NULL, worker, &jobs[t]) != 0) {
            fprintf(stderr, "pthread_create failed\n"); exit(2);
        }
    }
    for (int t = 0; t < p; t++) pthread_join(th[t], NULL);
    free(th); free(jobs);
}

void mm_ref_ijk(const double *A, const double *B, double *C, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            double s = 0.0;
            for (int k = 0; k < n; k++) s += A[(long)i * n + k] * B[(long)k * n + j];
            C[(long)i * n + j] = s;
        }
}
