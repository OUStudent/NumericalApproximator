
#include "../Distributions.h"

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

double distributions_gammaValue(double alpha) {
    if((int)alpha==alpha) { // if alpha is integer then (alpha-1)! is value
        double fact[(int)alpha-1];
        int index = 0;
        for(int i = alpha-1 ; i >= 0; i--) {
            fact[i] = alpha-index;
            index++;
        }
        double sum = fact[0];
        for(int i = 1; i < alpha-1; i++) {
            sum *= fact[i];
        }
        return sum;
    }
    Scanner *integral;
    scanner_initialize(&integral);
    scanner_scan(integral, "x^(a-1)*e^(~x)");
    Parser *myParse;
    parser_initialize(&myParse);
    parser_addVariable(myParse, "a", NULL,alpha);
    ParseReturn *ret = integration_infinityBounds(integral, myParse, 0);
    scanner_destructor(&integral);
    parser_destructor(&myParse);
    double val = parseReturn_getValue(ret);
    parseReturn_destructor(&ret);
    return val;
}

// like pnorm in R
double distributions_pnorm(double x, double mean, double sd) {
    char *string = "(1/(2*PI)^0.5)*e^(~(x^2)/2)";
    Scanner *myScanner;
    scanner_initialize(&myScanner);
    scanner_scan(myScanner, string);
    Parser *myParse;
    parser_initialize(&myParse);
    double z = (x-mean)/sd;
    ParseReturn *ret = integration_infinityBounds(myScanner, myParse, z);
    double val = parseReturn_getValue(ret);
    parseReturn_destructor(&ret);
    parser_destructor(&myParse);
    scanner_destructor(&myScanner);
    return 1-val;
}

// Uses bisection method
double distributions_qnorm(double p, double mean, double sd) {
    if(p> 1 || p < 0) return -1;
    static const double TOLERANCE = 1e-6;
    int maxIter = 300;

    Scanner *temp;
    scanner_initialize(&temp);
    char *integral = "(1/(2*PI)^0.5)*e^(~(x^2)/2)";
    scanner_scan(temp, integral);
    Parser *myParse;
    parser_initialize(&myParse);
    ParseReturn *p1;
    double a = -2;
    double b = 2;
    double prev = b;
    while(1) {
       // p1 = integration_infinityBounds(temp, myParse, -prev);
      //  a = 1-p-parseReturn_getValue(p1);
       // parseReturn_destructor(&p1);
      //  p1 = integration_infinityBounds(temp, myParse, prev);
      //  b = 1-p-parseReturn_getValue(p1);
      //  parseReturn_destructor(&p1);
      //  a = distributions_pnorm(-prev, mean, sd)-p;
        b = distributions_pnorm(prev, mean, sd)-p;
        if(a*b >= 0) {
            b = prev*2;
          //  a = -prev - 0.5;
            prev = b;
        }
        else {
         //   a = -prev;
            b = prev;
            break;
        }
    }
  //  printf("a: %f\n", a);
  //  printf("b: %f\n", b);
    double m = 0;
    int iter = 0;
    double t1, t2;
    while(1) {
        m = (a+b) / 2;
        p1 = integration_infinityBounds(temp, myParse, m);
        t1 = 1-p-parseReturn_getValue(p1);
       // t1 = distributions_pnorm(m, mean, sd)-p;
        parseReturn_destructor(&p1);
        //t2 = distributions_pnorm(a, mean, sd)-p;
        p1 = integration_infinityBounds(temp, myParse, a);
        t2 = 1-p-parseReturn_getValue(p1);
       
         //   p1 = integration_infinityBounds(temp, myParse, a);
         //   t2 = 1-p-parseReturn_getValue(p1);
        
        parseReturn_destructor(&p1);
        if(t1==0) break;
        else if(t2*t1 < 0) b=m;
        else a=m;
        iter++;
        if((b-a) < TOLERANCE) break;
        else if(iter==maxIter) break;
    }
   // printf("Max Iter Reached: %d \n", iter);
    scanner_destructor(&temp);
    parser_destructor(&myParse);
    return (m*sd)+mean;
}



// like pchisq() in R
double distributions_pchisq(double x, double df) {
    if(x<0) x = -x;
    char string[100];
    double a = df /2;
    double gamma = distributions_gammaValue(a);
    Scanner *myScanner;
    scanner_initialize(&myScanner);
    sprintf(string, "x^(%f)*e^(~x/2)/(2^%f*%f)", a-1, a, gamma);
    
    scanner_scan(myScanner, string);
    Parser *myParse;
    parser_initialize(&myParse);
    ParseReturn *sum = integration_modifiedAdaptiveSimpQuadrature(myScanner, myParse, 0, x);
    parser_destructor(&myParse);
    scanner_destructor(&myScanner);
    double val = parseReturn_getValue(sum);
    parseReturn_destructor(&sum);
    return val;
}

// Uses bisection method
double distributions_qchisq(double p, double df) {
    if(p> 1 || p < 0) return -1;
    double alpha = (df /2)-1;
    double c1 = 1/distributions_gammaValue(alpha+1);
    static const double TOLERANCE = 1e-6;
    int maxIter = 300;
    
    Scanner *temp;
    scanner_initialize(&temp);
    char integral[30];
    sprintf(integral, "x^%f*e^(~x)", alpha);
    scanner_scan(temp, integral);
    Parser *myParse;
    parser_initialize(&myParse);
    ParseReturn *p1;
    
    p1 = integration_modifiedAdaptiveSimpQuadrature(temp, myParse, -0.00000001,0);
    double a = -p+c1*parseReturn_getValue(p1);
    parseReturn_destructor(&p1);
    p1 = integration_modifiedAdaptiveSimpQuadrature(temp, myParse, 0, 100);
    double b = -p+c1*parseReturn_getValue(p1);
    parseReturn_destructor(&p1);
    if(a*b >= 0) a = 0.0000000001;
    a = 0.00000001;
    b = 100;
    double m = 0;
    int iter = 0;
    double t1, t2;
    while(1) {
        
        m = (a+b) / 2;
        p1 = integration_modifiedAdaptiveSimpQuadrature(temp, myParse, 0, m);
        t1 = -p+c1*parseReturn_getValue(p1);
        parseReturn_destructor(&p1);
        if(a < 0) {
            p1 = integration_modifiedAdaptiveSimpQuadrature(temp, myParse, a, 0);
            t2 = -p+c1*parseReturn_getValue(p1);
        }
        else {
            p1 = integration_modifiedAdaptiveSimpQuadrature(temp, myParse, 0, a);
            t2 = -p+c1*parseReturn_getValue(p1);
        }
        parseReturn_destructor(&p1);
        if(t1==0) break;
        else if(t2*t1 < 0) b=m;
        else a=m;
        iter++;
        if((b-a) < TOLERANCE) break;
        else if(iter==maxIter) break;
    }
 //   printf("Max Iter Reached: %d \n", iter);
    scanner_destructor(&temp);
    parser_destructor(&myParse);
    return 2 * m;// only 2*m for this version of bisection
  
}

