
#include "../MultivariateStatistics.h"

// given a matrix of data, obtain the mean vector and return
// Returns NULL if an error occured
Matrix* multi_meanVector(Matrix *data) {
    int ncol = matrix_ncol(data);
    int nrow = matrix_nrow(data);
    Matrix *mean;
    matrix_initialize(&mean, ncol , 1, 0);
    double sum;
    for(int k = 0; k < ncol; k++) {
        double sum = matrix_sumCol(data, k);
        matrix_set(mean, k, 0, sum/(double)(nrow));
    }
    return mean;
}

// Standardizes the data by centering and scaling the data;
// If meanVector is not known, pass in null; 
Matrix* multi_standardize(Matrix *data, Matrix *mean) {
    int meanWasNull = 0;
    if(mean ==NULL) {
        meanWasNull = 1;
        mean = multi_meanVector(data);
    }
    int p = matrix_ncol(data);
    int n = matrix_nrow(data);
    Matrix *temp;
    matrix_initialize(&temp, n, p, 0);
    double val;
    for(int i = 0; i < n;i++) {
        for(int j = 0; j < p; j++) {
            val = (matrix_get(data, i, j) - matrix_get(mean, j, 0)) / pow(pow((matrix_get(data, j, j) - matrix_get(mean, j, 0)), 2)/ (double)(n-1), 0.5);
            matrix_set(temp, i, j, val);
        }
    }

    if(meanWasNull) matrix_destructor(&mean);
    return temp;
}

// Calculates the covariance matrix of the data; if meanvector is known pass in; if not, pass in NULL
Matrix* multi_covariance(Matrix *data, Matrix *mean) {
    int meanWasNull = 0;
    if(mean ==NULL) {
        meanWasNull = 1;
        mean = multi_meanVector(data);
    }
    int p = matrix_ncol(data);
    int n = matrix_nrow(data);
    Matrix *temp;
    matrix_initialize(&temp, p, p, 0);
    double sum = 0;
    for(int i = 0; i < p; i++) {
        for(int k = 0; k < p; k++) {
            sum = 0;
            for(int j = 0; j < n; j++) {
                sum += (matrix_get(data, j, i) - matrix_get(mean, i, 0))*(matrix_get(data, j, k)-matrix_get(mean, k, 0));
            }   
            sum = sum / (double) (n-1);
            matrix_set(temp, i, k, sum);
        }
    }
    if(meanWasNull) matrix_destructor(&mean);
    return temp;
}

// Calculates the correlation matrix of data from its covariance; if covariance is not known, pass in NULL;
Matrix* multi_correlation(Matrix *data, Matrix *cov) {
    int p = matrix_ncol(data);
    int n = matrix_nrow(data);
    int covWasNULL = 0;
    if(cov==NULL) {
        covWasNULL = 1;
        cov = multi_covariance(data, NULL);
    }
    double value = 0;
    Matrix *cor;
    matrix_initialize(&cor, p, p, 0);

    for(int i = 0; i < p; i++) {
        for(int k = 0; k < p; k++) {
            value = matrix_get(cov, i, k) / (pow(matrix_get(cov, i, i), 0.5)*pow(matrix_get(cov, k, k), 0.5));
            matrix_set(cor, i, k, value);
            value = 0;
        }
    }
    if(covWasNULL)matrix_destructor(&cov);
    return cor;
}

// Calculates the zscores of the data by centering and standardizing it
// if either or both meanVector and covariance are not known pass in NULL
Matrix* multi_zscores(Matrix *data, Matrix *meanVector, Matrix *cov) {
    int meanWasNull = 0;
    int covWasNull = 0;
    if(meanVector==NULL) {
        meanVector = multi_meanVector(data);
        meanWasNull = 1;
    }
    if(cov==NULL) {
        if(meanWasNull) cov = multi_covariance(data, NULL);
        else cov = multi_covariance(data, meanVector);
        covWasNull = 1;
    }
    Matrix *z;
    int n = matrix_nrow(data);
    int p = matrix_ncol(data);
    matrix_initialize(&z, n, p, 0);
    double value = 0;
    for(int j = 0; j < n; j++) {
        for(int k = 0; k < p; k++) {
            value = (matrix_get(data, j, k) - matrix_get(meanVector, k, 0) )/ pow(matrix_get(cov, k, k), 0.5);
            matrix_set(z, j, k, value);
            value = 0;
        }
    }
    if(meanWasNull) matrix_destructor(&meanVector);
    if(covWasNull) matrix_destructor(&cov);
    return z;
}


struct PrincipalComponent {
    Eigen *pc;
    Matrix *kthProportion;
};
typedef struct PrincipalComponent PrincipalComponent;

PrincipalComponent* pca_solve(Matrix *covCor, Eigen *e) {
    PrincipalComponent *myPC = (PrincipalComponent*) malloc(sizeof(PrincipalComponent));
    if(e==NULL) e = eigen_Solve(covCor);
    else e = eigen_copy(e);
    myPC->pc = e;
    matrix_initialize(&myPC->kthProportion, 1, eigen_getLength(e), 0); 
    double sum = 0;
    Matrix *eValues = eigen_getValue(e);
    for(int i = 0; i < eigen_getLength(e); i++) {
        sum+= matrix_get(eValues, 0, i);
    }
    for(int i = 0; i < eigen_getLength(e); i++) {
        matrix_set(myPC->kthProportion, 0, i, (matrix_get(eValues, 0, i) / sum) * 100); // percentage
    }
    matrix_destructor(&eValues);
    return myPC;
}


// returns the variance (eigen value) matrix 
// will need to be freed
Matrix* pca_getVariance(PrincipalComponent *myPC) {
    return eigen_getValue(myPC->pc);
}

// returns the components, i.e. the eigenvectors
// will need to be freed
Matrix* pca_getComponent(PrincipalComponent *myPC) {
    return eigen_getVector(myPC->pc);
}

int pca_getLength(PrincipalComponent *myPC) {
   int x = eigen_getLength(myPC->pc);
   return x;
}

double pca_getKthProp(PrincipalComponent *myPC, int index) {
    return matrix_get(myPC->kthProportion, 0, index);
}

void pca_destructor(PrincipalComponent **myPC) {
    eigen_destructor(&((*myPC)->pc));
    matrix_destructor(&(*myPC)->kthProportion);
    free(*myPC);
}

struct Outliers {
    int length;
    double prob; // the necessary value to exceed
    Matrix *outliers; // first column is data row index of outler and second column is its squard distances
};
typedef struct Outliers Outliers;

Matrix* statistics_getSquaredDistances(Matrix *data, Matrix *cov, Matrix *mean) {
    int n1 = matrix_nrow(data);
    int n2 = matrix_ncol(data);
    mean = matrix_transpose(mean, 0);
    Matrix *covInverse = LAalgorithms_findInverse(cov);
    int index = 0;
    double out[n1][1];
    for(int i = 0; i < n1; i++) {
        Matrix *row = matrix_getRow(data, i);
        Matrix *result = matrix_minusMatrices(row, mean);
        Matrix *t1 = matrix_multiplyMatrices(result, covInverse);
        matrix_transpose(result, 1);
        Matrix *t2 = matrix_multiplyMatrices(t1, result);

        out[i][0] = matrix_get(t2, 0, 0);
    
        matrix_destructor(&result);
        matrix_destructor(&row);
        matrix_destructor(&t1);
        matrix_destructor(&t2);
    }
    matrix_destructor(&covInverse);
    matrix_destructor(&mean);
    Matrix *sqrd;
    matrix_initializeFromArray(&sqrd, n1, 1, out);
    return sqrd;
}

// Finds the outliers of the dataset given the confidence level alpha
// If zscores are not known pass in NULL
// Alpha must be between 0.0 and 1.0
// Returns NULL on error
Outliers* outlier_findOutliers(Matrix *data, Matrix *cov, double alpha) {
    Matrix *mean = multi_meanVector(data);
    int covWasNull = 0;
    if(cov==NULL) {
        covWasNull = 1;
        cov = multi_covariance(data, mean);
    }
    int n1 = matrix_nrow(data);
    int n2 = matrix_ncol(data);
    Outliers *myOut = (Outliers*) malloc(sizeof(Outliers));
    double p = distributions_qchisq(alpha, n2);
    matrix_transpose(mean, 1);
    Matrix *covInverse = LAalgorithms_findInverse(cov);
    int index = 0;
    double out[n1][2];
    for(int i = 0; i < n1; i++) {
        Matrix *row = matrix_getRow(data, i);
        Matrix *result = matrix_minusMatrices(row, mean);
        Matrix *t1 = matrix_multiplyMatrices(result, covInverse);
        matrix_transpose(result, 1);
        Matrix *t2 = matrix_multiplyMatrices(t1, result);

        double value = matrix_get(t2, 0, 0);
        if(value >= p) {
            // add outlier
            out[index][0] = i; // 0 based
            out[index][1] = value;
            index++;
        }
    
        matrix_destructor(&result);
        matrix_destructor(&row);
        matrix_destructor(&t1);
        matrix_destructor(&t2);
    }
   
    Matrix *myOutliers;
    if(index==0) myOutliers=NULL;
    else matrix_initializeFromArray(&myOutliers, index, 2, out);
  //  printf("Outliers:\n");
   // matrix_print(myOutliers);
  //  printf("Outlier Data Rows:\n");
    myOut->prob = p;
    for(int i = 0; i < index; i++) {
        Matrix *row = matrix_getRow(data, out[i][0]);
        matrix_print(row);
        matrix_destructor(&row);
    }
    myOut->length = index;
    myOut->outliers = myOutliers;
    matrix_destructor(&mean);
    matrix_destructor(&covInverse);
    if(covWasNull) matrix_destructor(&cov);
    return myOut;
}

double outlier_getProb(Outliers *myOut) {
    return myOut->prob;
}

// Returns the number of outliers
int outlier_getLength(Outliers *myOut) {
    return myOut->length;
}

Matrix* outlier_getAllOutlier(Outliers *myOut) {
    return myOut->outliers;
}

// Returns the outlier at the given index
// will need to be freed at some point
Matrix* outlier_getOutlier(Outliers *myOut, int index) {
    Matrix *row = matrix_getRow(myOut->outliers, index);
    return row;
}

// Removes the outlier indices from the data matrix
// if flag is 1, new matrix is stored in data; if 0, new matrix is returned
Matrix* outlier_removeOutliers(Matrix **data, Outliers *outliers, int flag) {
    Matrix *temp;
    if(!flag) matrix_initializeFromMatrix(&temp, *data);
    for(int i = outliers->length-1; i >= 0 ; i--) {
        if(!flag) matrix_deleteRow(&temp, matrix_get(outliers->outliers, i, 0), 1);
        else matrix_deleteRow(data, matrix_get(outliers->outliers, i, 0),1);
    }
    if(!flag) return temp;
    else return NULL;
}

void outlier_destructor(Outliers **myOut) {
    if((*myOut)->length==0) {
        free(*myOut);
        return;
    }
    matrix_destructor(&(*myOut)->outliers);
    free(*myOut);
}





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
// if flag is 1 use corr
// if flag is 0 use cov
void factor_rotate(FactorAnalysis* myFacta, int m, int dir, int flag) {
    Matrix *l; 
    if(m > 3) {
    	matrix_initializeFromMatrix(&l, myFacta->rawLoadings);
    	myFacta->rotatedLoadings = l;
    	return;
	}
    l = myFacta->rawLoadings; // DO NOT CALL DESTRUCTOR
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
   //     printf("Val: %f\n", value);
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
    
    Matrix *covCor;
    if(flag) covCor = multi_correlation(myFacta->data, NULL);
    else covCor = multi_covariance(myFacta->data, NULL);
    int p = matrix_ncol(myFacta->data);
    Matrix *psi;
    matrix_initialize(&psi, p, p, 0);
    double sum;
    // constructing PSI
    for(int i = 0; i < p; i++) {
        sum = 0;
        // sum of the loadings squared
        for(int j = 0; j < m; j++) {
            sum += pow(matrix_get(rotatedloadings, i, j), 2);
        }
        matrix_set(psi, i, i, matrix_get(covCor, i, i)-sum);
    }
    Matrix *specificVar;
    Matrix *h;
    matrix_initialize(&h, p, 1, 0);
    matrix_initialize(&specificVar, p, 1, 0);
    for(int i = 0; i < p; i++) {
        // comunalities
        matrix_set(specificVar, i, 0, matrix_get(psi, i, i));
        matrix_set(h, i, 0, matrix_get(covCor, i, i)-matrix_get(psi, i, i)); // cov/cor[i,i] -h[i,0];
    }
    matrix_destructor(&myFacta->psi);
	matrix_destructor(&myFacta->communalities);
	matrix_destructor(&myFacta->specificVariances);   
	myFacta->psi = psi;
	myFacta->specificVariances = specificVar;
	myFacta->communalities = h;
	
    matrix_destructor(&r);
}
 
 // flag is used to signify whether or not data was standardized before or not
 // if flag is 1, assumes data was transformed into zscores (centered and scaled)
 // if flag is 0, only centers the data
void factor_scores(FactorAnalysis *myFacta, Matrix *data, int m, int flag) {
    Matrix *t1 = matrix_transpose(myFacta->rotatedLoadings, 0);
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
   // matrix_transpose(a, 1); 
    matrix_print(a);
    matrix_destructor(&a);
    matrix_destructor(&A);
    matrix_destructor(&t7);
}

FactorAnalysis* factor_analysis(PrincipalComponent *myPC, Matrix *data, int m, int flag) {
    Matrix *covCor;
    if(flag) covCor = multi_correlation(data, NULL);
    else covCor = multi_covariance(data, NULL);
    
    int n = pca_getLength(myPC);
    Matrix *loadings;
    matrix_initialize(&loadings, n, m, 0);
    double value;
    printf("Components:\n");
    Matrix *pcVar = pca_getVariance(myPC);
    Matrix *pcComp = pca_getComponent(myPC);
    // construct the loadings
    // square root of eigenvalue times eigenvector
    for(int i = 0; i < m; i++) { // do columns first
        value = pow(matrix_get(pcVar, 0, i), 0.5);
        Matrix *temp = matrix_getCol(pcComp, i);
        for(int j = 0; j < n; j++) { // iterate through rows per column
            matrix_set(loadings, j, i, value*matrix_get(temp, j, 0));
        }
        matrix_destructor(&temp);
    }
    matrix_destructor(&pcVar);
    matrix_destructor(&pcComp);
    printf("Loadings\n");
    matrix_print(loadings);
    int p = n;
    Matrix *psi;
    matrix_initialize(&psi, p, p, 0);
    double sum;
    // constructing PSI
    for(int i = 0; i < p; i++) {
        sum = 0;
        // sum of the loadings squared
        for(int j = 0; j < m; j++) {
            sum += pow(matrix_get(loadings, i, j), 2);
        }
        matrix_set(psi, i, i, matrix_get(covCor, i, i)-sum);
    }
    Matrix *specificVar;
    Matrix *h;
    matrix_initialize(&h, n, 1, 0);
    matrix_initialize(&specificVar, n, 1, 0);
    for(int i = 0; i < n; i++) {
        // comunalities
        matrix_set(specificVar, i, 0, matrix_get(psi, i, i));
        matrix_set(h, i, 0, matrix_get(covCor, i, i)-matrix_get(psi, i, i)); // cov/cor[i,i] -h[i,0];
    }
    printf("PSI:\n");
    matrix_print(psi);
    printf("Communalities:\n");
    matrix_print(h);
    printf("Specific Variances:\n");
    matrix_print(specificVar);
    Matrix *Lt = matrix_transpose(loadings, 0);
    Matrix *t = matrix_multiplyMatrices(loadings, Lt);
    Matrix *i = matrix_addMatrices(t, psi);
    Matrix *residual = matrix_minusMatrices(covCor, i);

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
	matrix_initializeFromMatrix(&fa->data, data);
    printf("ANALYSIS:\n\n");
    printf("STUFF ENDS:\n");

  //  factor_rotate(fa, m, 2); 
  //  factor_scores(fa, data, m, 0);
    matrix_destructor(&covCor);
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
    matrix_destructor(&(*myFacta)->data);
    free(*myFacta);
}

/**
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

