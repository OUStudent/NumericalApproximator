
#include "../ImportExport.h"

struct ImportReturn {
    Matrix *mat;
    int error;    
};
typedef struct ImportReturn ImportReturn;

static ImportReturn *ret;
static Matrix* matrixCSVHelper(char *fileName);
int importReturn_getError() {
    return ret->error;
}

int import_addVariable(char *fileName, char *varName) {
    import_matrixCSV(fileName);
    if(ret->error > 0) {
        int temp = ret->error;
        importReturn_destructor();
        return temp;
    }
    calculator_importVariable(ret->mat, varName);
    //matrix_print(ret->mat);
    free(ret);
    return 0;
}

//Returns matrix; creates new matrix is flag is 1; else return copy
Matrix* importReturn_getMatrix(int flag) {
    if(flag) {
        Matrix *temp;
        matrix_initializeFromMatrix(&temp, ret->mat);
        return temp;
    }
    else {
        return ret->mat;
    }
}

void importReturn_destructor() {
    if(ret->error == 0) matrix_destructor(&ret->mat);
    free(ret);
}

static Matrix* matrixCSVHelper(char *fileName) {
    FILE *file = fopen(fileName, "r");
    if(file==NULL) { // file does not exit
        return NULL;
    }
    char c = getc(file);
    int firstLine = 1;
    int maxRow = 3;
    int maxCol = 3;
    int nrow = 0;
    int ncol = 0;
    int colIndex = 0;
    double **mat = (double**) malloc(maxRow*sizeof(double*));
    for(int i = 0; i < maxRow; i++) {
        mat[i] = (double*) malloc(maxCol*sizeof(double));
    }
    int SIZE = 10;
    char *myString;
    int firstDecimal;
    while(c != EOF) {
        if((isdigit(c) || c=='.') && firstLine) { // No header
            
        }
        else {
            //
        }
            if(isdigit(c) || c=='.') {
                Again:
                firstDecimal= 1;
                if(c=='.') firstDecimal = 0;
                myString = (char*) malloc(sizeof(char) * SIZE);
                for(int i = 0; i < SIZE; i++) {
                    myString[i] = '\0';
                }
                int iter = 0;
                int inFile;
                do {
                    if(iter == SIZE-1) { // resize
                        int prevSize = SIZE;
                        myString = realloc(myString, sizeof(char)*(SIZE*=1.5));
                        for(int i = prevSize; i < SIZE; i++) {
                            myString[i] = '\0';
                        }
                    }
                    myString[iter++] = c; 
                    inFile = getc(file);

                    if(inFile =='\0') { // end of file reached 
                        goto End;
                    }

                    c = inFile;
                    if(c=='.') {
                        if(firstDecimal) firstDecimal=0;
                        else {
                            printf("ERROR: Two decimals found\n");
                        // return scanReturnHelper(myScan, errorScanner, unrecognized);
                        }
                    }
                    
                } while(isdigit(c)||c=='.');
                End:
                if(colIndex >= maxCol) { // should only change on the first line
                    maxCol *= 1.5;
                    for(int i = 0; i < maxRow; i++) {
                        mat[i] = (double*) realloc(mat[i], maxCol*sizeof(double));
                    }
                  //  for(int i = nrow+1; i < maxRow; i++) {
                       // free(mat[i]);
                      //  mat[i] = (double*) malloc(sizeof(double*)*maxCol);
                  //  } 
                }
                if(nrow >= maxRow) {
                    int oldRow = maxRow;
                    maxRow *= 1.5;
                    mat = (double **) realloc(mat, maxRow*sizeof(double**));
                    for(int i = oldRow; i < maxRow; i++) {
                    mat[i] = (double*) malloc(sizeof(double)*maxCol);
                    }
                }
                mat[nrow][colIndex++] = atof(myString);
                free(myString);
                if(c==',') {
                    c = getc(file);
                    goto Again;
                }
                firstLine = 0;
                nrow++;
                ncol = colIndex;
                colIndex = 0;
            }
            c = getc(file);
    }

    Matrix *temp;
    matrix_initializeFromPointer(&temp, mat, nrow, ncol);
    for(int i = 0; i < maxRow; i++) {
        free(mat[i]);
    }
    free(mat);
    fclose(file);
    return temp;
}

// returns 1 on error
int import_matrixCSV(char *fileName) {
    ret = (ImportReturn*) malloc(sizeof(ImportReturn));
    ret->mat = NULL;
    ret->error = 0;
    FILE *file = fopen(fileName, "r");
    if(file==NULL) { // file does not exit
        ret->error = 1; 
        return 1;
    }
    char c = getc(file);
    int firstLine = 1;
    int maxRow = 3;
    int maxCol = 3;
    int nrow = 0;
    int ncol = 0;
    int colIndex = 0;
    double **mat = (double**) malloc(maxRow*sizeof(double*));
    for(int i = 0; i < maxRow; i++) {
        mat[i] = (double*) malloc(maxCol*sizeof(double));
    }
    int SIZE = 10;
    char *myString;
    int firstDecimal;
    while(c != EOF) {
        if((isdigit(c) || c=='.') && firstLine) { // No header
            
        }
        else {
            //
        }
            if(isdigit(c) || c=='.') {
                Again:
                firstDecimal= 1;
                if(c=='.') firstDecimal = 0;
                myString = (char*) malloc(sizeof(char) * SIZE);
                for(int i = 0; i < SIZE; i++) {
                    myString[i] = '\0';
                }
                int iter = 0;
                int inFile;
                do {
                    if(iter == SIZE-1) { // resize
                        int prevSize = SIZE;
                        myString = realloc(myString, sizeof(char)*(SIZE*=1.5));
                        for(int i = prevSize; i < SIZE; i++) {
                            myString[i] = '\0';
                        }
                    }
                    myString[iter++] = c; 
                    inFile = getc(file);

                    if(inFile =='\0') { // end of file reached 
                        goto End;
                    }

                    c = inFile;
                    if(c=='.') {
                        if(firstDecimal) firstDecimal=0;
                        else {
                            printf("ERROR: Two decimals found\n");
                        // return scanReturnHelper(myScan, errorScanner, unrecognized);
                        }
                    }
                    
                } while(isdigit(c)||c=='.');
                End:
                if(colIndex >= maxCol) { // should only change on the first line
                    maxCol *= 1.5;
                    for(int i = 0; i < maxRow; i++) {
                        mat[i] = (double*) realloc(mat[i], maxCol*sizeof(double));
                    }
                  //  for(int i = nrow+1; i < maxRow; i++) {
                       // free(mat[i]);
                      //  mat[i] = (double*) malloc(sizeof(double*)*maxCol);
                  //  } 
                }
                if(nrow >= maxRow) {
                    int oldRow = maxRow;
                    maxRow *= 1.5;
                    mat = (double **) realloc(mat, maxRow*sizeof(double**));
                    for(int i = oldRow; i < maxRow; i++) {
                    mat[i] = (double*) malloc(sizeof(double)*maxCol);
                    }
                }
                mat[nrow][colIndex++] = atof(myString);
                free(myString);
                if(c==',') {
                    c = getc(file);
                    goto Again;
                }
                firstLine = 0;
                nrow++;
                ncol = colIndex;
                colIndex = 0;
            }
            c = getc(file);
    }

    Matrix *temp;
    matrix_initializeFromPointer(&temp, mat, nrow, ncol);
    for(int i = 0; i < maxRow; i++) {
        free(mat[i]);
    }
    free(mat);
    ret->mat = temp;
    fclose(file);
    return 0;
}

