
#ifndef ROOTSOLVING_H
#define ROOTSOLVING_H

#include "../Compilation/Compilation.h"

ParseReturn* root_bisection(Scanner *original, Parser *myParse, double a, double b);
ParseReturn* root_newton(Scanner *original, Scanner *derivative, Parser *parseOriginal, Parser *parserDeriv, double init);

#endif // ROOTSOLVING_H
