
#ifndef OUTLIERS_H
#define OUTLIERS_H

#include "../Statistics/MultivariateStatistics.h"
void outliers_findOutliers(Matrix *data, Matrix *mean, Matrix *cov, double alpha);
Matrix* outliers_returnOutliers();
Matrix* outliers_getSqrdDistances();
void outliers_destructor();
double outliers_getProb();
Matrix* outliers_removeOutliers(Matrix *data);

#endif // OUTLIERS_H
