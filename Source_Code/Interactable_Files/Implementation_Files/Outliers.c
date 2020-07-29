
#include "../Outliers.h"

Outliers *myOut;
Matrix *sqrdDistances;

void outliers_findOutliers(Matrix *data, Matrix *mean, Matrix *cov, double alpha) {
    int meanNull = 0;
    int covNull = 0;
    if(mean==NULL) {
        meanNull = 1;
        mean = multi_meanVector(data);
    }
    if(cov==NULL) {
        covNull = 1;
        cov = multi_covariance(data, mean);
    }
    myOut = outlier_findOutliers(data, cov, alpha);
    
    sqrdDistances = statistics_getSquaredDistances(data, cov, mean);

    if(meanNull) matrix_destructor(&mean);
    if(covNull) matrix_destructor(&cov);
}

// returns copy
Matrix* outliers_removeOutliers(Matrix *data) {
	return outlier_removeOutliers(&data, myOut, 0);
}

double outliers_getProb() {
    return outlier_getProb(myOut);
}

void outliers_destructor() {
	outlier_destructor(&myOut);
	matrix_destructor(&sqrdDistances);
}

// returns NULL if 
Matrix* outliers_returnOutliers() {
    return outlier_getAllOutlier(myOut);
}

Matrix* outliers_getSqrdDistances() {
    return sqrdDistances;
}

