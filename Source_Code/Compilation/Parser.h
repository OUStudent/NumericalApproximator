/****
 * Author: Brandon Morgan
 * Last Updated: 5/18/2020
 * Description:
 *      Parser that uses a Recursive Descent Parsing algorithm on a given set of tokens. Also calculates
 *      the given values. Parser data structure is capable of holding variables.
 * 
 * Example Use in external File:
 *      // These next Two lines must always be done before using
 *          Parser *myParse;
 *          parser_initialize(&myParse);
 *      
 *      // Example function calls
 *          int unrecognized; // return value for my scan
 *          // set unrecognized to return value
 *          // if unrecognized == 1, no error
 *          if( (unrecognized= scanner_scan(myScan, "34*cos(9)"))==1) {
 *              Parser *myParse;
 *              parser_initialize(&myParse);   
 *              if(parser_parse(&myScan, myParse)) { // returns 0 if a semantic error occured
 *                  double value = parser_solve(&myScan, myParse);
 *                  // Note: if you want to reuse your scanner again, must call destructor and reinitialize
 *                  // scanner_destructor(&myScan);
 *                  // scanner_initialize(&myScan);
 *               }
 *               else // error in equation semantics
 *              
 *          } 
 *          else if(unrecognized==0) printf("Syntactic error\n"); // syntactic error
 *          else printf("Unrecognized Char %c\n", (char) unrecognized); // unrecognized holds asci value int of unrecognized char returned from scan
 *          
 *          scanner_destructor(&myScan);
 * *****/

#ifndef PARSER_H
#define PARSER_H

/***
 * 
 * Data Structure Definitions only Below:
 * 
 * ***/ 

typedef struct Parser Parser;
typedef struct ParseReturn ParseReturn;

#include "Scanner.h"
#include "../DataStructures/AVLTree.h"

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

int parser_getVariableType(Parser *myParse, char *name);

Matrix* parser_getVariableMatrix(Parser *myParse, char *name);

void parser_addVariableMatrix(Parser *myParse, char *name, Matrix *mat);
Matrix* parser_getAssignmentMatrix(Parser *myParse, char *name);
 double parser_getAssignmentValue(Parser *myParse, char *name);
int parser_getAssignmentType(Parser *myParse, char *name);
void parser_destructor(Parser **myParse);
void parser_initialize(Parser **myParse);
char* parser_checkVariables(Scanner **myScan, Parser *myParser);
int parser_parse(Scanner **myScan, Parser *myParser);
ParseReturn* parser_solve(Scanner **myScan, Parser *myParser);
int parser_addVariable(Parser *myParse, char *name, Matrix *val, double value);
void parseReturn_destructor(ParseReturn** ret);
double parseReturn_getValue(ParseReturn *ret);
int parseReturn_getError(ParseReturn *ret);
char* parseReturn_getVarName(ParseReturn *ret);
char parseReturn_getToken(ParseReturn *ret);
Matrix* parseReturn_luGetL(ParseReturn *ret);
Matrix* parseReturn_luGetU(ParseReturn *ret);
Matrix* parseReturn_qrGetQ(ParseReturn *ret);
Matrix* parseReturn_qrGetR(ParseReturn *ret);
Matrix* parseReturn_eGetValues(ParseReturn *ret);
Matrix* parseReturn_eGetVectors(ParseReturn *ret);
Matrix* parseReturn_getMatrix(ParseReturn *ret, int flag);
int parseReturn_getType(ParseReturn *ret);
int parser_removeVariable(Parser *myParse, char *name);
int parser_replaceVariable(Parser *myParse, char *name, double value);

#endif // PARSER_H
