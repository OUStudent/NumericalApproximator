
#ifndef ANALYSES_H
#define ANALYSES_H

#include "../LinearAlgebra/LinearAlgebraAlgorithms.h"
#include "../Statistics/MultivariateStatistics.h"

// if flag is 1: use cov
// if flag is 0: use cor
void analyses_pca(Matrix *data, int flag);
Matrix* analyses_pcaEigenValues();
Matrix* analyses_pcaEigenVectors();
double* analyses_pcaKthProp();
void analyses_pcaDestructor();
Matrix* analyses_pcaScores(Matrix *data);

Matrix* analyses_zscores(Matrix *data);
void analyses_zscoresDestructor();

void analyses_factorDestructor();
void analyses_factorAnalysis(Matrix *data, int m, int flag);
void analyses_factorRotate(int dir, int flag);
void analyses_factorScores(Matrix* data, int flag);
Matrix *analyses_factorGetRawLoadings();
double analyses_factorGetAngle();
Matrix *analyses_factorGetRotatedLoadings();
Matrix *analyses_factorGetCommunalities();
Matrix *analyses_factorGetFactorScores();
Matrix *analyses_factorGetPsi();
Matrix *analyses_factorGetResiduals();
Matrix *analyses_factorGetSpecificVariances();

Matrix *analyses_clusterCor(Matrix *data);
void analyses_clusterDestructor();

#endif // ANALYSES_H
