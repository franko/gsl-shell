#define min(x,y) (((x) < (y)) ? (x) : (y))

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

int main() {
    double *A = NULL, *B = NULL, *C = NULL;
    int m, n, k, i;
    double alpha, beta;

    printf ("\n This example computes real matrix C=alpha*A*t(B)+beta*C using \n"
        " BLAS function dgemm, where A, B, and  C are matrices and \n"
        " alpha and beta are double precision scalars\n\n");

    m = 2000, k = 200, n = 1000;
    printf (" Initializing data for matrix multiplication C=A*t(B) for matrix \n"
        " A(%ix%i) and matrix B(%ix%i)\n\n", m, k, n, k);
    alpha = 1.0; beta = 0.0;

    printf (" Allocating memory for matrices \n\n");
    A = (double *) malloc( m*k*sizeof( double ) );
    B = (double *) malloc( n*k*sizeof( double ) );
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
        int col = i / n, row = i % n;
        int element_index = row * k + col;
        B[element_index] = (double)(-i-1);
    }

    for (i = 0; i < (m*n); i++) {
        C[i] = 0.0;
    }

    printf (" Computing matrix product using BLAS dgemm function via CBLAS interface \n\n");
    cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasTrans, 
                m, n, k, alpha, A, k, B, k, beta, C, n);
    printf ("\n Computations completed.\n\n");

    printf (" Top left corner of matrix A: \n");
    print_matrix_top_left(A, m, k, 6, 6);

    printf ("\n Top left corner of matrix B: \n");
    print_matrix_top_left(B, n, k, 6, 6);
    
    printf ("\n Top left corner of matrix C: \n");
    print_matrix_top_left(C, m, n, 6, 6);

    printf ("\n Deallocating memory \n\n");
    free(A);
    free(B);
    free(C);

    printf (" Example completed. \n\n");
    return 0;
}
