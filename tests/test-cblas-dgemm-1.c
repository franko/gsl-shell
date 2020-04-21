#define min(x,y) (((x) < (y)) ? (x) : (y))

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "cblas.h"

static void print_matrix_top_left(double *A, int m, int n, int m_limit, int n_limit) {
    for (int i=0; i<min(m,m_limit); i++) {
        for (int j=0; j<min(n,n_limit); j++) {
            printf ("%12.0f", A[j+i*n]);
        }
        printf ("\n");
    }
}

static int check_matrix_values_top_left(double *A, int m, int n, int m_limit, int n_limit, const double *A_ref, const double eps_abs, const double eps_rel) {
    int difference_seen = 0;
    for (int i=0; i<min(m,m_limit); i++) {
        for (int j=0; j<min(n,n_limit); j++) {
            const double element_ref = A_ref[j+i*n_limit];
            const double elements_diff = A[j+i*n] - element_ref;
            if (fabs(elements_diff) > eps_abs || 
                fabs(elements_diff / element_ref) > eps_rel) {
                difference_seen += 1;
                printf ("Difference for element: (%d; %d) value: %12.0f expected: %12.0f\n", i, j, A[j+i*n], element_ref);
            }
        }
    }
    return difference_seen;
}

int main() {
    double *A = NULL, *B = NULL, *C = NULL;
    int m, n, k, i;
    double alpha, beta;

    printf ("\n This example computes real matrix C=alpha*A*B+beta*C using \n"
        " BLAS function dgemm, where A, B, and  C are matrices and \n"
        " alpha and beta are double precision scalars\n\n");

    m = 2000, k = 200, n = 1000;
    printf (" Initializing data for matrix multiplication C=A*B for matrix \n"
        " A(%ix%i) and matrix B(%ix%i)\n\n", m, k, k, n);
    alpha = 1.0; beta = 0.0;

    printf (" Allocating memory for matrices  \n\n");
    A = (double *) malloc( m*k*sizeof( double ) );
    B = (double *) malloc( k*n*sizeof( double ) );
    C = (double *) malloc( m*n*sizeof( double ) );
    if (A == NULL || B == NULL || C == NULL) {
        printf( "\n ERROR: Can't allocate memory for matrices. Aborting... \n\n");
        free(A);
        free(B);
        free(C);
        return 1;
    }

    printf (" Intializing matrix data \n\n");
    for (i = 0; i < (m*k); i++) {
        A[i] = (double)(i+1);
    }

    for (i = 0; i < (k*n); i++) {
        B[i] = (double)(-i-1);
    }

    for (i = 0; i < (m*n); i++) {
        C[i] = 0.0;
    }

    printf (" Computing matrix product using BLAS dgemm function via CBLAS interface \n\n");
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, 
                m, n, k, alpha, A, k, B, n, beta, C, n);
    printf ("\n Computations completed.\n\n");

    printf (" Top left corner of matrix A: \n");
    print_matrix_top_left(A, m, k, 6, 6);

    printf ("\n Top left corner of matrix B: \n");
    print_matrix_top_left(B, k, n, 6, 6);
    
    printf ("\n Top left corner of matrix C: \n");
    print_matrix_top_left(C, m, n, 6, 6);

    const double eps_abs = 100, eps_rel = 1e-8;
    const double C_expect[36] = {
        -2666620100, -2666640200, -2666660300, -2666680400, -2666700500, -2666720600,
        -6646660100, -6646720200, -6646780300, -6646840400, -6646900500, -6646960600,
        -10626700100, -10626800200, -10626900300, -10627000400, -10627100500, -10627200600,
        -14606740100, -14606880200, -14607020300, -14607160400, -14607300500, -14607440600,
        -18586780100, -18586960200, -18587140300, -18587320400, -18587500500, -18587680600,
        -22566820100, -22567040200, -22567260300, -22567480400, -22567700500, -22567920600,
    };
    int check = check_matrix_values_top_left(C, m, n, 6, 6, C_expect, eps_abs, eps_rel);

    printf ("\n Deallocating memory \n\n");
    free(A);
    free(B);
    free(C);

    printf (" Example completed. \n\n");
    return (check == 0 ? 0 : 1);
}
