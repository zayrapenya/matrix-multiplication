#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double *multiply(const double *A, const double *B, int n);

static int equal(double a, double b) {
    return fabs(a - b) < 1e-9;
}

int main(void) {

    /* Test 1: known 2x2 multiplication */
    {
        double A[] = {
            1.0, 2.0,
            3.0, 4.0
        };

        double B[] = {
            5.0, 6.0,
            7.0, 8.0
        };

        double expected[] = {
            19.0, 22.0,
            43.0, 50.0
        };

        double *C = multiply(A, B, 2);

        if (C == NULL) {
            printf("Error: memory allocation failed.\n");
            return 1;
        }

        for (int i = 0; i < 4; i++) {
            if (!equal(C[i], expected[i])) {
                printf("Known 2x2 test failed.\n");
                free(C);
                return 1;
            }
        }

        free(C);
    }

    /* Test 2: identity matrix */
    {
        double A[] = {
            1.0, 2.0,
            3.0, 4.0
        };

        double I[] = {
            1.0, 0.0,
            0.0, 1.0
        };

        double *C = multiply(A, I, 2);

        if (C == NULL) {
            return 1;
        }

        for (int i = 0; i < 4; i++) {
            if (!equal(C[i], A[i])) {
                printf("Identity test failed.\n");
                free(C);
                return 1;
            }
        }

        free(C);
    }

    /* Test 3: zero matrix */
    {
        double A[] = {
            1.0, 2.0,
            3.0, 4.0
        };

        double Z[] = {
            0.0, 0.0,
            0.0, 0.0
        };

        double *C = multiply(A, Z, 2);

        if (C == NULL) {
            return 1;
        }

        for (int i = 0; i < 4; i++) {
            if (!equal(C[i], 0.0)) {
                printf("Zero matrix test failed.\n");
                free(C);
                return 1;
            }
        }

        free(C);
    }

    printf("All C correctness tests passed.\n");

    return 0;
}
