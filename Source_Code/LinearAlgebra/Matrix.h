/*********
 * Author: Brandon Morgan
 * Last Updates 5/21/2020
 * Description: 
 *       Object like class for storing matrices (2D Arrays), capable of adding, multiplying, transposing
 *       matrices, as well as multiplying matrices by constants. Also capable of adding or inserting, and 
 *       deleting, rows and columns of matrices. Other features are available as seen below...
 * 
 * Example Use in external File:
 *      // These next two lines must always be down before using if initializing from scratch
 *          Matrix *myMat;
 *          matrix_initialize(&myMat, 5, 5, 0); // makes a 5x5 matrix whose values are 0
 *      // On the other hand Matrices can be initialized like the following
 *          Matrix *m2;
 *          matrix_initializeFromMatrix(&m1, myMat); // copies contents of myMat into m1 - we are assumming myMat has been populated with values
 *          // or
 *          double array[][3] = {{1, 2, 3}, {4, 5, 6}};
 *          Matrix *m3
 *          matrix_initializeFromArray(&m3, 2, 3, array);
 *      
 *      // Example function calls using matrices defined earlier
 *          Matrix *temp = matrix_transpose(m3, 1); // flag 0 returns transposed matrix 
 *          Matrix *product = matrix_multiplyMatrices(m3, temp); // 2x3 * 3x2 => 2x2 matrix
 *          matrix_multiplyConstant(product, -1, 1) // Multiplies proudct matirx by -1, flag 1 assigns new matrix to product
 *          matrix_destructor(&temp);
 *          matrix_destructor(&product);
 *          matrix_destructor(&m3); 
 *          matrix_destructor(&myMat); 
 * 
 *  ******/

#ifndef MATRIX_H
#define MATRIX_H

#include "stdlib.h"
#include "stdio.h"
#include "math.h"

/***
 * 
 * Data Structure Definitions only Below:
 * 
 * ***/ 

typedef struct Matrix Matrix;

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

Matrix* matrix_addMatrices(Matrix *m1, Matrix*m2);
Matrix* matrix_addRow(Matrix *m1, int flag); // adds empty Row to end
Matrix* matrix_addCol(Matrix *m1, int flag); // adds empty col to end
Matrix* matrix_addColRow(Matrix* m1, int n, int flag); // adds 'n' empty rows and columns to end
Matrix* matrix_addConstant(Matrix *m1, double constant, int flag);
double matrix_dotProduct(Matrix *m1, Matrix *m2);
void matrix_destructor(Matrix **mat);
Matrix* matrix_deleteRow(Matrix  **m1, int row, int flag);
Matrix* matrix_deleteCol(Matrix  **m1, int col, int flag);
double matrix_get(Matrix *mat, int row, int col);
Matrix* matrix_getCol(Matrix *m1, int col);
Matrix* matrix_getRow(Matrix *m1, int row);
unsigned long matrix_getMallocSizeOf();
void matrix_initialize(Matrix **mat, int nrow, int ncol, double value); // sets all values to 0
void matrix_initializeFromArray(Matrix **mat, int nrow, int ncol, double array[][ncol]);
void matrix_initializeFromMatrix(Matrix **mat, Matrix *old);
void matrix_initializeFromPointer(Matrix **mat, double **ptr, int nrow, int ncol);
Matrix* matrix_insertCol(Matrix *m1, Matrix *m2, int col, int flag); // inserts column m2 into m1 at column index
Matrix* matrix_insertRow(Matrix *m1, Matrix *m2, int row, int flag); // inserts row m2 into m1 at row index
Matrix* matrix_multiplyConstant(Matrix *m1, double c, int flag);
Matrix* matrix_multiplyMatrices(Matrix *m1, Matrix *m2);
Matrix* matrix_minusMatrices(Matrix *m1, Matrix*m2);
int matrix_nrow(Matrix *mat);
int matrix_ncol(Matrix *mat);
void matrix_print(Matrix *m1);
int matrix_set(Matrix *mat, int row, int col, double value);
int matrix_setCol(Matrix *m1, Matrix *m2, int col); // sets the values at column index m2 to m1
int matrix_setRow(Matrix *m1, Matrix *m2, int row); // sets the values at row index m2 to m1
double matrix_sum(Matrix *m1);
double matrix_sumCol(Matrix *m1, int index);
double matrix_sumRow(Matrix *m1, int index);
int matrix_isSquare(Matrix *mat);
int matrix_isSymmetric(Matrix *m1);
Matrix* matrix_transpose(Matrix *mat, int flag);
Matrix* matrix_unitVector(Matrix* a, int flag);

#endif // MATRIX_H

