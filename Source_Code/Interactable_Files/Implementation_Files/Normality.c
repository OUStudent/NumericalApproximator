
#include "../Normality.h"

static Matrix* qqplotHelperUnivariate(int row) {
    int nrow = row;//matrix_nrow(data);
   // if(nrow > 370) return NULL; // error: max nrow reached for bisection
    double p[nrow][2]; 
    for(int i = 1; i <= nrow; i++) {
        p[i-1][0] = (i-0.5)/nrow;
        p[i-1][1] = distributions_qnorm(p[i-1][0], 0, 1);
    }
    Matrix *ret;
    matrix_initializeFromArray(&ret, row, 2, p);
    return ret;
}

static Matrix* qqplotHelperMultivariate(int row, int col) {
    int nrow = row;//matrix_nrow(data);
    int ncol = col; // matrix_ncol(data);
    double p[nrow][2]; // probability
    for(int i = 1; i <= nrow; i++) {
        p[i-1][0] = (i-0.5)/nrow;
        p[i-1][1] = distributions_qchisq(p[i-1][0], ncol);
    }
    Matrix *ret;
    matrix_initializeFromArray(&ret, row, 2, p);
    return ret;
}

struct Value {
    double value;
};
typedef struct Value Value;

void freeValue(void *value) {
    Value *val = (Value*) value;
    free(val);
}

int compare(const void* l, const void* r) {
    Value *left = (Value*) l;
    Value *right = (Value*) r;
    if(left->value < right->value) return -1;
    else if(left->value > right->value) return 1;
    else return 0;
}

Matrix* normality_qqplotMultivariate(Matrix *data, Matrix *cov, Matrix *mean) {
    int covNull = 0;
    int meanNull = 0;
    if(mean==NULL) {
        mean = multi_meanVector(data);
        meanNull = 1;
    }
    if(cov==NULL) {
        cov = multi_covariance(data, mean);
        covNull = 1;
    }
    int nrow = matrix_nrow(data);
    int ncol = matrix_ncol(data);

    Matrix *sqrd = statistics_getSquaredDistances(data, cov, mean);
    DoublyLinkedList *myLL;
    doublyLinkedList_initialize(&myLL, compare, freeValue);
    for(int i = 0; i < nrow; i++) {
        Value *val = (Value*) malloc(sizeof(Value));
        val->value = matrix_get(sqrd, i, 0);
        doublyLinkedList_add(myLL, val);
    }
    doublyLinkedList_mergeSort(myLL, 1);
    int iter = 0;
    while(doublyLinkedList_hasNext(myLL)) {
        Value *val = (Value*) doublyLinkedList_next(myLL);
        matrix_set(sqrd, iter, 0, val->value);
        iter++;
    }
    doublyLinkedList_destructor(&myLL);

    matrix_addCol(sqrd, 1);
    Matrix *prob = qqplotHelperMultivariate(nrow, ncol);
    for(int i = 0; i < nrow; i++) {
        matrix_set(sqrd, i, 1, matrix_get(prob, i, 1));
    }

    matrix_destructor(&prob);
    if(covNull) matrix_destructor(&cov);
    if(meanNull) matrix_destructor(&mean);
    matrix_print(sqrd);
   return sqrd;
}

Matrix* normality_qqplotUnivariate(Matrix *data) {
    int nrow = matrix_nrow(data);
    int ncol = matrix_ncol(data);
    DoublyLinkedList *myLL;
    Matrix *sortedData;
    matrix_initialize(&sortedData, nrow, ncol+1, 0);
    for(int i = 0; i < ncol; i++) {

        doublyLinkedList_initialize(&myLL, compare, freeValue);
        for(int j = 0; j < nrow; j++) {
            Value *value = (Value*) malloc(sizeof(Value));
            value->value = matrix_get(data, j, i);
            doublyLinkedList_add(myLL, value);
        }
        doublyLinkedList_mergeSort(myLL, 1);

        int iter = 0;
        while(doublyLinkedList_hasNext(myLL)) {
            Value *val = (Value *) doublyLinkedList_next(myLL);
            matrix_set(sortedData, iter, i, val->value);
            iter++;
        }

        doublyLinkedList_destructor(&myLL);
    }
    Matrix *prob = qqplotHelperUnivariate(nrow);
    for(int i = 0; i < nrow; i++) {
        matrix_set(sortedData, i, ncol, matrix_get(prob, i, 1));
    }
    matrix_destructor(&prob);
    return sortedData;
}

