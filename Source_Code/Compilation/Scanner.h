/****
 * Author: Brandon Morgan
 * Last Updated: 5/18/2020
 * Description:
 *      Scanner/Lexer for breaking down mathematical equations into tokens to be used by the parser.
 * 
 * Example Use in external File:
 *      // These next Two lines must always be done before using
 *          Scanner *myScan;
 *          scanner_initialize(&myScan);
 *      
 *      // Example function calls
 *          int unrecognized; // return value for my scan
 *          // set unrecognized to return value
 *          // if unrecognized == 1, no error
 *          if( (unrecognized= scanner_scan(myScan, "34*cos(9)"))==1) scanner_print(myScan); 
 *          else if(unrecognized==0) printf("Syntactic error\n"); // syntactic error
 *          else printf("Unrecognized Char %c\n", (char) unrecognized); // unrecognized holds asci value int of unrecognized char returned from scan
 *          scanner_destructor(&myScan);
 * *****/

#ifndef SCANNER_H
#define SCANNER_H

/***
 * 
 * Data Structure Definitions only Below:
 * 
 * ***/ 

typedef struct Scanner Scanner;

#include "../DataStructures/SinglyLinkedList.h"
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "../LinearAlgebra/LinearAlgebra.h"
#include "../NumericalAnalysis/NumericalAnalysis.h"
#include "../Statistics/Statistics.h"

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// Scanner

void scanner_destructor(Scanner **myScan);
int scanner_getSize(Scanner *myScan);
char* scanner_getInput(Scanner *myScan);
void* scanner_getAtIndex(Scanner *myScan, int index); // returns void* entry at given index in linked list
void scanner_initialize(Scanner **myScan);
void scanner_initializeFromTokens(Scanner **newScan, Scanner *oldScan);
void scanner_print(Scanner *myScan);
int scanner_scan(Scanner *myScan, char *input);

// Enumerator 

int scanner_hasNext(Scanner *myScan);
void* scanner_next(Scanner *myScan);
void* scanner_peek(Scanner *myScan);
void scanner_resetEnumerator(Scanner *myScan);

#endif // SCANNER_H
