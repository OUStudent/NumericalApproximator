
#ifndef INTEGRATION_H
#define INTEGRATION_H

#include "../Compilation/Parser.h"
#include "../DataStructures/Stack.h"

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// Integration

ParseReturn* integration_modifiedAdaptiveSimpQuadrature(Scanner *myScan, Parser *myParser, double a, double b);
ParseReturn* integration_infinityBounds(Scanner *myScan, Parser *myParse, double a);

#endif // INTEGRATION_H
