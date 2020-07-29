
#include "../Integration.h"

/***
 * 
 * Data Structure Declarations only Below:
 * 
 * ***/ 

struct Value {
    double val;
};
typedef struct Value Value;

struct ParseReturn {
    Matrix *val;
    LUReturn *lu;
    Eigen *e;
    GrahmSchmidt *qr;
    char *varName; // only for -2
    double value;
    char unrec; // unrecognized token
    int type; // -2 for unrec var, -1 for assignment, 0 for number, 1 for matrix, 2 for bool, 3 for LU, 4 for QR, 5 for eigen
    int error;
};

/***
 * 
 * Static Function Definitions only Below:
 * 
 * ***/

static void freeStackValue(void* node) {
    Value *v = (Value*) node;
    free(v);
}

// Modified Compositite Simpsons rule
static ParseReturn* modifiedCompSimp(Scanner *original, Parser *myParse, double a, double b, int n) {
    // assume no errors in equation
    //if(n==0) return -1;
   // if(!(n%2)==0) return -1; // n must be even
    // integral = h/3 * (f(x_0)+4f(x_1) +2f(x_2) ... + 4(x_n-1) + f(x_n))
    double h = (b-a)/(double)n;
    char *t;
    Scanner *temp;
    scanner_initializeFromTokens(&temp, original);
    parser_addVariable(myParse, "x", NULL,a);
    ParseReturn *ret = parser_solve(&temp, myParse);
    if(ret->type != 0) {
        parseReturn_destructor(&ret);
        ret = (ParseReturn*) malloc(sizeof(ParseReturn));
        ret->type = 0;
        ret->error = 8;
        scanner_destructor(&temp);
        return ret;
    }
    if(ret->error>0) {
        scanner_destructor(&temp);
        return ret;
    }
    
    double first = parseReturn_getValue(ret);
    parseReturn_destructor(&ret);
    scanner_destructor(&temp);
    scanner_initializeFromTokens(&temp, original);
    parser_replaceVariable(myParse, "x", b);
    ret = parser_solve(&temp, myParse);
    double last = parseReturn_getValue(ret);
    scanner_destructor(&temp);
    double x = a+h; // increment by value
    double sum = 0; // sum of inside values

    for(int i = 0; i < (n-1); i++) {
        parseReturn_destructor(&ret);
        scanner_initializeFromTokens(&temp, original);
        parser_replaceVariable(myParse, "x", x);
        ret = parser_solve(&temp, myParse);
        double val = parseReturn_getValue(ret);
       
        if(i%2==0) { // i value is even
            sum += 4 * val; // 4*f(x_i)
        }
        else {
             sum += 2 * val; // 2*f(x_i)
        }
        x += h; // update increment
        scanner_destructor(&temp);
    }
    parser_removeVariable(myParse,"x");
    ret->value = (h/3)*(first+sum+last);
    return ret;
}


/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// Performs Modified Adaptive Simpsons Quadrature over a singular Integral 
// Returns the estimated integral value
ParseReturn* integration_modifiedAdaptiveSimpQuadrature(Scanner *myScan, Parser *myParser, double a, double b) {
     double TOLERANCE = 1e-6;
     Stack *myStack;
     stack_initialize(&myStack, NULL, freeStackValue);
     Value *A = (Value*) malloc(sizeof(Value));
     A->val = a;
     Value *B = (Value*) malloc(sizeof(Value));
     B->val = b;
     stack_push(myStack, A);
     stack_push(myStack, B);

     double I = 0;
     Value *aa; 
     Value *bb; 
     ParseReturn* I1;
     double I2; 
     double m;
     const int maxIter = 10000; 
     int iter = 0;
     while(stack_getSize(myStack) > 0) {
        bb = (Value*) stack_pop(myStack);
        aa = (Value*) stack_pop(myStack);
        I1 = modifiedCompSimp(myScan, myParser, aa->val, bb->val, 6);
        if(I1->error>0) {
            free(bb);
            free(aa);
            stack_destructor(&myStack);
            return I1;
        }
        m = (aa->val + bb->val)/2;
        ParseReturn* t2 = modifiedCompSimp(myScan, myParser, aa->val, m, 6);
        ParseReturn* t3 = modifiedCompSimp(myScan, myParser, m, bb->val, 6);
        I2 =  t2->value + t3->value;
        
        if(fabs(I2-I1->value)/15 < (bb->val-aa->val)*TOLERANCE) {
            I += I2;
        }
        else {
            Value *temp = (Value*) malloc(sizeof(Value));
            temp->val = m;
            Value *temp2 = (Value*) malloc(sizeof(Value));
            temp2->val = bb->val;
            Value *temp3 = (Value*) malloc(sizeof(Value));
            temp3->val = aa->val;
            Value *temp4 = (Value*) malloc(sizeof(Value));
            temp4->val = m;
            stack_push(myStack, temp);
            stack_push(myStack, temp2);
            stack_push(myStack, temp3);
            stack_push(myStack, temp4);
        }
        if(aa!=NULL) free(aa);
        if(bb!=NULL) free(bb);
        iter++;
        
        parseReturn_destructor(&t2);
        parseReturn_destructor(&t3);
        if(stack_getSize(myStack) ==0) break;
        if(maxIter < iter) break;
        parseReturn_destructor(&I1);
     }
 //    printf("Total Iter: %d\n", iter);
     stack_destructor(&myStack);
     I1->value = I;
     return I1;
}

// Performs Modified Adaptive Simpsons Quadrature over a singular Integral with an upper infinity bound 
// Returns the estimated integral value
ParseReturn* integration_infinityBounds(Scanner *myScan, Parser *myParse, double a) {
    Scanner *temp;
    char *original = scanner_getInput(myScan);
    scanner_initialize(&temp);
    char *first = "(x^(~2))*(";
    int lengthOrig = strlen(original);
    int lengthFirst = strlen(first);
    int size = (lengthOrig +lengthFirst);
    char *t = (char*) malloc(size*sizeof(char));
    for(int i = 0; i < (lengthOrig+lengthFirst); i++) {
        if(i < lengthFirst) t[i] = first[i];
        else t[i] = '\0';
    }
    int index = 0;
    for(int i = 0; i < lengthOrig ; i++) {
        if(lengthFirst+index+1>=size) t = (char*) realloc(t, (size*=2));
        if(original[i] =='x') {
            if(lengthFirst+index+4 >= size) t = (char*) realloc(t, (size*=2));
            t[lengthFirst+index] = '(';
            index++;
            t[lengthFirst+index] = '1';
            index++;
            t[lengthFirst+index] = '/';
            index++;
            t[lengthFirst+index] = 'x';
            index++;
            t[lengthFirst+index] = ')';
        }
        else t[lengthFirst+index] = original[i];
        index++;
    }
    if(lengthFirst+index+1 >= size) t = (char*) realloc(t, (size+=1));
    t[lengthFirst+index] = ')';
    for(int i = 1+lengthFirst+index; i < size; i++) {
        t[i] = '\0';
    }
    scanner_scan(temp, t);

    ParseReturn *ret;
    ParseReturn *tempPR;
    if(a==0) { // cant solve 1/x for x = 0 so we use 0.0000001
        ret = integration_modifiedAdaptiveSimpQuadrature(myScan, myParse, 0, 1);
        if(ret->error >0) return ret;
        tempPR = integration_modifiedAdaptiveSimpQuadrature(temp, myParse, 0.000001, 1);
        ret->value += tempPR->value;
        parseReturn_destructor(&tempPR);
    }
    else if (a < 0) { // lower bound is negative
        ret = integration_modifiedAdaptiveSimpQuadrature(myScan, myParse, a, 1);
        if(ret->error >0) return ret;
        tempPR = integration_modifiedAdaptiveSimpQuadrature(temp, myParse, 0.000001, 1);
        ret->value += tempPR->value;
        parseReturn_destructor(&tempPR);
    } // lower bound is not 0
    else ret = integration_modifiedAdaptiveSimpQuadrature(temp, myParse, 0.000001, 1/a);
    scanner_destructor(&temp);
    free(t);
    free(original);
    return ret;
}
