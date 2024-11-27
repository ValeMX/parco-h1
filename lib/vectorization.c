#include <immintrin.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "functions.h"

#define CODE "V"

bool checkSymImp(const double* M, int n) {
    bool check = true;

    __m256d epsilon_vec = _mm256_set1_pd(EPSILON);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j += 4) {
            __m256d row = _mm256_loadu_pd(&M[i * n + j]);    // load row values
            __m256d col = _mm256_set_pd(M[(j + 3) * n + i],  // load column values
                                        M[(j + 2) * n + i],
                                        M[(j + 1) * n + i],
                                        M[j * n + i]);
            __m256d diff = _mm256_sub_pd(row, col);                                           // subtraction
            __m256d abs_diff = _mm256_andnot_pd(_mm256_set1_pd(-0.0), diff);                  // absolute value
            int mask = _mm256_movemask_pd(_mm256_cmp_pd(abs_diff, epsilon_vec, _CMP_LE_OQ));  // comparison with epsilon and converting in 4 bit mask

            // (mask == 0b1111 = 15 if all comparisons are true)
            if (mask != 15) {
                check = false;
            }
        }
    }

    return check;
}

void matTransposeImp(const double* M, double* T, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            T[j * n + i] = M[i * n + j];
        }
    }
}

// The attribute is necessary to avoid the compiler optimization on the repetitions loops
int __attribute__((optimize("O0"))) main(int argc, char** argv) {
    int dim = 0;
    int rep = 0;
    if (argc < 2) {
        printf("Correct usage: program-name [M dimension as exponent of 2] [number of repetitions (default 5)]");
        return 1;
    } else if (argc == 2) {
        dim = atoi(argv[1]);
        rep = 500;
    } else {
        dim = atoi(argv[1]);
        rep = atoi(argv[2]) > 0 ? atoi(argv[2]) : 500;
    }

    unsigned int n = pow(2, dim);
    printf("Matrix dimension: %d\n\n", n);
    printf("Repetitions: %d\n\n", rep);

    // Variables declaration
    struct timespec s1, s2, e1, e2;   // start, end times
    double t1, t2, flops, bandwidth;  // execution times and performance metrics
    bool symmetric = false;           // symmetry check
    double* M;                        // input matrix
    double* T;                        // transposed matrix

    // Matrices allocation
    if (initMatrices(&M, &T, n) == -1) {
        printf("Error in allocating matrices!\n\n");
        return -1;
    }

    // Execution
    clock_gettime(CLOCK_MONOTONIC, &s1);
    for (int i = 0; i < rep; i++) symmetric = checkSymImp(M, n);
    clock_gettime(CLOCK_MONOTONIC, &e1);

    clock_gettime(CLOCK_MONOTONIC, &s2);
    for (int i = 0; i < rep; i++) matTransposeImp(M, T, n);
    clock_gettime(CLOCK_MONOTONIC, &e2);
    // --------------------------------

    // Results printing and saving
    t1 = elapsedTime(s1, e1) / rep;
    t2 = elapsedTime(s2, e2) / rep;

    flops = (double)((n * n) / 2 - n) / t1;
    bandwidth = (double)(2 * n * n * sizeof(double)) / t2;

    printf("Sequential execution: symmetry: %s\n", symmetric ? "true" : "false");
    printf("checkSym:\t%.9f seconds\t%10.4g GFLOPS\n", t1, flops * 1e-9);
    printf("matTranspose:\t%.9f seconds\t%10.4g GB/s\n\n", t2, bandwidth * 1e-9);

    testResults(M, T, n);

    if (saveResultsILP(CODE, n, flops, bandwidth) == -1) {
        printf("Error in saving results!\n\n");
    }

    // Matrices deallocation
    free(M);
    free(T);

    return 0;
}