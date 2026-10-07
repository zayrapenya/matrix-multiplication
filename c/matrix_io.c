#include <stdio.h>
#include <stdlib.h>

double *load_matrix(const char *path, int n) {
    FILE *file = fopen(path, "r");

    if (file == NULL) {
        return NULL;
    }

    double *matrix = malloc((size_t)n * n * sizeof(double));

    if (matrix == NULL) {
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (fscanf(file, "%lf,", &matrix[i * n + j]) != 1) {
                free(matrix);
                fclose(file);
                return NULL;
            }
        }
    }

    fclose(file);
    return matrix;
}
