
#include "../LinearAlgebraAlgorithms.h"

/***
 * 
 * Data Structures only Below:
 * 
 * ***/ 

struct LUReturn {
    Matrix* upper;
    Matrix* lower;
    Matrix* solution;
};

struct GrahmSchmidt {
    Matrix *Q;
    Matrix *R;
    Matrix *solution;
};

struct Eigen {
    int size;
    Matrix *values; // 1xn matrix
    Matrix *vectors; // ?xn matrix
};

/****
 * 
 * Function declarations and implementations below:
 * 
 * ***/

static Matrix* eigenSolvHelper(Matrix *augT);


/**
 * 
 * LU Decomposition
 * 
 *  ***/

// Performs LU decomposition and stores the upper and lower triangles into LUReturn struct
// Returns NULL if param matrix is not square
// Also solves system if Matrix *b is given; if b is not known pass in NULL
LUReturn* lu_decompose(Matrix* m1, Matrix *b) {
    if(!matrix_isSquare(m1)) return NULL;
    int n = matrix_ncol(m1);
    Matrix *upper;
    Matrix* lower;
    matrix_initialize(&upper, n, n, 0);
    matrix_initialize(&lower, n, n, 0);

    for(int i = 0; i < n; i++) {
        for(int k = i; k < n; k++) {
            double sum = 0;
            for(int j = 0; j < i; j++) {
                sum += (matrix_get(lower, i, j)*matrix_get(upper, j, k));
            }
            matrix_set(upper, i, k , matrix_get(m1, i, k) - sum);
        }
        for(int k = i; k < n; k++) {
            if(i==k)  matrix_set(lower, i, i, 1); // Diagonals are 1
            else {
                double sum = 0; 
                for(int j = 0; j < i; j++) {
                    sum += (matrix_get(lower, k, j) * matrix_get(upper, j, i));
                }
                matrix_set(lower, k, i, (matrix_get(m1, k, i)-sum) / matrix_get(upper, i, i));
            }
        }
    }
    LUReturn* re = (LUReturn*) malloc(sizeof(LUReturn));
    re->upper = upper;
    re->lower = lower;
    re->solution = NULL;
    lu_solve(re, b);
    return re;
}

// Destructor 
void lu_destructor(LUReturn** myLU) {
    matrix_destructor(&((*myLU)->upper));
    matrix_destructor(&((*myLU)->lower));
    if((*myLU)->solution !=NULL) matrix_destructor(&((*myLU)->solution));
    free(*myLU);
}

// Given an LU data structure, solve the system given b
// if previous solution is detected, it is freed
// returns 1 if succesful; 0 otherwise
int lu_solve(LUReturn *myLU, Matrix* b) {
    if(b==NULL || myLU==NULL) return 0;
    Matrix *y = LAalgorithms_forwSub(myLU->lower, b);
    Matrix *x = LAalgorithms_backSub(myLU->upper, y);
    matrix_destructor(&y);
    if(myLU->solution!=NULL) matrix_destructor(&(myLU->solution));
    myLU->solution = x;
    return 1;
}

// Returns a copy of the lower triangular matrix L
// This returned matrix will need to called in destructor at some point
Matrix* lu_getL(LUReturn* myLU) {
    Matrix *temp;
    matrix_initializeFromMatrix(&temp, myLU->lower);
    return temp;
}

// Returns a copy of the upper triangular matrix U
// This returned matrix will need to called in destructor at some point
Matrix* lu_getU(LUReturn* myLU) {
    Matrix *temp;
    matrix_initializeFromMatrix(&temp, myLU->upper);
    return temp;
}

// Returns a copy of solution matrix 
// Returns NULL if no solution is present
// This returned matrix will need to called in destructor at some point
Matrix* lu_getSol(LUReturn* myLU) {
    Matrix *temp;
    matrix_initializeFromMatrix(&temp, myLU->solution);
    return temp;
}

/**
 * 
 * Grahm Schmidt QR
 * 
 *  ***/

// Destructor 
void grahmschmidt_destructor(GrahmSchmidt** myQR) {
    matrix_destructor(&((*myQR)->Q));
    matrix_destructor(&((*myQR)->R));
    if((*myQR)->solution!=NULL)  matrix_destructor(&((*myQR)->solution));
    free(*myQR);
}

// Initializer for GrahmSchmidt solution
void grahmschmidt_initialize(GrahmSchmidt** myGS) {
    (*myGS) = (GrahmSchmidt*) malloc(sizeof(GrahmSchmidt));
    (*myGS)->solution = NULL;
    (*myGS)->R = NULL;
    (*myGS)->Q = NULL;
}

// Performs GrahmSchmidt QR on an arbitrary Matrix A and stores the Q and R
// matrices inside of the passed in gs
// Also calculates the solution if B is given; if B is not known; pass in NULL
void grahmschmidt_QR(GrahmSchmidt *gs, Matrix *A, Matrix *b) {
    Matrix *Q;
    matrix_initializeFromMatrix(&Q, A);
    int n = matrix_ncol(A);
    Matrix *R;
    matrix_initialize(&R, n, n, 0);
    for(int j = 0; j < n; j++) {
        Matrix *q = matrix_getCol(Q, j);
        Matrix *qt = NULL;
        for(int i = 0; i < j; i++) {
            if(q!=NULL) matrix_destructor(&q);
            q = matrix_getCol(Q, j);
            qt = matrix_getCol(Q, i);
            matrix_transpose(qt, 1);
            matrix_set(R, i, j, matrix_dotProduct(qt, q));
            Matrix *temp = matrix_getCol(Q, i);
            matrix_multiplyConstant(temp, -matrix_get(R, i, j), 1);
            Matrix *temp2 = matrix_addMatrices(q, temp);
            matrix_destructor(&q);
            matrix_initializeFromMatrix(&q, temp2);
            matrix_setCol(Q, q, j);
            matrix_destructor(&temp);
            matrix_destructor(&temp2);
            if(qt!=NULL) matrix_destructor(&qt);
        }
        qt = matrix_transpose(q, 0);
        matrix_set(R, j , j, pow(matrix_dotProduct(qt, q), 0.5));
        if(matrix_get(R, j, j) !=0) matrix_multiplyConstant(q, (1/matrix_get(R, j, j)), 1);
        matrix_setCol(Q, q, j);
        if(qt!=NULL) {
            matrix_destructor(&qt);
        }
        if(q!=NULL) {
            matrix_destructor(&q);
        }
    }
    matrix_initializeFromMatrix(&(gs->Q), Q);
    matrix_initializeFromMatrix(&(gs->R), R);
    matrix_destructor(&Q);
    matrix_destructor(&R);
    
    if(b!=NULL) {
        grahmschmidt_solve(gs, b);
    }
    else gs->solution=NULL;
}

// Given a grahm schmit data structure, solve for the solution given B
// if previous solution is present, it is freed
// Return 1 if succesful; 0 otherwise
int grahmschmidt_solve(GrahmSchmidt* myGS, Matrix *B) {
    if(myGS->Q==NULL || myGS->R==NULL) return 0;
    if(B==NULL) return 0;
    Matrix *Qt = matrix_transpose(myGS->Q, 0);
    Matrix *Qtb = matrix_multiplyMatrices(Qt, B);
    Matrix *sol = LAalgorithms_backSub(myGS->R, Qtb);
    matrix_destructor(&Qt);
    matrix_destructor(&Qtb);
    if(myGS->solution !=NULL) matrix_destructor(&(myGS->solution));
    myGS->solution = sol;
    return 1;
}

// Returns a copy of the Q matrix in Grahm Schmidt QR
// This copy will need to have the destructor called upon at some point
// Returns NULL if data structure is NULL
Matrix* grahmschmidt_getQ(GrahmSchmidt *myGS) {
    if(myGS->Q==NULL) return NULL;
    Matrix *temp;
    matrix_initializeFromMatrix(&temp, myGS->Q);
    return temp;
}

// Returns a copy of the R matrix in Grahm Schmidt QR
// This copy will need to have the destructor called upon at some point
// Returns NULL if data structure is NULL
Matrix* grahmschmidt_getR(GrahmSchmidt *myGS) {
    if(myGS->R==NULL) return NULL;
    Matrix *temp;
    matrix_initializeFromMatrix(&temp, myGS->R);
    return temp;
}

// Returns a copy of the solution matrix in Grahm Schmidt QR
// This copy will need to have the destructor called upon at some point
// Returns NULL if data structure is NULL
Matrix* grahmschmidt_getSol(GrahmSchmidt *myGS) {
    if(myGS->solution==NULL) return NULL;
    Matrix *temp;
    matrix_initializeFromMatrix(&temp, myGS->solution);
    return temp;
}

/**
 * 
 * Algorithms/Transformations/Rotations/etc..
 * 
 *  ***/

// Performs forward substitution on a square lower triangular matrix A and a single column of solutions B
// If b is NULL, assumes A is nx(n+1) so b is made from the last column of A
// Returns NULL if an error occured; else the solution in Matrix* form
Matrix* LAalgorithms_forwSub(Matrix *A, Matrix *b) {
    int wasNull = 0;
    int row = matrix_nrow(A);
    int col = matrix_ncol(A);
    if(b==NULL) wasNull =1;
    if(!matrix_isSquare(A)&&!wasNull) return NULL; // A must be square if B is not NULL
    
    if(wasNull) {
        wasNull = 1;
        col--;
        b = matrix_getCol(A, matrix_ncol(A) -1);
    }

    if(matrix_ncol(b) > 1 || matrix_nrow(b) != matrix_nrow(A)) return NULL; // b must be 1 col with nrow= A->nrow
    Matrix *temp;
    matrix_initialize(&temp, matrix_nrow(A), 1, 0);
    for(int i = 0; i < row; i++) {
        for(int j = i+1; j < col; j++) {
            if(matrix_get(A, i, j)!=0 && matrix_get(A, i, j) > 1e-9) { // must be upper triangular
                matrix_destructor(&temp);
                if(wasNull) matrix_destructor(&b);
                return NULL;
            }
        }
    }

    for(int i = 0; i < row; i++) {
        double sum = 0;
        for(int j = 0; j < i; j++) {
            sum+= matrix_get(temp, j, 0) * matrix_get(A, i, j);
        }
        if(matrix_get(A, i, i)==0) matrix_set(temp, i, 0, 0);
        else matrix_set(temp, i, 0, (matrix_get(b, i, 0) - sum) / matrix_get(A, i, i));
        
    }
    if(wasNull) matrix_destructor(&b);
    return temp;
}


// Performs backwards substitution on a square upper triangular matrix A and a single column of solutions B
// If b is NULL, assumes A is nx(n+1) so b is made from the last column of A
// Returns NULL if an error occured; else the solution in Matrix* form
Matrix* LAalgorithms_backSub(Matrix *A, Matrix *b) {
    int wasNull = 0;
    int row = matrix_nrow(A);
    int col = matrix_ncol(A);
    if(b==NULL) wasNull =1;
    if(!matrix_isSquare(A)&&!wasNull) return NULL; // A must be square if B is not NULL
    
    if(wasNull) {
        wasNull = 1;
        col--;
        b = matrix_getCol(A, matrix_ncol(A)-1);
    }

    if(matrix_ncol(b)> 1 || matrix_nrow(b) !=matrix_nrow(A)) return NULL; // b must be 1 col with nrow= A->nrow
    Matrix *temp;
    matrix_initialize(&temp, matrix_nrow(A), 1, 0);
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < i; j++) {
            if(matrix_get(A, i, j)!=0 && matrix_get(A, i, j) > 1e-9) { // must be upper triangular
                matrix_destructor(&temp);
                if(wasNull) matrix_destructor(&b);
                return NULL;
            }
        }
    }
    for(int i = row-1; i>= 0; i--) {
        double sum = 0;
        for(int j = col-1; j > i; j--) {
            sum+= matrix_get(temp, j, 0) * matrix_get(A, i, j);
        }
        if(matrix_get(A, i, i)==0)  matrix_set(temp, i, 0, 0);
        else matrix_set(temp, i, 0, (matrix_get(b, i, 0) - sum) / matrix_get(A, i, i)); 
    }
    if(wasNull) matrix_destructor(&b);
    return temp;
}

// Performs Naive Gaussian Elimination on by combining m1 and m2 to form nx(n+1)
// m1 must be square and m2 must be a single column whose rows match m1
// Returns new matrix if flag is 0; if flag is 1, assigns resulting matrix to param matrix
// if m2 is NULL, m2 is created from the last column of m1 and it is temporarily deleted from m1
Matrix* LAalgorithms_naiveGaussElim(Matrix *m1, Matrix *m2) {
    int wasNULL = 0;
    if(m2==NULL) {
        wasNULL = 1;
        m2 = matrix_getCol(m1, matrix_ncol(m1)-1); 
        m1 = matrix_deleteCol(&m1, matrix_ncol(m1)-1, 0);
    }
    if(!matrix_isSquare(m1)) {
        if(wasNULL) {
            matrix_destructor(&m1);
            matrix_destructor(&m2);
        }
        return NULL;
    }
    if(matrix_ncol(m2) > 1 || matrix_nrow(m2) != matrix_nrow(m1)){
        if(wasNULL) {
            matrix_destructor(&m1);
            matrix_destructor(&m2);
        }
        return NULL;
    }
        Matrix *temp = matrix_insertCol(m1, m2, matrix_ncol(m1), 0);
        int n = matrix_nrow(temp);
        for(int i = 0; i < n-1; i++) {
            for(int k = i+1; k < n; k++) {
                double alpha = - (matrix_get(temp, k, i) / matrix_get(temp, i, i));
                for(int j = 0; j < n+1; j++) {
                    matrix_set(temp, k, j, matrix_get(temp, k, j) + alpha*matrix_get(temp, i, j));
                }
            }
        }
        if(wasNULL) {
            matrix_destructor(&m1);
            matrix_destructor(&m2);
        }
        return temp;
}

Matrix* LAalgorithms_HouseHoldersMethod(Matrix *a) {
    Matrix *A;
    matrix_initializeFromMatrix(&A, a);
    // A needs to be symmetric
    int n = matrix_nrow(A);
    double alpha;
    double r;
    Matrix* w = NULL;
    Matrix* P = NULL;
    double sum;
    Matrix *I;
    matrix_initialize(&I, n, n, 0);
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if(i==j) matrix_set(I, i, j, 1);
        }
    }
    for(int k = 0; k < n-2; k++) {
        matrix_initialize(&w, n, 1, 0);
        sum = 0;
        for(int j = k+1; j < n; j++) {
            sum += pow(matrix_get(A, j, k), 2);
        }
        if(matrix_get(A, k+1, k) < 1e-9) alpha = -pow(sum, 0.5);
        else if(matrix_get(A, k+1, k) < 1) alpha = pow(sum, 0.5);
        else alpha = -pow(sum, 0.5);
        r = pow(0.5*pow(alpha, 2)-0.5*alpha*matrix_get(A, k+1, k), 0.5);
        for(int i = 0; i <= k; i++) {
            matrix_set(w, i, 0, 0);
        }
        matrix_set(w, k+1, 0, (matrix_get(A, k+1, k) -alpha)/(2*r));
        for(int j = k+2; j<n; j++) {
            matrix_set(w, j, 0, matrix_get(A, j, k)/(2*r));
        }
        Matrix *temp  = matrix_transpose(w, 0);
        matrix_multiplyConstant(w, -2, 1);
        Matrix *temp2 = matrix_multiplyMatrices(w, temp);
        P = matrix_addMatrices(I, temp2);
        Matrix *temp3 = matrix_multiplyMatrices(P, A);
        matrix_destructor(&A);
        A = matrix_multiplyMatrices(temp3, P);
        matrix_destructor(&temp);
        matrix_destructor(&temp2);
        matrix_destructor(&temp3);
        matrix_destructor(&P);
        matrix_destructor(&w);
    }
    matrix_destructor(&I);
    return A;
}

// Returns 1 if matrix is positive definite; 0 otherwise
// A positive definite matrix is a symmetric matrix whose eigen values are positve
int LAalgorithms_isPositiveDefinite(Matrix *a) {
    Eigen *e = eigen_Solve(a);
    if(e==NULL) return 0;
    for(int i = 0; i < e->size; i++) {
        if(matrix_get(e->values, 0, i) < 0) {
            eigen_destructor(&e);
            return 0;
        }
    }
    eigen_destructor(&e);
    return 1;
}

// Assumes that an inverse exists, matrix must be square
// Calculates inverse by LU
Matrix* LAalgorithms_findInverse(Matrix *a) {
    LUReturn* myLU = lu_decompose(a, NULL);
    if(myLU==NULL) return NULL;
    int nrow = matrix_nrow(a);
    Matrix *inverse;
    matrix_initialize(&inverse, nrow, nrow, 0);

    for(int i = 0; i <nrow; i++) {
        Matrix *identity;
        matrix_initialize(&identity, nrow, 1, 0);
        matrix_set(identity, i, 0, 1);
        Matrix *L = lu_getL(myLU);
        Matrix *U = lu_getU(myLU);
        Matrix *z = LAalgorithms_forwSub(L, identity);
        Matrix *col = LAalgorithms_backSub(U, z);

        matrix_setCol(inverse, col, i);
        matrix_destructor(&z);
        matrix_destructor(&L);
        matrix_destructor(&U);
        matrix_destructor(&identity);
        matrix_destructor(&col);
    }
    lu_destructor(&myLU);
    return inverse;
}


/**
 * 
 * Eigen Values/Vectors
 * 
 *  ***/

// Destructor
void eigen_destructor(Eigen **e) {
    if(*e==NULL) { // failed eigen
        free(*e);
        return;
    }
    matrix_destructor(&(*e)->values);
    matrix_destructor(&(*e)->vectors);
    free(*e);
}
#include <stdio.h>
// Returns the number of values/vectors in Eigen
int eigen_getLength(Eigen *e) {
    if(e==NULL) printf("NULL\n"); 
    return e->size;
}

// Solves for the given eigen values and eigen vectors of the symmetric matrix a
// First performs a transformation to obtain a house houlders matrix of A,
// then performs grahm schmidt QR until the trace(R) has reached a tolerance of 1e-9,
// where the trace(R) is the eigenvalues of A; then the eigen vectors are obtain by 
// performing Naive Gaussian Elimination on each lambda of the homogenous solution
// (A - lamda * I) * e = 0, then a special type of backwards substitution is used to
// find the unit vectors.
// Returns NULL if a is non symmetric
Eigen* eigen_Solve(Matrix *a) {
    if(!matrix_isSymmetric(a)) return NULL;
    int maxIter = 100;
    Matrix *A;
    matrix_initializeFromMatrix(&A, a);
    double TOLERANCE = 1e-9;
    Matrix* house = LAalgorithms_HouseHoldersMethod(A);
    matrix_destructor(&A);
    GrahmSchmidt* gs;
    grahmschmidt_initialize(&gs);
    grahmschmidt_QR(gs, house, NULL);
    matrix_destructor(&house);
    int nrow = matrix_nrow(gs->R);
    
    for(int i = 0; i < maxIter; i++) {
        double prev = 0;
        for(int j = 0; j < nrow; j++) {
           
            prev +=  matrix_get(gs->R, j, j);
        }
        Matrix *A = matrix_multiplyMatrices(gs->R, gs->Q); 
        grahmschmidt_destructor(&gs);
        grahmschmidt_initialize(&gs);
        grahmschmidt_QR(gs, A, NULL);
        matrix_destructor(&A);
        double now = 0;
        for(int j = 0; j < nrow; j++) {
            now +=  matrix_get(gs->R, j, j);
        }
        if(fabs(prev - now) < TOLERANCE) {
            break;
        }
        
    }

    double eigenValues[nrow];
    for(int i = 0; i < nrow; i++) {
        eigenValues[i] =  matrix_get(gs->R, i, i);
    }
    grahmschmidt_destructor(&gs);
    Eigen *e = (Eigen*) malloc(sizeof(Eigen));
    e->size = nrow;
 //   e->values = malloc(nrow*sizeof(double));
    matrix_initialize(&e->values, 1, nrow, 0);
    matrix_initialize(&e->vectors, nrow, nrow, 0);
    for(int k = 0 ; k < nrow; k++) {
        Matrix *I;
        matrix_initialize(&I, nrow, nrow, 0);
        for(int i = 0; i <nrow; i++) {
            for(int j = 0; j<nrow; j++) {
                if(i==j)  matrix_set(I, i, j, eigenValues[k]);
            }
        }
        Matrix *temp = matrix_minusMatrices(a, I);
        Matrix *b;
        matrix_initialize(&b, nrow, 1, 0);

        Matrix *temp2 = LAalgorithms_naiveGaussElim(temp, b);
        Matrix *sol = eigenSolvHelper(temp2);
        matrix_unitVector(sol, 1);
        matrix_setCol(e->vectors, sol, k);
        matrix_set(e->values, 0, k, eigenValues[k]);
      //  e->values[k] = eigenValues[k];
        matrix_destructor(&sol);
        matrix_destructor(&temp2);
        matrix_destructor(&temp);
        matrix_destructor(&b);
        matrix_destructor(&I);
    }
    return e;
}

// Helper function for solving the homogenous augmented matrix
static Matrix* eigenSolvHelper(Matrix *augT) {
    Matrix *aug;
    matrix_initializeFromMatrix(&aug, augT);
    matrix_deleteCol(&aug, matrix_ncol(aug)-1, 1); // delete 0 matrix

    int numberUnkowns = 1;
    Matrix *b;
    matrix_initialize(&b, matrix_nrow(aug), 1, 0);
    if(matrix_ncol(aug)==2 && matrix_nrow(aug)==2) {
        matrix_set(b, 1, 0, 1);
        matrix_set(b, 0, 0, -matrix_get(aug, 0, 1)/ matrix_get(aug, 0, 0));
        matrix_destructor(&aug);
        return b;
    }
    for(int i = matrix_nrow(aug)-1; i > numberUnkowns; i--) {
        matrix_set(b, i, 0, 1); 
    }

    for(int i = 0; i < matrix_nrow(aug)-2; i++) {
        for(int j = matrix_ncol(aug)-numberUnkowns-1; j >= 0; j--) {
            double alpha = 0;
            for(int k = j+1; k < matrix_ncol(aug); k++) {
                alpha += matrix_get(aug, j, k) * matrix_get(b, k, 0);
            }
            matrix_set(b, j, 0,  -alpha/matrix_get(aug, j, j));
        }
    }
    matrix_destructor(&aug);
    return b;
}

// Returns a copy matrix of the eigenvalue 1xn vector 
// will need to be freed
Matrix* eigen_getValue(Eigen *e) {
    if(e==NULL) return NULL;
    if(e->values==NULL) return NULL;
    Matrix *temp;
    matrix_initializeFromMatrix(&temp, e->values);
    return temp;
}

// Returns copy matrix of the eigenvectors where each
// column is the eigenvector 
// will need to be freed
Matrix* eigen_getVector(Eigen *e) {
    if(e==NULL) return NULL;
    if(e->vectors==NULL) return NULL;
    Matrix *temp;
    matrix_initializeFromMatrix(&temp, e->vectors);
    return temp;
}

// Creates a copy of the eigen struct
// will need to be freed
Eigen* eigen_copy(Eigen *e) {
    Eigen *n = (Eigen*) malloc(sizeof(Eigen));
    n->size = e->size;
    matrix_initializeFromMatrix(&n->values, e->values);
    matrix_initializeFromMatrix(&n->vectors, e->vectors);
    return n;
}
