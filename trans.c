/*
 * trans.c - Matrix transpose B = A^T
 * 
 * Author: enhanc
 *
 * Each transpose function must have a prototype of the form:
 * void trans(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp);
 * A is the source matrix, B is the destination
 * tmp points to a region of memory able to hold TMPCOUNT (set to 256) doubles as temporaries
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 2KB direct mapped cache with a block size of 64 bytes.
 *
 * Programming restrictions:
 *   No out-of-bounds references are allowed
 *   No alterations may be made to the source array A
 *   Data in tmp can be read or written
 *   This file cannot contain any local or global doubles or arrays of doubles
 *   You may not use unions, casting, global variables, or
 *     other tricks to hide array data in other forms of local or global memory.
 */
#include <stdio.h>
#include <stdbool.h>
#include "cachelab.h"
#include "contracts.h"

#define SQUARE_MATRIX_BLOCK 8
#define RECT_MATRIX_BLOCK 4
/* Forward declarations */
static int findMin(int a, int b);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
static const char transpose_submit_desc[] = "Transpose submission";


static void transpose_submit(size_t M, size_t N, const double A[N][M], double B[M][N], double *tmp)
{
    /*
     * This is a good place to call your favorite transposition functions
     * It's OK to choose different functions based on array size, but
     * your code must be correct for all values of M and N
     */
    if (M == 32 && N == 32){

        int i, j, k, l;
    
        for (i = 0; i < M; i += SQUARE_MATRIX_BLOCK) {
            for (j = 0; j < N; j += SQUARE_MATRIX_BLOCK ) {
                for (k = i ; k < i + SQUARE_MATRIX_BLOCK; k++) {
                    for (l = j; l < j + SQUARE_MATRIX_BLOCK; l++) {
                        if (i == j) {
                        // diagonal, put a row of block into tmp
                        *(tmp + SQUARE_MATRIX_BLOCK * (i + 1) + l) = A[k][l];
                       } else {
                        B[l][k] = A[k][l];
                       }
                    }
                    if (i == j) {
                        //now put into a column of block from tmp
                        for (l = j; l < j + SQUARE_MATRIX_BLOCK; l++) {
                            B[l][k] = *(tmp + SQUARE_MATRIX_BLOCK * (i + 1) + l);
                        }
                    }
                }
            }
        }
    }
    else {
        int i, j, k, l;
        
        for (i = 0; i < N; i += RECT_MATRIX_BLOCK) {
            for (j = 0; j < M; j += RECT_MATRIX_BLOCK ) {
                for (k = i ; k < findMin(N,i + RECT_MATRIX_BLOCK); k++) {
                    for (l = j; l < findMin(M,j + RECT_MATRIX_BLOCK); l++) {
                        B[l][k] = A[k][l];
                       }
                    }
                }
            }
        }
    }
/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions(void)
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc);
}

static int findMin(int a, int b) {
    if (a < b) return a;
    return b;
}
