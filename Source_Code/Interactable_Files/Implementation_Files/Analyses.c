
#include "../Analyses.h"

PrincipalComponent *myPC = NULL;
Matrix *eigenValues = NULL;
Matrix *eigenVectors = NULL;
double *kthProp = NULL;
Matrix *zscores = NULL;
Matrix *pcaScores = NULL;
FactorAnalysis *myFacta;

// if flag is 1: use cov
// if flag is 0: use cor
void analyses_pca(Matrix *data, int flag) {
    Matrix *covCor;
    if(flag) covCor = multi_covariance(data,NULL);
    else covCor = multi_correlation(data, NULL);
    myPC = pca_solve(covCor, NULL);
}

Matrix* analyses_pcaEigenValues() {
    eigenValues = pca_getVariance(myPC);
    return eigenValues;
}

Matrix* analyses_pcaEigenVectors() {
    eigenVectors = pca_getComponent(myPC);
    return eigenVectors;
}

double* analyses_pcaKthProp() {
    int size = pca_getLength(myPC);
    kthProp = (double*) malloc(size*sizeof(double));
    for(int i = 0; i <size; i++) {
        kthProp[i] = pca_getKthProp(myPC, i);
    }
    return kthProp;
}

Matrix* analyses_pcaScores(Matrix *data) {
	pcaScores = matrix_multiplyMatrices(data, eigenVectors);
	return pcaScores;
}

void analyses_pcaDestructor() {
    pca_destructor(&myPC);
    if(eigenVectors!=NULL) matrix_destructor(&eigenVectors);
    if(eigenValues!=NULL) matrix_destructor(&eigenValues);
    if(kthProp !=NULL) free(kthProp);
    if(pcaScores !=NULL) matrix_destructor(&pcaScores);
	analyses_zscoresDestructor();
}
//analyses_zscores
Matrix* analyses_zscores(Matrix *data) {
	 // analyses_zscores
	zscores = multi_zscores(data, NULL, NULL);
	return zscores;
}

void analyses_zscoresDestructor() {
	if(zscores != NULL)	matrix_destructor(&zscores);
}

static int M; // number of components used in FA throughout processes

void analyses_factorDestructor() {
    factor_destructor(&myFacta);
}

// if flag is 1 use cor
// if flag is 0 use cov
void analyses_factorAnalysis(Matrix *data, int m, int flag) {
    Matrix* covCor;
    if(flag) covCor = multi_correlation(data, NULL);
    else covCor = multi_covariance(data, NULL);
    PrincipalComponent *myPC2 = pca_solve(covCor, NULL);
    myFacta = factor_analysis(myPC2, data, m, flag);
    M = m;
}

void analyses_factorRotate(int dir, int flag) {
    factor_rotate(myFacta, M, dir, flag);
}

// if flag is 1, assumes data is zscores
// if flag is 0, data is not zscore so center data
void analyses_factorScores(Matrix* data, int flag) {
    factor_scores(myFacta, data, M, flag);
}

Matrix *analyses_factorGetRawLoadings() {
    return myFacta->rawLoadings;
}

double analyses_factorGetAngle() {
	printf("Degree in C: %f\n", myFacta->degree);
    return myFacta->degree;
}

Matrix *analyses_factorGetRotatedLoadings() {
    return myFacta->rotatedLoadings;
}

Matrix *analyses_factorGetCommunalities() {
    return myFacta->communalities;
}

Matrix *analyses_factorGetFactorScores() {
    return myFacta->factorScores;
}

Matrix *analyses_factorGetPsi() {
    return myFacta->psi;
}

Matrix *analyses_factorGetResiduals() {
    return myFacta->residuals;
}

Matrix *analyses_factorGetSpecificVariances() {
    return myFacta->specificVariances;
}
