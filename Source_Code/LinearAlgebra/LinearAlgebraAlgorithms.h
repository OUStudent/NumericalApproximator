/*********
 * Author: Brandon Morgan
 * Last Updates 5/21/2020
 * Description: 
 *       Library full of linear algebra functions for solving systems of equations, performing gaussian elimination,
 *       rotations or transformations onto matrices, and most notable solving and finding the eigen values and
 *       eigen vectors of a symmetric matrix.
 * 
 * Example Use in external File:
 *      // Assuming Matrix myMat has values when initialized
 *          Matrix *myMat;
 *          Matrix *b;
 *          LUReturn *myLU = lu_decompose(myMat, b);
 *          matrix_destructor(&myMat); 
 * 
 *  ******/

#ifndef LAALGORITHMS_H
#define LAALGORITHMS_H

#include "Matrix.h"

/***
 * 
 * Data Structure Definitions only Below:
 * 
 * ***/ 

typedef struct LUReturn LUReturn;

typedef struct GrahmSchmidt GrahmSchmidt;

typedef struct Eigen Eigen;


/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// LU Factorization

void lu_destructor(LUReturn** myLU);
LUReturn* lu_decompose(Matrix* m1, Matrix *b);
Matrix* lu_getL(LUReturn* myLU);
Matrix* lu_getU(LUReturn* myLU);
Matrix* lu_getSol(LUReturn* myLU);
int lu_solve(LUReturn *myLU, Matrix* b);
// no initializer

// Grahm Schmidt

void grahmschmidt_destructor(GrahmSchmidt** myQR);
void grahmschmidt_initialize(GrahmSchmidt** myGS);
void grahmschmidt_QR(GrahmSchmidt *gs, Matrix *A, Matrix *b);
int grahmschmidt_solve(GrahmSchmidt* myGS, Matrix *B);
Matrix* grahmschmidt_getQ(GrahmSchmidt *myGS);
Matrix* grahmschmidt_getR(GrahmSchmidt *myGS);
Matrix* grahmschmidt_getSol(GrahmSchmidt *myGS);

// Algorithms/Transformations/Rotations

Matrix* LAalgorithms_HouseHoldersMethod(Matrix *a);
Matrix* LAalgorithms_naiveGaussElim(Matrix *m1, Matrix *m2);
Matrix* LAalgorithms_forwSub(Matrix *A, Matrix *b);
Matrix* LAalgorithms_backSub(Matrix *A, Matrix *b);
int LAalgorithms_isPositiveDefinite(Matrix *a);
Matrix* LAalgorithms_findInverse(Matrix *a);

// Eigen vectors/values

void eigen_destructor(Eigen **e);
int eigen_getLength(Eigen *e);
Eigen* eigen_Solve(Matrix *a);
Matrix* eigen_getValue(Eigen *e);
Matrix* eigen_getVector(Eigen *e);
Eigen* eigen_copy(Eigen *e);
// No initializer

#endif // LAALGORITHMS_H
