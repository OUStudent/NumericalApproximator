
#include "../RootSolving.h"

// returned error value : -.0123456789

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

ParseReturn* root_bisection(Scanner *original, Parser *myParse, double a, double b) {
    static const double TOLERANCE = 1e-7;
    int maxIter = 300;
    Scanner *temp;
    scanner_initializeFromTokens(&temp, original);
    parser_addVariable(myParse, "x", NULL, a);
    ParseReturn *ret = parser_solve(&temp, myParse);
    if(ret->type != 0) {
        parseReturn_destructor(&ret);
        ret = (ParseReturn*) malloc(sizeof(ParseReturn));
        ret->type = 0;
        ret->error = 8;
        scanner_destructor(&temp);
        return ret;
    }
    if(ret->error > 0) {
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
    
    if(a*b >=0 )  { // requires change of signs along interval
        ret->error = 13;
        scanner_destructor(&temp);       
        return ret;
    }
    double m = 0;
    int iter = 0;
    double t1, t2;
    while(1) {
        parseReturn_destructor(&ret);
        scanner_destructor(&temp);
        scanner_initializeFromTokens(&temp, original);
        m = (a+b) / 2;
        parser_replaceVariable(myParse, "x", m);
        ret = parser_solve(&temp, myParse);
        t1 = parseReturn_getValue(ret);
        scanner_destructor(&temp);
        scanner_initializeFromTokens(&temp, original);
        parser_replaceVariable(myParse, "x", a);
        parseReturn_destructor(&ret);
        ret = parser_solve(&temp, myParse);
        t2 = parseReturn_getValue(ret);
        if(t2*t1<0) b = m;
        else a = m;
        iter++;
        if((b-a) < TOLERANCE) break;
        else if(iter==maxIter) break;
    }
 //   printf("Max Iter Reached: %d \n", iter);
    parser_removeVariable(myParse,"x");
    scanner_destructor(&temp);
    ret->value = m;
    return ret;
}

// Helper functionf or newton's method for calculating derivative
static double deriv(Scanner *original, Parser *myParse, double x) {
    Scanner *temp;
    scanner_initializeFromTokens(&temp, original);
    // assumes variable x is already present in myParse
    double h = 0.0000000001;
    parser_replaceVariable(myParse, "x", x+h);
    ParseReturn *ret = parser_solve(&temp, myParse);
    double fxh = parseReturn_getValue(ret);
    parseReturn_destructor(&ret);
    scanner_destructor(&temp);
    scanner_initializeFromTokens(&temp, original);
    parser_replaceVariable(myParse, "x", x);
    ret = parser_solve(&temp, myParse);
    double fx = parseReturn_getValue(ret); 
    parseReturn_destructor(&ret);
    scanner_destructor(&temp);
    return (fxh-fx)/h;
}

ParseReturn* root_newton(Scanner *original, Scanner *derivative, Parser *parseOriginal, Parser *parserDeriv, double init) {
    static const double TOLERANCE = 1e-9;
    int maxIter = 300;
    double x_n = init;
    Scanner *temp;
    scanner_initializeFromTokens(&temp, original);
    Scanner *tempD;
    ParseReturn *d;
    if(derivative!=NULL) scanner_initializeFromTokens(&tempD, original);
    parser_addVariable(parseOriginal, "x", NULL,x_n);
    ParseReturn *ret = parser_solve(&temp, parseOriginal);
    if(ret->type != 0) {
        parseReturn_destructor(&ret);
        ret = (ParseReturn*) malloc(sizeof(ParseReturn));
        ret->type = 0;
        ret->error = 8;
        scanner_destructor(&temp);
        return ret;
    }
    if(ret->error > 0) {
        scanner_destructor(&temp);
        if(derivative!=NULL) scanner_destructor(&tempD);
        return ret;
    }
    double x_n1;
    if(derivative==NULL) x_n1 = x_n -  parseReturn_getValue(ret) / deriv(original, parseOriginal, x_n);
    else {
        parser_addVariable(parserDeriv, "x", NULL,x_n);
        d = parser_solve(&tempD, parserDeriv);
        x_n1 = x_n -  parseReturn_getValue(ret) / parseReturn_getValue(d);
        parseReturn_destructor(&d);
    }
    
    int iter = 0;
    while(iter < maxIter) {
        parseReturn_destructor(&ret);
        x_n = x_n1;
        scanner_destructor(&temp);
        scanner_initializeFromTokens(&temp, original);
        parser_replaceVariable(parseOriginal, "x", x_n);
        ret = parser_solve(&temp, parseOriginal);
        if(derivative==NULL) {
            x_n1 = x_n - parseReturn_getValue(ret) / deriv(original, parseOriginal, x_n);
        }
        else {
            scanner_destructor(&tempD);
            scanner_initializeFromTokens(&tempD, derivative);
            parser_replaceVariable(parserDeriv, "x", x_n);
            d = parser_solve(&tempD, parserDeriv);
            x_n1 = x_n -  parseReturn_getValue(ret) / parseReturn_getValue(d);
            parseReturn_destructor(&d);
        }

        if(fabs(x_n1-x_n)<TOLERANCE) {
            break;
        }
        iter++;
    }
    scanner_destructor(&temp);
    parser_removeVariable(parseOriginal,"x");
    if(derivative!=NULL) {
        scanner_destructor(&tempD);
        parser_removeVariable(parserDeriv,"x");
    }
    ret->value = x_n1;
    return ret;
}
