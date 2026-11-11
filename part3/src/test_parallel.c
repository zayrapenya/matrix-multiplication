#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix_parallel.h"

static int failures = 0;
#define CHECK(cond, ...) do { if (!(cond)) { failures++; printf("FAIL: "); printf(__VA_ARGS__); printf("\n"); } } while (0)

int main(void) {
    const int sizes[] = {1, 2, 3, 7, 16, 33, 100};
    const int threads[] = {1, 2, 3, 4, 8, 16};
    const double REL_TOL = 1e-9;   /* vs. i-j-k reference: different summation order */
    double worst_ref = 0.0;

    for (size_t s = 0; s < sizeof sizes / sizeof *sizes; s++) {
        int n = sizes[s];
        size_t bytes = (size_t)n * n * sizeof(double);
        double *A = malloc(bytes), *B = malloc(bytes), *Cs = malloc(bytes), *Cp = malloc(bytes), *R = malloc(bytes), *I = calloc((size_t)n * n, sizeof(double));
        fill_random(A, n, 42 + 17 * (uint64_t)n);
        fill_random(B, n, 42 + 17 * (uint64_t)n + 1000003);
        for (int i = 0; i < n; i++) I[(long)i * n + i] = 1.0;

        mm_seq_ikj(A, B, Cs, n);
        mm_ref_ijk(A, B, R, n);
        for (long x = 0; x < (long)n * n; x++) {
            double err = fabs(Cs[x] - R[x]) / fmax(1.0, fabs(R[x]));
            if (err > worst_ref) worst_ref = err;
            CHECK(err <= REL_TOL, "seq vs ref n=%d idx=%ld", n, x);
        }
        for (size_t t = 0; t < sizeof threads / sizeof *threads; t++) {
            int p = threads[t];
            mm_par_ikj(A, B, Cp, n, p);
            /* same kernel + same per-row order => results must be bit-identical */
            CHECK(memcmp(Cs, Cp, bytes) == 0, "par != seq (bitwise) n=%d p=%d", n, p);
        }
        /* identity: A*I = A */
        mm_par_ikj(A, I, Cp, n, 4);
        CHECK(memcmp(A, Cp, bytes) == 0, "A*I != A n=%d", n);
        /* zero: A*0 = 0 */
        double *Z = calloc((size_t)n * n, sizeof(double));
        mm_par_ikj(A, Z, Cp, n, 4);
        for (long x = 0; x < (long)n * n; x++) CHECK(Cp[x] == 0.0, "A*0 != 0 n=%d", n);
        free(A); free(B); free(Cs); free(Cp); free(R); free(I); free(Z);
    }
    printf("max relative error vs i-j-k reference: %.3e (tolerance %.0e)\n", worst_ref, REL_TOL);
    printf(failures ? "TESTS FAILED (%d)\n" : "ALL TESTS PASSED\n", failures);
    return failures ? 1 : 0;
}
