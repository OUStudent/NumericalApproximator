
#include "../Calculator.h"

Scanner *myScan;
Parser *myParse;

ParseReturn *ret;

struct MethodHolder {
    Matrix *one; // L / U / Value
    Matrix *two; // U / R / Vector
};
typedef struct MethodHolder MethodHolder;

MethodHolder *mRet;

int calculator_getVariableType(char *name) {
    return parser_getVariableType(myParse, name);
}

Matrix* calculator_getVariableMatrix(char *name) {
    return parser_getVariableMatrix(myParse, name);
}

void calculator_removeVariables() {
    parser_destructor(&myParse);
    parser_initialize(&myParse);
}

void calculator_importVariable(Matrix *var, char *name) {
    parser_addVariableMatrix(myParse, name, var);
}

void calculator_initialize() {
    parser_initialize(&myParse);
}

void calculator_scannerDestructor() {
	scanner_destructor(&myScan);
}

char calculator_scan(char *s) {
    scanner_initialize(&myScan);
    int unrec = scanner_scan(myScan, s);
    if(unrec!=1) {
        // error: unrecoginized symbol
        char symbol = (char) unrec;
        return symbol;
    }
    return '\0';
}

char* calculator_unrecVar() {
	return parseReturn_getVarName(ret);
}

char calculator_unrecToken() {
	return parseReturn_getToken(ret);	
}

void calculator_unrecVarDestructor() {
    parseReturn_destructor(&ret);
}

int calculator_assignmentType(char *name) {
	return parser_getAssignmentType(myParse, name);
}

Matrix* calculator_assignmentMatrix(char *name) {
	return parser_getAssignmentMatrix(myParse,name);
}

double calculator_assignmentValue(char *name)  {
	return parser_getAssignmentValue(myParse,name);
}

int calculator_calculate() {
    ret = parser_solve(&myScan, myParse);
    int error = parseReturn_getError(ret);
    if(error>0) { // error occured and scanner destructed
        
        if(error==37) {
            printf("Unrecognized Variable: %s\n", parseReturn_getVarName(ret));
            scanner_destructor(&myScan);
            return error;
        }
        else if(error ==12) {
            printf("Unrecognized Token: %c\n", parseReturn_getToken(ret));
            scanner_destructor(&myScan);
            return error;
            
        }
        else printf("Calculator_Calculate Error: %d\n", error);
        parseReturn_destructor(&ret);
        scanner_destructor(&myScan);
        return error;
    }
    else {
        scanner_destructor(&myScan);
        return 0;
    }
}

int calculator_type() {
	
    return parseReturn_getType(ret);
}

double calculator_double() {
    return parseReturn_getValue(ret);
}

Matrix* methodHolder_getTop() {
    if(mRet==NULL) return NULL;
    return mRet->one;
}

Matrix* methodHolder_getBottom() {
    if(mRet==NULL) return NULL;
    return mRet->two;
}

// Returns copy
// Do not destruct
Matrix* calculator_matrix() {
    return parseReturn_getMatrix(ret, 1);
}

void calculator_method() {
    Matrix *top, *bottom;
    switch(calculator_type()) {
        case 3:
        top = parseReturn_luGetL(ret);
        bottom = parseReturn_luGetU(ret);
        break;
        case 4:
        top = parseReturn_qrGetQ(ret);
        bottom = parseReturn_qrGetR(ret);
        break;
        case 5:
        top = parseReturn_eGetValues(ret);
        bottom = parseReturn_eGetVectors(ret);
        break;
        default:
        return;
        // error
    }
    mRet = (MethodHolder*) malloc(sizeof(MethodHolder));
    mRet->one = top;
    mRet->two = bottom;
}

void methodHolderDestructor() {
    if(mRet ==NULL) return;
    matrix_destructor(&mRet->one);
    matrix_destructor(&mRet->two);
    free(mRet);
}

void calculator_ParseReturnDestructor() {
    parseReturn_destructor(&ret);
}

void calculator_destructor() {
    parser_destructor(&myParse);
}

void myLoop(char *s) {
    char unrec = calculator_scan(s);
    if(unrec != '\0') {
        printf("Unrecognized Symbol: %c\n", unrec);
        
        calculator_scannerDestructor();
        return;
    }
    int error = calculator_calculate();
    if(error > 0) {
        printf("Parser Error: %d\n", error);
       return;
    }
    calculator_method(); // if method, will set; but if not; no need to worry
    int type = calculator_type();
    Matrix *temp; // for type 1 only
    Matrix *top, *bottom; // for type 3-5 only
    int myBool;
    switch(type) {
        case -1:
            printf("Assignment\n");
            break;
        case 0:
            printf("Value: %.10f\n", calculator_double());
            break;
        case 1:
            temp = calculator_matrix();
            matrix_print(temp);
            break;
        case 2:
            myBool = (int) calculator_double();
            if(myBool==-1) printf("ERROR: The request you have made has failed...\n");
            else if(myBool == 0) printf("FALSE\n");
            else if(myBool == 1) printf("TRUE\n");
            else if(myBool == 2) printf("Success\n");
            printf("Bool: %d\n", (int) calculator_double());
            break;
        case 3:
            printf("L:\n");
            top = methodHolder_getTop();
            matrix_print(top);
            printf("U:\n");
            bottom = methodHolder_getBottom();
            matrix_print(bottom);
            methodHolderDestructor();
            break;
        case 4:
            printf("Q:\n");
            top = methodHolder_getTop();
            matrix_print(top);
            printf("R:\n");
            bottom = methodHolder_getBottom();
            matrix_print(bottom);
            methodHolderDestructor();
            break;
        case 5:
            printf("Values:\n");
            top = methodHolder_getTop();
            matrix_print(top);
            printf("Vectors:\n");
            bottom = methodHolder_getBottom();
            matrix_print(bottom);
            methodHolderDestructor();
            break;
        default:
            printf("Switch Case Type Error\n");
            break;
    }
   calculator_ParseReturnDestructor();
}

//#include "ImportExport.h"

void main() {
  //  calculator_initialize();
  //  Matrix *temp;
  //  double array[1][1];
  //  array[0][0] = 34;
  //  matrix_initializeFromArray(&temp, 1, 1, array);
   // matrix_print(temp);
 //   matrix_destructor(&temp);
  ////  import_matrixCSV("C:/Users/BScot/OneDrive/Desktop/Book1.csv");
  //  temp = importReturn_getMatrix(1);
 //   matrix_print(temp);
 //   matrix_destructor(&temp);
  //  importReturn_destructor();
 //   myLoop("a=[[23]]");
 //   myLoop("a");
  //  myLoop("b=[[23]]");
  //  myLoop("b");
  //  myLoop("a*b");
   // myLoop("newton(x-4, NULL, 3.99999999998)");
  //  myLoop("integrate(1/x^2, 2, infty)");
 //   myLoop("x=[[23,23,4],[5,3,4]]");
  //  myLoop("x");
 //   myLoop("x=[[23,23,4],[5,3,4]]");
  //  myLoop("x");
  //  myLoop("x=[[23,23,4],[5,3,4]]");
  //  myLoop("x");
  ///  myLoop("x=[[23,23,4],[5,3,4]]");
  //  myLoop("x");
   // myLoop("x=[[23,23,4],[5,3,4]]");
   // myLoop("x");
   /// myLoop("x=[[23,23,4],[5,3,4]]");
   // myLoop("x");
  //  myLoop("a=[[34]]");
  //  myLoop("a");
 // myLoop("[[23]]");
  //  printf("Type: %d\n", calculator_assignmentType("a"));
    //myLoop("a=[[23,3],[23,3]]");
  //  myLoop("a=[[23,3],[23,3]]");
  //  myLoop("a=[[23,3],[23,3]]");
   // myLoop("b=[[23,3],[23,3]]");
  //  calculator_destructor();
}

