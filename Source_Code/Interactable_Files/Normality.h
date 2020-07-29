
#ifndef NORMALITY_H
#define NORMALITY_H

#include "../LinearAlgebra/Matrix.h"
#include "../Statistics/Distributions.h"
#include "../DataStructures/DoublyLinkedList.h"

Matrix* normality_qqplotMultivariate(Matrix *data, Matrix *cov, Matrix *mean);
Matrix* normality_qqplotUnivariate(Matrix *data);

#endif // NORMALITY_H
