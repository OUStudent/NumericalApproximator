
#ifndef DISTRIBUTIONS_H
#define DISTRIBUTIONS_H

#include "../NumericalAnalysis/Integration.h"

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/
double distributions_qnorm(double p, double mean, double sd);
double distributions_pnorm(double x, double mean, double sd);
double distributions_gammaValue(double alpha);
double distributions_pchisq(double x, double df);
double distributions_qchisq(double p, double df);

#endif // DISTRIBUTIONS_H
