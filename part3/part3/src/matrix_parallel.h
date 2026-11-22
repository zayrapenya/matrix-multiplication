#ifndef MATRIX_PARALLEL_H
#define MATRIX_PARALLEL_H
#include <stdint.h>

/* All matrices are n x n, row-major, double precision. */
void fill_random(double *M, int n, uint64_t seed);

/* Sequential baseline: i-k-j (best dense method of Assignment 2).
   The kernel zero-initialises C itself (inside the timed region). */
void mm_seq_ikj(const double *A, const double *B, double *C, int n);

/* Parallel version: SAME row kernel as mm_seq_ikj, rows of C split into p
   contiguous blocks, one pthread per block. Thread creation/join is part of
   the timed region. With p = 1 it still spawns one worker thread so that the
   framework overhead is visible. */
void mm_par_ikj(const double *A, const double *B, double *C, int n, int p);

/* Naive i-j-k reference, used only for correctness checks. */
void mm_ref_ijk(const double *A, const double *B, double *C, int n);
#endif
