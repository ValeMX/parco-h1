#include <immintrin.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "functions.h"

#define CODE "BP"

#define BLOCK_SIZE 32

static inline const double* next_block(const double* M, int n, int rb, int cb) {
    int size = BLOCK_SIZE < n ? BLOCK_SIZE : n;
    int cb1 = cb + 1;
    int rb1 = rb;

    if (cb1 == n / size) {
        rb1 += 1;
        cb1 = 0;
    }
    return M + (rb1 * n + cb1) * BLOCK_SIZE;
}

bool checkSymImp(const double* M, int n) {
    const int size = BLOCK_SIZE < n ? BLOCK_SIZE : n;
    bool check = true;

    for (int rb = 0; rb < n / size; rb++) {                      // row blocks indexing
        for (int cb = 0; cb <= rb; cb++) {                       // column blocks indexing
            const double* first = M + (rb * n + cb) * size;      // block matrix of lower triangular part
            const double* last = M + (cb * n + rb) * size;       // block matrix of higher triangular part
            const double* prf_first = next_block(M, n, rb, cb);  // prefetching address of lower triangular part
            const double* prf_last = next_block(M, n, cb, rb);   // prefetching address of higher triangular part
            for (int i = 0; i < size; i++) {                     // symmetry check
                _mm_prefetch(prf_first + i * n, _MM_HINT_NTA);
                _mm_prefetch(prf_last + i * n, _MM_HINT_NTA);
                for (int j = 0; j < size; j++) {
                    if (fabs(first[i * n + j] - last[j * n + i]) > EPSILON) {
                        check = false;
                    }
                }
            }
        }
    }

    return check;
}

void matTransposeImp(const double* M, double* T, int n) {
    const int size = BLOCK_SIZE < n ? BLOCK_SIZE : n;

    for (int rb = 0; rb < n / size; rb++) {                   // row blocks indexing
        for (int cb = 0; cb < n / size; cb++) {               // column blocks indexing
            const double* source = M + (rb * n + cb) * size;  // block matrix of lower triangular part
            double* destination = T + (cb * n + rb) * size;   // block matrix of higher triangular part
            const double* prf_source = next_block(M, n, rb, cb);
            for (int i = 0; i < size; i++) {  // transposing
                _mm_prefetch(prf_source + i * n, _MM_HINT_NTA);
                for (int j = 0; j < size; j++) {
                    destination[i * n + j] = source[j * n + i];
                }
            }
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