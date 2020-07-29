
#ifndef MULTIVARIATE_H
#define MULTIVARIATE_H

#include "../LinearAlgebra/LinearAlgebraAlgorithms.h" // this includes Matrix.h
#include "../DataStructures/SinglyLinkedList.h"
#include "Distributions.h"

Matrix* multi_meanVector(Matrix *data);
Matrix* multi_standardize(Matrix *data, Matrix *mean);
Matrix* multi_covariance(Matrix *data, Matrix *mean);
Matrix* multi_correlation(Matrix *data, Matrix *cov);
Matrix* multi_zscores(Matrix *data, Matrix *meanVector, Matrix *cov);

typedef struct PrincipalComponent PrincipalComponent;
PrincipalComponent* pca_solve(Matrix *covCor, Eigen *e);
Matrix* pca_getVariance(PrincipalComponent *myPC);
Matrix* pca_getComponent(PrincipalComponent *myPC);
int pca_getLength(PrincipalComponent *myPC);
double pca_getKthProp(PrincipalComponent *myPC, int index);
void pca_destructor(PrincipalComponent **myPC);

Matrix* statistics_getSquaredDistances(Matrix *data, Matrix *cov, Matrix *mean);

typedef struct Outliers Outliers;
Matrix* outlier_getAllOutlier(Outliers *myOut);
Outliers* outlier_findOutliers(Matrix *data, Matrix *cov, double alpha);
int outlier_getLength(Outliers *myOut);
Matrix* outlier_getOutlier(Outliers *myOut, int index);
Matrix* outlier_removeOutliers(Matrix **data, Outliers *outliers, int flag);
void outlier_destructor(Outliers **myOut);
double outlier_getProb(Outliers *myOut);

struct FactorAnalysis {
    Matrix *communalities;
    Matrix *factorScores;
    Matrix *psi;
    Matrix *rawLoadings;
    Matrix *rotatedLoadings;
    Matrix *residuals;
    Matrix *specificVariances;
    Matrix *data; // a copy of the original data matrix
    double degree; // degree through which the loadings were rotated
};
typedef struct FactorAnalysis FactorAnalysis;
FactorAnalysis* factor_analysis(PrincipalComponent *myPC, Matrix *data,int m, int flag);
void factor_scores(FactorAnalysis *myFacta, Matrix *data, int m, int flag);
void factor_rotate(FactorAnalysis* myFacta, int m, int dir, int flag);
void factor_destructor(FactorAnalysis **myFacta) ;

/**
struct FactorAnalysis {
    Matrix *communalities;
    Matrix *factorScores;
    Matrix *psi;
    Matrix *rawLoadings;
    Matrix *rotatedLoadings;
    Matrix *residuals;
    Matrix *specificVariances;
    double degree; // degree through which the loadings were rotated
};
typedef struct FactorAnalysis FactorAnalysis;

// helper function that performs a varimax rotation
static double factor_varimax(Matrix *mat) {
    int p = matrix_nrow(mat);
    int m = matrix_ncol(mat);
    Matrix *h; 
    matrix_initialize(&h, p, 1, 0);
    double t;
    for(int i = 0; i < p; i++) {
        t = 0;
        for(int j = 0; j < m; j++) {
            t += pow(matrix_get(mat, i, j), 2);
        }
        matrix_set(h, i, 0, t);
    }
    double sum1;
    double sum2;
    double L;
    double sum = 0;
    for(int j = 0; j < m; j++) {
        sum1 = 0;
        sum2 = 0;
        for(int i = 0; i < p; i++) {
            L = matrix_get(mat, i, j) / matrix_get(h, i, 0);
            sum1 += pow(L, 4);
            sum2 += pow(L, 2);
        }
        sum += (sum1-pow(sum2, 2))/p;
    }
    matrix_destructor(&h);
    return sum / p;
}

// max 'm' value of 
static void factor_rotate(FactorAnalysis* myFacta, int m, int dir) {
    Matrix *l = myFacta->rawLoadings; // DO NOT CALL DESTRUCTOR
    static const double DEGREE = 0.0174532925; // in Rad
    double max = 0;
    int d = 0;
    double value;
    Matrix *r;
    if(m==2) matrix_initialize(&r, m, m, 0);
    else if(m==3) matrix_initialize(&r, m, m, 0);

    for(int i = 0; i < 361; i++) {
        if(m==2) {
            matrix_set(r, 0, 0, cos(i*DEGREE));
            matrix_set(r, 0, 1, sin(i*DEGREE));
            matrix_set(r, 1, 0, -sin(i*DEGREE));
            matrix_set(r, 1, 1, cos(i*DEGREE));
        }
        else {
            if(dir==0){ // 'x' dir
                matrix_set(r, 0, 0, 1);
                matrix_set(r, 0, 1, 0);
                matrix_set(r, 0, 2, 0);
                matrix_set(r, 1, 0, 0);
                matrix_set(r, 1, 1, cos(i*DEGREE));
                matrix_set(r, 1, 2, -sin(i*DEGREE));
                matrix_set(r, 2, 0, 0);
                matrix_set(r, 2, 1, sin(i*DEGREE));
                matrix_set(r, 2, 2, cos(i*DEGREE));
            }
            else if(dir==1) { // 'y' dir
                matrix_set(r, 0, 0, cos(i*DEGREE));
                matrix_set(r, 0, 1, 0);
                matrix_set(r, 0, 2, sin(i*DEGREE));
                matrix_set(r, 1, 1, 0);
                matrix_set(r, 1, 1, 1);
                matrix_set(r, 1, 2, 0);
                matrix_set(r, 2, 0, -sin(i*DEGREE));
                matrix_set(r, 2, 1, 0);
                matrix_set(r, 2, 2, cos(i*DEGREE));
            }
            else if(dir==2) { // 'z' dir
                matrix_set(r, 0, 0, cos(i*DEGREE));
                matrix_set(r, 0, 1, -sin(i*DEGREE));
                matrix_set(r, 0, 2, 0);
                matrix_set(r, 1, 0, sin(i*DEGREE));
                matrix_set(r, 1, 1, cos(i*DEGREE));
                matrix_set(r, 1, 2, 0);
                matrix_set(r, 2, 1, 0);
                matrix_set(r, 2, 1, 0);
                matrix_set(r, 2, 2, 1);
            }
            else ; // error
        }
        // set R through DEGREE 'i' mxm dim

        Matrix *temp = matrix_multiplyMatrices(l, r);
        value = factor_varimax(temp);
        printf("Val: %f\n", value);
        if(i==0) max = value;
        else if(value > max) {
            max = value;
            d = i;
        }
        matrix_destructor(&temp);
    }
    if(m==2) {
        matrix_set(r, 0, 0, cos(d*DEGREE));
        matrix_set(r, 0, 1, sin(d*DEGREE));
        matrix_set(r, 1, 0, -sin(d*DEGREE));
        matrix_set(r, 1, 1, cos(d*DEGREE));
    }
    else {
        if(dir==0){ // 'x' dir
            matrix_set(r, 0, 0, 1);
            matrix_set(r, 0, 1, 0);
            matrix_set(r, 0, 2, 0);
                matrix_set(r, 1, 0, 0);
                matrix_set(r, 1, 1, cos(d*DEGREE));
                matrix_set(r, 1, 2, -sin(d*DEGREE));
                matrix_set(r, 2, 0, 0);
                matrix_set(r, 2, 1, sin(d*DEGREE));
                matrix_set(r, 2, 2, cos(d*DEGREE));
            }
            else if(dir==1) { // 'y' dir
                matrix_set(r, 0, 0, cos(d*DEGREE));
                matrix_set(r, 0, 1, 0);
                matrix_set(r, 0, 2, sin(d*DEGREE));
                matrix_set(r, 1, 1, 0);
                matrix_set(r, 1, 1, 1);
                matrix_set(r, 1, 2, 0);
                matrix_set(r, 2, 0, -sin(d*DEGREE));
                matrix_set(r, 2, 1, 0);
                matrix_set(r, 2, 2, cos(d*DEGREE));
            }
            else if(dir==2) { // 'z' dir
                matrix_set(r, 0, 0, cos(d*DEGREE));
                matrix_set(r, 0, 1, -sin(d*DEGREE));
                matrix_set(r, 0, 2, 0);
                matrix_set(r, 1, 0, sin(d*DEGREE));
                matrix_set(r, 1, 1, cos(d*DEGREE));
                matrix_set(r, 1, 2, 0);
                matrix_set(r, 2, 1, 0);
                matrix_set(r, 2, 1, 0);
                matrix_set(r, 2, 2, 1);
            }
            else ; // error
        }
        printf("DEGREE: %d\n", d);
    // set R through degree 'degree'
    Matrix *rotatedloadings = matrix_multiplyMatrices(l, r);
    myFacta->degree = d;
    myFacta->rotatedLoadings = rotatedloadings;
    matrix_destructor(&r);
}
 
 // flag is used to signify whether or not data was standardized before or not
 // if flag is 1, assumes data was transformed into zscores (centered and scaled)
 // if flag is 0, only centers the data
static void factor_scores(FactorAnalysis *myFacta, Matrix *data, int m, int flag) {
    Matrix *t1 = matrix_transpose(myFacta->rawLoadings, 0);
    matrix_print(t1);
    Matrix *t2;
    matrix_initializeFromMatrix(&t2, myFacta->psi);
    matrix_print(t2);
    for(int i = 0; i < matrix_nrow(t2); i++) { // inverse of diagonal matrix
        matrix_set(t2, i, i, 1/matrix_get(t2, i, i));
    }
    matrix_print(t2);
    Matrix *t3 = matrix_multiplyMatrices(t1, t2);
    matrix_print(t3);
    Matrix *t4 = matrix_multiplyMatrices(t3, myFacta->rawLoadings);
    matrix_print(t4);
    Matrix *t5 = LAalgorithms_findInverse(t4);
    matrix_print(t5);

    Matrix *t6 = matrix_multiplyMatrices(t5, t1);
    matrix_print(t6);
    Matrix *t7 = matrix_multiplyMatrices(t6, t2);
    matrix_print(t7);
    matrix_destructor(&t1);
    matrix_destructor(&t2);
    matrix_destructor(&t3);
    matrix_destructor(&t4);
    matrix_destructor(&t5);
    matrix_destructor(&t6);
    
    int n = matrix_nrow(data);
    Matrix *scores;
    if(flag) {
        Matrix *standard = multi_zscores(data, NULL, NULL);
        printf("STANDARDIZED TESTSSS:\n");
        matrix_print(standard);
        printf("END TST  \n");
        matrix_transpose(standard, 1);
        matrix_initialize(&scores, m, n, 0);
        for(int i = 0;i < n; i++) {
            Matrix *dummy = matrix_getCol(standard, i);
            Matrix *T = matrix_multiplyMatrices(t7, dummy);
            matrix_setCol(scores, T, i);
            matrix_destructor(&T);
            matrix_destructor(&dummy);
        }
        matrix_transpose(scores, 1);
        matrix_print(scores);
        printf("End\n");
        matrix_destructor(&standard);
    }
    else { // center data
        Matrix *meanVec = multi_meanVector(data);
        matrix_transpose(meanVec, 1);
        matrix_initialize(&scores, m, n, 0);
        for(int i = 0; i < n; i++) {
            Matrix *tempRow = matrix_getRow(data, i);
            Matrix *dummy = matrix_minusMatrices(tempRow, meanVec);
            matrix_transpose(dummy, 1);
            Matrix *T = matrix_multiplyMatrices(t7, dummy);
            matrix_setCol(scores, T, i);
            matrix_destructor(&T);
            matrix_destructor(&dummy);
            matrix_destructor(&tempRow);
        }
        matrix_transpose(scores, 1);
        matrix_print(scores);
        printf("End\n");
        matrix_destructor(&meanVec);
    }
   
    printf("SCores:\n\n\n");
    matrix_print(scores);
    myFacta->factorScores = scores;

    Matrix *A = matrix_transpose(scores, 0);
    Matrix *a  = matrix_multiplyMatrices(myFacta->rawLoadings, A);
    printf("L * Scores\n");
    matrix_transpose(a, 1); 
    matrix_print(a);
    matrix_destructor(&a);
    matrix_destructor(&A);
    matrix_destructor(&t7);
}

FactorAnalysis* factor_analysis(PrincipalComponent *myPC, Matrix *data, Matrix *covariance, int m) {
    int n = pca_getLength(myPC);
    Matrix *loadings;
    matrix_initialize(&loadings, n, m, 0);
    double value;
    printf("Components:\n");
    for(int i = 0; i < m; i++) { // do columns first
        printf("EigenValue %f\n", pca_getVariance(myPC, i));
        value = pow(pca_getVariance(myPC, i), 0.5);
        Matrix *temp = pca_getComponent(myPC, i);
         matrix_print(temp);
         
         printf("Value: %f \n", value);
        for(int j = 0; j < n; j++) { // iterate through rows per column
            matrix_set(loadings, j, i, value*matrix_get(temp, j, 0));
        }
       
       
        matrix_destructor(&temp);
    }
    printf("Loadings\n");
    matrix_print(loadings);
    int p = n;
    Matrix *psi;
    matrix_initialize(&psi, p, p, 0);
    double sum;
    for(int i = 0; i < p; i++) {
        sum = 0;
        for(int j = 0; j < m; j++) {
            sum += pow(matrix_get(loadings, i, j), 2);
        }
        matrix_set(psi, i, i, matrix_get(covariance, i, i)-sum);
    }
    Matrix *specificVar;
    Matrix *h;
    matrix_initialize(&h, n, 1, 0);
    matrix_initialize(&specificVar, n, 1, 0);
    for(int i = 0; i < n; i++) {
        matrix_set(h, i, 0, 1-matrix_get(psi, i, i));
        matrix_set(specificVar, i, 0, 1-1-matrix_get(psi, i, i)); // 1 -h[i,0];
    }
    Matrix *Lt = matrix_transpose(loadings, 0);
    Matrix *t = matrix_multiplyMatrices(loadings, Lt);
    Matrix *i = matrix_addMatrices(t, psi);
    Matrix *residual = matrix_minusMatrices(covariance, i);

    matrix_destructor(&Lt);
    matrix_destructor(&t);
    matrix_destructor(&i);

    FactorAnalysis *fa = (FactorAnalysis*) malloc(sizeof(FactorAnalysis));
    fa->communalities = h;
    fa->psi = psi;
    fa->specificVariances = specificVar;
    fa->rawLoadings = loadings;
    fa->residuals = residual;
    fa->rotatedLoadings = NULL;
    fa->factorScores = NULL;
    printf("ANALYSIS:\n\n");
    matrix_print(h);
    matrix_print(psi);
    matrix_print(specificVar);
    matrix_print(loadings);
    matrix_print(residual);
    printf("STUFF ENDS:\n");
    factor_rotate(fa, m, 2); 
    factor_scores(fa, data, m, 0);
    return fa;
}

void factor_destructor(FactorAnalysis **myFacta) {
    matrix_destructor(&(*myFacta)->communalities);
    matrix_destructor(&(*myFacta)->psi);
    matrix_destructor(&(*myFacta)->specificVariances);
    matrix_destructor(&(*myFacta)->rawLoadings);
    matrix_destructor(&(*myFacta)->residuals);
    matrix_destructor(&(*myFacta)->rotatedLoadings);
    matrix_destructor(&(*myFacta)->factorScores);
    free(*myFacta);
}


struct CanonicalCorrelation {
    int length;
    SinglyLinkedList *U;
    SinglyLinkedList *V;
    
};
typedef struct CanonicalCorrelation CanonicalCorrelation;

// x1 and x2 must have a minumum of 2 columns each
CanonicalCorrelation* cca_analysis(Matrix *data, Matrix *x1, Matrix *x2) {

    int p = 2;
    double a[][2] = {{1, .4}, {.4, 1}};
    double b[][2] = {{.5, .6}, {.3, .4}};
    double c[][2] = {{.5, .3}, {.6, .4}};
    double d[][2] = {{1 ,.2}, {.2, 1}};
    Matrix *s11, *s22, *s12, *s21;
    matrix_initializeFromArray(&s11, 2, 2, a);
    matrix_initializeFromArray(&s12, 2, 2, b);
    matrix_initializeFromArray(&s21, 2, 2, c);
    matrix_initializeFromArray(&s22, 2, 2, d);
    Eigen *e = eigen_Solve(s11);
    Matrix *eVal;
    matrix_initialize(&eVal, p, p, 0);
    Matrix *eVect;
    matrix_initialize(&eVect, p, p, 0);
    printf("Length: %d\n", eigen_getLength(e));
    for(int i = 0; i < p; i++) {
        matrix_set(eVal, i, i, 1/pow(eigen_getValue(e, i), 0.5));
        Matrix *temp = eigen_getVector(e, i);
        matrix_print(temp);
        matrix_setCol(eVect, temp, i);
        matrix_destructor(&temp);
    }
    
    printf("REAL INVERSE:\n");
    Matrix *t0 = matrix_transpose(eVect, 0);
    Matrix *t1 = matrix_multiplyMatrices(eVal, t0);
    Matrix *t2 = matrix_multiplyMatrices(eVect, t1);
    matrix_print(t2);

    Matrix *t3 = matrix_multiplyMatrices(t2, s12);
    printf("temp\n");
    matrix_print(t3);
    Matrix *t4 = LAalgorithms_findInverse(s22);
    printf("temp\n");
    matrix_print(t4);
    Matrix *t5 = matrix_multiplyMatrices(t3, t4);
    printf("temp\n");
    matrix_print(t5);
    Matrix *t6 = matrix_multiplyMatrices(t5, s21);
    printf("temp\n");
    matrix_print(t6);
    Matrix *t7 = matrix_multiplyMatrices(t6, t2);
    printf("final:\n");
    matrix_print(t7);
    

    Eigen *E = eigen_Solve(t7);

    Matrix *t8 = eigen_getVector(E, 0);
    printf("temp\n");
    matrix_print(t8);
    Matrix *t9 = matrix_multiplyMatrices(t2, t8);
    printf("temp\n");
    matrix_print(t9);
    Matrix *t10 = matrix_multiplyMatrices(s21, t9);
    printf("temp\n");
    matrix_print(t10);
    Matrix *b1 = matrix_multiplyMatrices(t4, t10);
    printf("final:\n");
    matrix_print(b1);

    Matrix *t11 = matrix_multiplyMatrices(s22, b1);
    matrix_transpose(b1, 1);
    Matrix *var = matrix_multiplyMatrices(b1, t11);
    double val = 1/pow(matrix_get(var, 0, 0), 0.5);

    Matrix *v1 = matrix_multiplyConstant(b1, val, 0);
    matrix_transpose(v1, 1);
    printf("V1:\n");
    matrix_print(v1);

    printf("Canonical Correlation: %f\n", pow(eigen_getValue(E, 0), 0.5));
    matrix_destructor(&t0);
    matrix_destructor(&t1);
    matrix_destructor(&t2);
    matrix_destructor(&t3);
    matrix_destructor(&t4);
    matrix_destructor(&t5);
    matrix_destructor(&t6);
    matrix_destructor(&t7);
    matrix_destructor(&eVect);
    matrix_destructor(&eVal);
    eigen_destructor(&e);
    return NULL;  
}
**/
#endif // MULTIVARIATE_H
