#include <immintrin.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "functions.h"

#define CODE "OBT"

#define BLOCK_SIZE 32

bool checkSymOMP(const double* M, int n) {
    const int size = BLOCK_SIZE < n ? BLOCK_SIZE : n / omp_get_num_threads();
    const int num_blocks = n / size;
    bool check = true;

#pragma omp parallel reduction(&& : check)
    {
        bool c = true;

        const double* first;
        const double* last;

#pragma omp for
        for (int k = 0; k < (num_blocks * (num_blocks + 1)) / 2; k++) {
            int rb = (int)((-1 + sqrt(1 + 8 * k)) / 2);  // row blocks indexing
            int cb = k - (rb * (rb + 1)) / 2;            // column blocks indexing
            first = M + (rb * n + cb) * size;            // block matrix of lower triangular part
            last = M + (cb * n + rb) * size;             // block matrix of higher triangular part
            for (int i = 0; i < size; i++) {             // symmetry check
                for (int j = 0; j < size; j++) {
                    if (fabs(first[i * n + j] - last[j * n + i]) > EPSILON) {
                        c = false;
                    }
                }
            }
        }

        check = check && c;
    }

    return check;
}

void matTransposeOMP(const double* M, double* T, int n) {
    const int size = BLOCK_SIZE < n ? BLOCK_SIZE : n / omp_get_num_threads();
#pragma omp parallel
    {
        const double* source;
        double* destination;
#pragma omp allocate(source, destination) allocator(omp_thread_mem_alloc)

#pragma omp for collapse(2)
        for (int rb = 0; rb < n / size; rb++) {          // row blocks indexing
            for (int cb = 0; cb < n / size; cb++) {      // column blocks indexing
                source = M + (rb * n + cb) * size;       // block matrix of lower triangular part
                destination = T + (cb * n + rb) * size;  // block matrix of higher triangular part
                for (int i = 0; i < size; i++) {         // transposing
                    for (int j = 0; j < size; j++) {
                        destination[j * n + i] = source[i * n + j];
                    }
                }
            }
        }
    }
}

// The attribute is necessary to avoid the compiler optimization on the repetitions loops
int __attribute__((optimize("O0"))) main(int argc, char** argv) {
#ifdef _OPENMP
    printf("Compiled with OpenMP %d.%02d\n", _OPENMP / 100, _OPENMP % 100);
#else
    printf("Error: compile with -fopenmp flag");
#endif

    int dim = 0;
    int rep = 0;
    int threads = 0;
    if (argc < 2) {
        printf("Correct usage: program-name [M dimension as exponent of 2] [number of repetitions (default 5)] [number of threads (0 to run all cases)]\n\n");
        return 1;
    } else if (argc == 2) {
        dim = atoi(argv[1]);
        rep = 500;
        threads = 0;
    } else if (argc == 3) {
        dim = atoi(argv[1]);
        rep = atoi(argv[2]) > 0 ? atoi(argv[2]) : 500;
        threads = 0;
    } else {
        dim = atoi(argv[1]);
        rep = atoi(argv[2]) > 0 ? atoi(argv[2]) : 500;
        threads = atoi(argv[3]) >= 0 ? atoi(argv[3]) : 0;
    }

    unsigned int n = pow(2, dim);
    printf("Matrix dimension: %d\n", n);
    printf("Repetitions: %d\n\n", rep);

    // Variables declaration
    double ts1, ts2, te1, te2, t1, t2, s1, s2;  // execution times
    bool symmetric = false;                     // symmetry check
    double* M;                                  // input matrix
    double* T;                                  // transposed matrix

    // Matrices allocation
    if (initMatrices(&M, &T, n) == -1) {
        printf("Error in allocating matrices!\n\n");
        return -1;
    }

    omp_set_num_threads(1);

    ts1 = omp_get_wtime();
    for (int i = 0; i < rep; i++) symmetric = checkSymOMP(M, n);
    te1 = omp_get_wtime();

    ts2 = omp_get_wtime();
    for (int i = 0; i < rep; i++) matTransposeOMP(M, T, n);
    te2 = omp_get_wtime();

    s1 = (te1 - ts1) / rep;
    s2 = (te2 - ts2) / rep;

    printf("Sequential execution: symmetry: %s\n", symmetric ? "true" : "false");
    printf("checkSymOMP:\t%.9f seconds\n", s1);
    printf("matTransposeOMP:%.9f seconds\n\n", s2);

    printf("Results: threads (T), symmetry (S), ");
    printf("symmetry speedup (SS), symmetry efficiency (SE), transpose speedup (TS), transpose efficiency (TE), transpose bandwidth in GB/s (TB) \n\n");
    printf("\t|\tT\t|\tS\t|\tSS\t|\tSE\t|\tTS\t|\tTE\t|\tTB\t|\n");
    
    double bandwidth = 2 * n * n * sizeof(double) / (s2);
    if (saveResultsOMP(CODE, n, 1, 1, 100, 1, 100, bandwidth) == -1) {
        printf("Error in saving results!\n\n");
    }

    // Executions
    if (threads == 0) {
        for (int i = 2; i <= 64; i *= 2) {
            omp_set_num_threads(i);

            ts1 = omp_get_wtime();
            for (int j = 0; j < rep; j++) symmetric = checkSymOMP(M, n);
            te1 = omp_get_wtime();

            ts2 = omp_get_wtime();
            for (int j = 0; j < rep; j++) matTransposeOMP(M, T, n);
            te2 = omp_get_wtime();

            // Results printing and saving
            t1 = (te1 - ts1) / rep;
            t2 = (te2 - ts2) / rep;

            double speedup1 = (double)s1 / t1;
            double efficiency1 = (double)speedup1 / i * 100;
            double speedup2 = (double)s2 / t2;
            double efficiency2 = (double)speedup2 / i * 100;
            double bandwidth = 2 * n * n * sizeof(double) / (t2);

            printf("\t| %d\t\t| %d\t\t| %8.4f\t| %8.4f\t| %8.4f\t| %8.4f\t| %8.4f\t|\n", i, symmetric, speedup1, efficiency1, speedup2, efficiency2, bandwidth * 1e-9);

            if (saveResultsOMP(CODE, n, i, speedup1, efficiency1, speedup2, efficiency2, bandwidth) == -1) {
                printf("Error in saving results!\n\n");
            }
        }
    } else {
        omp_set_num_threads(threads);
        ts1 = omp_get_wtime();
        for (int j = 0; j < rep; j++) symmetric = checkSymOMP(M, n);
        te1 = omp_get_wtime();

        ts2 = omp_get_wtime();
        for (int j = 0; j < rep; j++) matTransposeOMP(M, T, n);
        te2 = omp_get_wtime();

        // Results printing and saving
        t1 = (te1 - ts1) / rep;
        t2 = (te2 - ts2) / rep;

        double speedup1 = (double)s1 / t1;
        double efficiency1 = (double)speedup1 / threads * 100;
        double speedup2 = (double)s2 / t2;
        double efficiency2 = (double)speedup2 / threads * 100;
        double bandwidth = 2 * n * n * sizeof(double) / (t2);

        printf("\t| %d\t\t| %d\t\t| %8.4f\t| %8.4f\t| %8.4f\t| %8.4f\t| %8.4f\t|\n", threads, symmetric, speedup1, efficiency1, speedup2, efficiency2, bandwidth);

        if (saveResultsOMP(CODE, n, threads, speedup1, efficiency1, speedup2, efficiency2, bandwidth) == -1) {
            printf("Error in saving results!\n\n");
        }
    }
    // --------------------------------

    printf("\n");
    testResults(M, T, n);

    // Matrices deallocation
    free(M);
    free(T);

    return 0;
}