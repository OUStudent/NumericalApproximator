
#ifndef IMPORTEXPORT_H
#define IMPORTEXPORT_H

#include <stdio.h>
#include "../LinearAlgebra/Matrix.h"
#include <string.h>
#include <ctype.h>
#include "Calculator.h"
int import_addVariable(char *fileName, char *varName);
void importReturn_destructor();
Matrix* importReturn_getMatrix(int flag);
int importReturn_getError();
int import_matrixCSV(char *fileName);

#endif // IMPORTEXPORT_H
