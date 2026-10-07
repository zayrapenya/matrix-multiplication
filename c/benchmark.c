#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double *load_matrix(const char *path, int n);
double *multiply(const double *A, const double *B, int n);

#define WARMUP_RUNS 3
#define MEASURED_RUNS 5

static double get_time_seconds(void) {
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (double)ts.tv_sec +
           (double)ts.tv_nsec / 1000000000.0;
}

static double run_multiplication(
        const double *A,
        const double *B,
        int n) {

    double *C = multiply(A, B, n);

    if (C == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    double checksum = 0.0;

    for (int i = 0; i < n * n; i++) {
        checksum += C[i];
    }

    free(C);

    if (checksum == 1.0 / 0.0) {
        fprintf(stderr, "Invalid result.\n");
        exit(EXIT_FAILURE);
    }

    return checksum;
}

static double median(double *values, int count) {
    double sorted[MEASURED_RUNS];

    for (int i = 0; i < count; i++) {
        sorted[i] = values[i];
    }

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (sorted[j] < sorted[i]) {
                double temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }

    return sorted[count / 2];
}

static double percentile(
        double *values,
        int count,
        double p) {

    double sorted[MEASURED_RUNS];

    for (int i = 0; i < count; i++) {
        sorted[i] = values[i];
    }

    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (sorted[j] < sorted[i]) {
                double temp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = temp;
            }
        }
    }

    double position = p * (count - 1);

    int lower = (int)position;
    int upper = lower + 1;

    if (upper >= count) {
        return sorted[lower];
    }

    double weight = position - lower;

    return sorted[lower] +
           weight * (sorted[upper] - sorted[lower]);
}

static void benchmark(
        const double *A,
        const double *B,
        int n,
        double *times) {

    for (int i = 0; i < WARMUP_RUNS; i++) {
        run_multiplication(A, B, n);
    }

    for (int i = 0; i < MEASURED_RUNS; i++) {

        double start = get_time_seconds();

        run_multiplication(A, B, n);

        double end = get_time_seconds();

        times[i] = end - start;
    }
}

int main(void) {

    const int sizes[] = {50, 100, 200, 300, 500};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);

    FILE *output = fopen(
        "data/results/c_results.csv",
        "w"
    );

    FILE *raw_output = fopen(
        "data/results/raw/c_raw.csv",
        "w"
    );

    if (output == NULL || raw_output == NULL) {
        perror("Could not open output file");
        if (output != NULL) fclose(output);
        if (raw_output != NULL) fclose(raw_output);
        return EXIT_FAILURE;
    }

    fprintf(raw_output, "language,n,run,seconds\n");

    fprintf(
        output,
        "language,n,warmup_runs,measured_runs,"
        "median_seconds,iqr_seconds\n"
    );

    for (int s = 0; s < num_sizes; s++) {

        int n = sizes[s];

        printf("Running C n=%d...\n", n);

        char path_A[256];
        char path_B[256];

        snprintf(
            path_A,
            sizeof(path_A),
            "data/input/n%d_A.csv",
            n
        );

        snprintf(
            path_B,
            sizeof(path_B),
            "data/input/n%d_B.csv",
            n
        );

        double *A = load_matrix(path_A, n);
        double *B = load_matrix(path_B, n);

        if (A == NULL || B == NULL) {
            fprintf(stderr, "Could not load matrices.\n");
            free(A);
            free(B);
            fclose(output);
            return EXIT_FAILURE;
        }

        double times[MEASURED_RUNS];

        benchmark(A, B, n, times);

        for (int i = 0; i < MEASURED_RUNS; i++) {
            fprintf(
                raw_output,
                "C,%d,%d,%.9f\n",
                n,
                i + 1,
                times[i]
            );
        }

        double med = median(times, MEASURED_RUNS);
        double q1 = percentile(times, MEASURED_RUNS, 0.25);
        double q3 = percentile(times, MEASURED_RUNS, 0.75);
        double iqr = q3 - q1;

        fprintf(
            output,
            "C,%d,%d,%d,%.9f,%.9f\n",
            n,
            WARMUP_RUNS,
            MEASURED_RUNS,
            med,
            iqr
        );

        printf(
            "  median=%.9fs IQR=%.9fs\n",
            med,
            iqr
        );

        free(A);
        free(B);
    }

    fclose(output);
    fclose(raw_output);

    printf(
        "\nResults saved to data/results/c_results.csv\n"
    );

    printf(
        "Raw measurements saved to data/results/raw/c_raw.csv\n"
    );

    return EXIT_SUCCESS;
}
