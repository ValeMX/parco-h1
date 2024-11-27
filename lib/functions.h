#include <omp.h>
#include <stdio.h>
#include <unistd.h>

#define EPSILON 1e-6
#define FILE_NAME_ILP "results_ilp.csv"
#define FILE_NAME_OMP "results_omp.csv"

void printMatrix(const double* M, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%10g ", M[i * n + j]);
        }
        printf("\n");
    }
    printf("\n");
}

double elapsedTime(struct timespec s, struct timespec e) {
    long seconds = e.tv_sec - s.tv_sec;
    long nanoseconds = e.tv_nsec - s.tv_nsec;
    double time = seconds + nanoseconds * 1e-9;
    return time;
}

int saveResultsILP(const char* code, int n, double flops, double bandwidth) {
    FILE* f = fopen(FILE_NAME_ILP, "a");

    if (f == NULL) return -1;

    fprintf(f, "%s,%d,%.9f,%.9f\n", code, n, flops, bandwidth);
    fclose(f);

    return 0;
}

int saveResultsOMP(const char* code, int n, int threads, double speedup1, double efficiency1, double speedup2, double efficiency2, double bandwidth) {
    FILE* f = fopen(FILE_NAME_OMP, "a");

    if (f == NULL) return -1;

    fprintf(f, "%s,%d,%d,%.9f,%.9f,%.9f,%.9f,%.9f\n", code, n, threads, speedup1, efficiency1, speedup2, efficiency2, bandwidth);
    fclose(f);

    return 0;
}

int initMatrices(double** M, double** T, int n) {
    *M = (double*)malloc(n * n * sizeof(double));
    *T = (double*)malloc(n * n * sizeof(double));

    if (*M == NULL || *T == NULL) {
        free(*M);
        free(*T);
        return -1;
    }

    srand(time(0));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            (*M)[i * n + j] = (double)rand() / RAND_MAX * 100;
        }
    }

    return 0;
}

void testResults(double* M, double* T, int n) {
    bool check = true;
    bool transposed = true;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (fabs(M[i * n + j] - M[j * n + i]) > EPSILON) {
                check = false;
            }

            if (T[j * n + i] != M[i * n + j]) {
                transposed = false;
            }
        }
    }

    printf("Tested results: symmetry %s and transposed %s.\n\n", check ? "true" : "false", transposed ? "correct" : "incorrect");
}