// Implementation file for Matrix.h

#include "../Matrix.h"

/***
 * 
 * Data Structures only Below:
 * 
 * ***/ 

struct Matrix {
    int ncol;
    int nrow;
    double **mat;
};


/****
 * 
 * Function declarations and implementations below:
 * 
 * ***/

// Adds an empty row to the end of the matrix
// Returns new matrix if flag is 0; if flag is 1, assigns resulting matrix to param matrix
Matrix* matrix_addRow(Matrix *m1, int flag) {
    if(flag) {
        m1->nrow++;
        m1->mat = (double**) realloc(m1->mat, sizeof(double**)*m1->nrow);
        for(int i = 0; i < m1->nrow-1; i++) {
            m1->mat[i] = (double *) realloc(m1->mat[i], sizeof(double)*m1->ncol);
        }
        m1->mat[m1->nrow-1] = (double*) malloc(sizeof(double)*m1->ncol);
        for(int i = 0; i < m1->ncol; i++) {
            m1->mat[m1->nrow-1][i] = 0;
        }
        return NULL;
    }
    else {
        Matrix *temp;
        matrix_initialize(&temp, m1->nrow+1, m1->ncol, 0);
        for(int i = 0; i < m1->nrow; i++) {
            for(int j = 0; j < m1->ncol; j++) {
                temp->mat[i][j] = m1->mat[i][j];
            }
        }
        return temp;
    }
}

// Adds an empty col to the end of the matrix
// Returns new matrix if flag is 0; if flag is 1, assigns resulting matrix to param matrix
Matrix* matrix_addCol(Matrix *m1, int flag) {
    if(flag) {
        m1->ncol++;
        m1->mat = (double**) realloc(m1->mat, sizeof(double**)*m1->nrow);
        for(int i = 0; i < m1->nrow; i++) {
            m1->mat[i] = (double *) realloc(m1->mat[i], sizeof(double)*m1->ncol);
        }
        for(int i = 0; i < m1->nrow; i++) {
            m1->mat[i][m1->ncol-1] = 0;
        }
        return NULL;
    }
    else {
        Matrix *temp;
        matrix_initialize(&temp, m1->nrow, m1->ncol+1, 0);
        for(int i = 0; i < m1->nrow; i++) {
            for(int j = 0; j < m1->ncol; j++) {
                temp->mat[i][j] = m1->mat[i][j];
            }
        }
        return temp;
    }
}

// Adds n amount of columns and rows to the end of the matrix
// Returns new matrix if flag is 0; if flag is 1, assigns resulting matrix to param matrix
// Returns NULL if n is less than dimensions
Matrix* matrix_addColRow(Matrix* m1, int n, int flag) {
    if(m1->ncol <  n || m1->nrow < n || n < 0) return NULL;
    if(flag) {
        m1->nrow += n; 
        m1->ncol += n; 
        m1->mat = (double**) realloc(m1->mat, sizeof(double**)*m1->nrow);
        for(int i = 0; i < m1->nrow-n; i++) {
            m1->mat[i] = (double *) realloc(m1->mat[i], sizeof(double)*m1->ncol);
        }
        for(int i = m1->nrow-n; i < m1->nrow; i++) {
            m1->mat[i] = (double*) malloc(sizeof(double)*m1->ncol);
        }
        for(int i = 0; i < m1->nrow-n; i++) {
            for(int j = m1->ncol-n; j < m1->ncol; j++) {
                 m1->mat[i][j] = 0;
            }
        }
        for(int i = m1->nrow-n; i < m1->nrow; i++) {
            for(int j = 0; j < m1->ncol; j++) {
                 m1->mat[i][j] = 0;
            }
        }
    }
    else {
        Matrix *temp;
        matrix_initialize(&temp, m1->nrow+n, m1->ncol+n, 0);
        for(int i = 0; i < m1->nrow; i++) {
            for(int j = 0; j < m1->ncol; j++) {
                temp->mat[i][j] = m1->mat[i][j];
            }
        }
        return temp;
    }
}

// Adds to matrices together and returns their product
// Returns NULL if sizes do not match
Matrix* matrix_addMatrices(Matrix *m1, Matrix*m2) {
    if(m1->ncol != m2->ncol || m1->nrow != m2->nrow) return NULL;
    Matrix *temp;
    matrix_initialize(&temp, m1->nrow, m1->ncol, 0);
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j < m1->ncol; j++) {
            temp->mat[i][j] = m1->mat[i][j] + m2->mat[i][j];
        }
    }
    return temp;
}

// Adds the param constant to all entries of the matrix
// If flag is 1 the new matrix is stored in param matrix; if 0, the new matrix is returned
Matrix* matrix_addConstant(Matrix *m1, double constant, int flag) {
    if(flag) {
        for(int i = 0; i < m1->nrow; i++) {
            for(int j = 0; j < m1->ncol; j++) {
                m1->mat[i][j] += constant;
            }
        }
        return NULL;
    }
    else {
        Matrix *temp;
        matrix_initializeFromMatrix(&temp, m1);
        for(int i = 0; i < temp->nrow; i++) {
            for(int j = 0; j < temp->ncol; j++) {
                temp->mat[i][j] += constant;
            }
        }
        return temp;
    }
}

// Performs a dot product (Sum of the product of the matrices) and returns the value
// No way for checking for errors as return is double
double matrix_dotProduct(Matrix *m1, Matrix *m2) {
    Matrix *product = matrix_multiplyMatrices(m1, m2);
    if(product==NULL) return 0;
    double sum = matrix_sum(product);
    matrix_destructor(&product);
    return sum;
}

// Deletes the specified row from the matrix
// If flag is 1 the new matrix is stored in param matrix and a copy is returned; if 0, the new matrix is returned
// On error return NULL
Matrix* matrix_deleteRow(Matrix  **m1, int row, int flag) {
    if(row >= (*m1)->nrow || row < 0) return NULL;
    if(flag) {
        Matrix* temp;
        matrix_initialize(&temp, (*m1)->nrow-1,  (*m1)->ncol, 0);
        for(int i = 0; i < row; i++) {
            for(int j = 0; j <  (*m1)->ncol; j++) {
                temp->mat[i][j] =  (*m1)->mat[i][j];
            }
        }
        for(int i = row+1; i <  (*m1)->nrow; i++) {
            for(int j = 0; j <  (*m1)->ncol; j++) {
                temp->mat[i-1][j] =  (*m1)->mat[i][j];
            }
        }
        matrix_destructor(& (*m1));
        matrix_initializeFromMatrix(& (*m1), temp);
        matrix_destructor(&temp);
        return temp;
    }
    else {
        Matrix* temp;
        matrix_initialize(&temp, (*m1)->nrow-1, (*m1)->ncol, 0);
        for(int i = 0; i < row; i++) {
            for(int j = 0; j <  (*m1)->ncol; j++) {
                temp->mat[i][j] =  (*m1)->mat[i][j];
            }
        }
        for(int i = row+1; i <  (*m1)->nrow; i++) {
            for(int j = 0; j <  (*m1)->ncol; j++) {
                temp->mat[i-1][j] =  (*m1)->mat[i][j];
            }
        }
        return temp;
    }
}

// Deletes the specified column from the matrix
// If flag is 1 the new matrix is stored in param matrix and a copy is returned; if 0, the new matrix is returned
// On error return NULL
Matrix* matrix_deleteCol(Matrix  **m1, int col, int flag) {
    if(col >= (*m1)->ncol || col < 0) return NULL;
    if(flag) {
        Matrix* temp;
        matrix_initialize(&temp, (*m1)->nrow, (*m1)->ncol-1, 0);
        for(int i = 0; i < (*m1)->nrow; i++) {
            for(int j = 0; j <  col; j++) {
                temp->mat[i][j] =  (*m1)->mat[i][j];
            }
        }
        for(int i = 0; i <  (*m1)->nrow; i++) {
            for(int j = col+1; j <  (*m1)->ncol; j++) {
                temp->mat[i][j-1] =  (*m1)->mat[i][j];
            }
        }
        matrix_destructor(& (*m1));
        matrix_initializeFromMatrix(& (*m1), temp);
        matrix_destructor(&temp);
        return temp;
    }
    else {
        Matrix* temp;
        matrix_initialize(&temp, (*m1)->nrow, (*m1)->ncol-1, 0);
        for(int i = 0; i < (*m1)->nrow; i++) {
            for(int j = 0; j <  col; j++) {
                temp->mat[i][j] =  (*m1)->mat[i][j];
            }
        }
        for(int i = 0; i <  (*m1)->nrow; i++) {
            for(int j = col+1; j <  (*m1)->ncol; j++) {
                temp->mat[i][j-1] =  (*m1)->mat[i][j];
            }
        }
        return temp;
    }
}

// Destructor
void matrix_destructor(Matrix **mat) {
    for(int i = 0; i < (*mat)->nrow; i++) {
        free((*mat)->mat[i]);
    }
    free((*mat)->mat);
    free(*mat);
}

// Returns the specified [row][col] value; 
// no way for checking for index out of bounds as return is double
double matrix_get(Matrix *mat, int row, int col) {
    return mat->mat[row][col];
}

// Returns the column from the matrix ; NULL if col is bigger than m1
Matrix* matrix_getCol(Matrix *m1, int col) {
    if(col > m1->ncol) return NULL;
    Matrix *temp;
    matrix_initialize(&temp, m1->nrow, 1, 0);
    for(int i = 0; i < m1->nrow; i++) {
        temp->mat[i][0] = m1->mat[i][col];
    }
    return temp;
}

// Returns the Row from the matrix ; NULL if col is bigger than m1
Matrix* matrix_getRow(Matrix *m1, int row) {
    if(row > m1->nrow) return NULL;
    Matrix *temp;
    matrix_initialize(&temp, 1, m1->ncol, 0);
    for(int i = 0; i < m1->ncol; i++) {
        temp->mat[0][i] = m1->mat[row][i];
    }
    return temp;
}

// Returns size of matrix if using malloc for Matrix outside of class
unsigned long matrix_getMallocSizeOf() {
    return sizeof(Matrix);
}

// Initializes matrix mat with nrow and ncol with each index to value
void matrix_initialize(Matrix **mat, int nrow, int ncol, double value) {
    (*mat) = (Matrix*) malloc(sizeof(Matrix));
    (*mat)->mat = (double**) malloc(nrow*sizeof(double));
    for(int i = 0; i < nrow; i++) {
        (*mat)->mat[i] = (double*) malloc(ncol*sizeof(double));
        for(int j = 0; j < ncol; j++) { // intialize all values to 0
            (*mat)->mat[i][j] =value;
        }
    }
    (*mat)->ncol = ncol;
    (*mat)->nrow = nrow;
}

void matrix_initializeFromPointer(Matrix **mat, double **ptr, int nrow, int ncol) {
    (*mat) = (Matrix*) malloc(sizeof(Matrix));
    (*mat)->mat = (double**) malloc(nrow*sizeof(double*));
    for(int i = 0; i < nrow; i++) {
        (*mat)->mat[i] = (double*) malloc(ncol*sizeof(double));
        for(int j = 0; j < ncol; j++) { // intialize all values to 0
            (*mat)->mat[i][j] = ptr[i][j];
        }
    }
    (*mat)->ncol = ncol;
    (*mat)->nrow = nrow;
}

// Initializes matrix mat whose values are the passed in 2D array
void matrix_initializeFromArray(Matrix **mat, int nrow, int ncol, double array[][ncol]) {
    (*mat) = (Matrix*) malloc(sizeof(Matrix));
    (*mat)->mat = (double**) malloc(nrow*sizeof(double*));
    for(int i = 0; i < nrow; i++) {
        (*mat)->mat[i] = (double*) malloc(ncol*sizeof(double));
        for(int j = 0; j < ncol; j++) { // intialize all values to 0
            (*mat)->mat[i][j] = array[i][j];
        }
    }
    (*mat)->ncol = ncol;
    (*mat)->nrow = nrow;
}

// Initializes matrix mat with the same dimensions and values as 'old' matrix
void matrix_initializeFromMatrix(Matrix **mat, Matrix *old) {
    int col = old->ncol;
    int row = old->nrow;
    (*mat) = (Matrix*) malloc(sizeof(Matrix));
    (*mat)->mat = (double**) malloc(row*sizeof(double*));
    for(int i = 0; i < row; i++) {
        (*mat)->mat[i] = (double*) malloc(col*sizeof(double));
        for(int j = 0; j < col; j++) { 
            (*mat)->mat[i][j] = old->mat[i][j];
        }
    }
    (*mat)->ncol = col;
    (*mat)->nrow = row;
}

// Inserts the 'col' of m1 to m2
// m2 must be a single column whose row number matches m1
// Returns new matrix if flag is 0; if flag is 1, assigns resulting matrix to param matrix
Matrix* matrix_insertCol(Matrix *m1, Matrix *m2, int col, int flag) {
    if(m2->ncol > 1 || m1->nrow != m2->nrow || col > m1->ncol+1|| col < 0) return NULL;
    if(flag) {
       matrix_addCol(m1, 1);
       for(int i = 0; i < m1->nrow; i++) {
           for(int j = m1->ncol-1; j > col; j--) {
               m1->mat[i][j] = m1->mat[i][j-1];
           }
       }
       for(int i = 0; i < m1->nrow; i++) {
            m1->mat[i][col] = m2->mat[i][0];
        }
       return NULL;
    }
    else {
        Matrix *temp;
        matrix_initialize(&temp, m1->nrow, m1->ncol+1, 0);
        for(int i = 0; i < m1->nrow; i++) {
            for(int j = 0; j < col; j++) {
                temp->mat[i][j] = m1->mat[i][j];           
            }
        }
        for(int i = 0; i < m1->nrow; i++) {
            temp->mat[i][col] = m2->mat[i][0];
        }
        for(int i = 0; i < m1->nrow; i++) {
            for(int j = m1->ncol; j >col; j--) {
                temp->mat[i][j] = m1->mat[i][j-1];
            }
        }
        return temp;
    }
}

// Inserts the 'row' of m1 to m2
// m2 must be a single row whose column number matches m1
// Returns new matrix if flag is 0; if flag is 1, assigns resulting matrix to param matrix
Matrix* matrix_insertRow(Matrix *m1, Matrix *m2, int row, int flag) {
   if(m2->nrow > 1 || m1->ncol != m2->ncol || row > m1->nrow+1|| row < 0) return NULL;
    if(flag) {
       matrix_addRow(m1, 1);
       for(int i = 0; i < m1->ncol; i++) {
           for(int j = m1->nrow-1; j > row; j--) {
               m1->mat[j][i] = m1->mat[j-1][i];
           }
       }
       for(int i = 0; i < m1->ncol; i++) {
            m1->mat[row][i] = m2->mat[0][i];
        }
       return NULL;
    }
    else {
        Matrix *temp;
        matrix_initializeFromMatrix(&temp, m1);
        matrix_addRow(temp, 1);
       for(int i = 0; i < temp->ncol; i++) {
           for(int j = temp->nrow-1; j > row; j--) {
               temp->mat[j][i] = temp->mat[j-1][i];
           }
       }
       for(int i = 0; i < temp->ncol; i++) {
            temp->mat[row][i] = m2->mat[0][i];
        }
        return temp;
    }
}

// Minus' matrices and returns their product
// Returns NULL if dimensions do not match
Matrix* matrix_minusMatrices(Matrix *m1, Matrix*m2) {
    if(m1->ncol != m2->ncol || m1->nrow != m2->nrow) return NULL;
    Matrix *temp;
    matrix_initialize(&temp, m1->nrow, m1->ncol, 0);
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j < m1->ncol; j++) {
            temp->mat[i][j] = m1->mat[i][j] - m2->mat[i][j];
        }
    }
    return temp;
}

// Multiplys matrix by constant and returns new matrix if flag is 0; 
// if flag is 1, assigns resulting matrix to m1
Matrix* matrix_multiplyConstant(Matrix *m1, double c, int flag) {
    Matrix *temp;
    if(!flag) {
        matrix_initializeFromMatrix(&temp,m1);
    }
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j < m1->ncol; j++) {
            if(flag) m1->mat[i][j] *= c;
            else temp->mat[i][j] *= c;
        }
    }
    if(flag) return NULL;
    else return temp;
}

// Multiplies matrices together and returns their product
// Returns NULL if dimensions do not match
Matrix* matrix_multiplyMatrices(Matrix *m1, Matrix *m2) {
    if(m1->ncol != m2->nrow) return NULL;
    Matrix *temp;
    matrix_initialize(&temp, m1->nrow, m2->ncol, 0);
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j < m2->ncol; j++) {
            temp->mat[i][j] = 0;
            for(int k = 0; k < m2->nrow; k++) {
                temp->mat[i][j] += m1->mat[i][k] * m2->mat[k][j];
            }
        }
    }
    return temp;
}

// Returns number of rows
int matrix_nrow(Matrix *mat) {
    return mat->nrow;
}
// Returns number of columns
int matrix_ncol(Matrix *mat) {
    return mat->ncol;
}

// Prints the given matrix to STDOUT
void matrix_print(Matrix *m1) {
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j<m1->ncol;j++) {
            printf("%f ", m1->mat[i][j]);
        }
        printf("\n");
    }
}

// Sets the matrix at indices of valus
// Returns 1 if succesful; 0 otherwise
int matrix_set(Matrix *mat, int row, int col, double value) {
    if(col > mat->ncol || row > mat->nrow) return 0;
    mat->mat[row][col] = value;
    return 1;
}

// Sets the 'col' of m1 to m2 by copying
// m2 must be a single column whose row number matches m1
// Returns 1 if succesful; 0 otherwise
int matrix_setCol(Matrix *m1, Matrix *m2, int col) {
    if(m1->nrow != m2->nrow) return 0;
    if(m1->ncol <= col || col < 0) return 0;
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j < m1->ncol; j++) {
            if(j == col) m1->mat[i][j] = m2->mat[i][0];
        }
    }
    return 1;
}

// Calculates the total sum of all the values in the matrix
double matrix_sum(Matrix *m1) {
    double sum = 0;
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j < m1->ncol; j++) {
            sum += m1->mat[i][j];
        }
    }
    return sum;
}

// Calculates the total sum of the column at the given index
double matrix_sumCol(Matrix *m1, int index) {
    double sum = 0;
    if(index < 0 || index > m1->ncol-1) return 0;
    for(int i = 0; i < m1->nrow; i++) {
        sum += m1->mat[i][index];
    }
    return sum;
}

// Calculates the total sum of the row at the given index
double matrix_sumRow(Matrix *m1, int index) {
    double sum = 0;
    if(index < 0 || index > m1->nrow-1) return 0;
    for(int i = 0; i < m1->ncol; i++) {
        sum += m1->mat[index][i];
    }
    return sum;
}

// Sets the 'row' of m1 to m2 by copying
// m2 must be a single row whose column number matches m1
// Returns 1 if succesful; 0 otherwise
int matrix_setRow(Matrix *m1, Matrix *m2, int row) {
    if(m1->ncol != m2->ncol) return 0;
    if(m1->nrow <= row || row < 0) return 0;
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j < m1->ncol; j++) {
            if(i == row) m1->mat[i][j] = m2->mat[0][j];
        }
    }
    return 1;
}

// Returns 1 if matrix is symmetric; otherwise 0
int matrix_isSymmetric(Matrix *m1) {
    double TOLERANCE = 1e-4;
    if(!matrix_isSquare(m1)) return 0;
    Matrix *temp = matrix_transpose(m1, 0);
    for(int i = 0; i < m1->nrow; i++) {
        for(int j = 0; j < m1->ncol; j++) {
            if(temp->mat[i][j] != m1->mat[i][j] && fabs(temp->mat[i][j] - m1->mat[i][j])>TOLERANCE)  {
                matrix_destructor(&temp);
                return 0;
            }
        }
    }
    matrix_destructor(&temp);
    return 1;
}

// Returns 1 if matrix is square; otherwise 0
int matrix_isSquare(Matrix *mat) {
    if(mat->nrow!=mat->ncol) return 0;
    return 1;
}

// Transposes Matrix mat
// If flag is 1 the new matrix is stored in param matrix; if 0, the new matrix is returned
Matrix* matrix_transpose(Matrix *mat, int flag) {
    Matrix *temp;
    matrix_initialize(&temp, mat->ncol, mat->nrow, 0);
    for(int i = 0; i < mat->nrow; i++) {
        for(int j = 0; j < mat->ncol; j++) {
            temp->mat[j][i] = mat->mat[i][j];
        }
    }
    if(flag) {
         for(int i = 0; i < mat->nrow; i++) {
            free(mat->mat[i]);
        }
        free(mat->mat);
        mat->nrow = temp->nrow;
        mat->ncol = temp->ncol;
        mat->mat = temp->mat;
        free(temp);
        return NULL;
    }
    else {
        return temp;
    }
}

// Creates a unit vector from the matrix a;
// Returns NULL if a has more than 1 column
// If flag is 1; new vector is stored in a; if 0, the new matrix is returned
Matrix* matrix_unitVector(Matrix* a, int flag) {
    if(a->ncol > 1) return NULL;
    if(flag) {
        double total = 0;
        for(int i = 0; i < a->nrow; i++) {
            total += pow(a->mat[i][0], 2);
        }
        total = pow(total, 0.5);
        for(int i = 0; i < a->nrow; i++) {
            a->mat[i][0] /= total; 
        }
        return NULL;
    }
    else {
        Matrix *temp;
        matrix_initializeFromMatrix(&temp, a);
        double total = 0;
        for(int i = 0; i < temp->nrow; i++) {
            total += pow(temp->mat[i][0], 2);
        }
        total = pow(total, 0.5);
        for(int i = 0; i < temp->nrow; i++) {
            temp->mat[i][0] /= total; 
        }
        return temp;
    }
}

