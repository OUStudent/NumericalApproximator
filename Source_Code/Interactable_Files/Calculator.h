
#include "AllHeaderFiles.h"

int calculator_getVariableType(char *name);
Matrix* calculator_getVariableMatrix(char *name);
void calculator_importVariable(Matrix *var, char *name);
void calculator_removeVariables();
Matrix* calculator_assignmentMatrix(char *name);
double calculator_assignmentValue(char *name);
void calculator_initialize();
char calculator_scan(char *s);
int calculator_assignmentType(char *name);
int calculator_calculate();
int calculator_type();
double calculator_double();
void calculator_scannerDestructor();
char calculator_unrecToken();
char* calculator_unrecVar();
void calculator_unrecVarDestructor();
Matrix* methodHolder_getTop();
Matrix* methodHolder_getBottom();
// Returns copy
// Do not destruct
Matrix* calculator_matrix();
void calculator_method();
void methodHolderDestructor();
void calculator_ParseReturnDestructor();
void calculator_destructor();

/**
Wednesday: Parser Error
Thursday: Parser Error
Friday: Parser Error, other bugs, Satruday's work
Saturday: Create PCA, FA, CCA .c files like Calculator.c -dont forget graphs
Sunday: Finish Saturday's work

Monday: Update Github, construct lib.so, start on Python gui
Tuesday: Python calculator GUI with Treeview
Wednesday: Python calculator GUI with Treeview
Thursday: Python menu GUI
Friday: Python PCA, FA, CCA gui
Saturday: Python PCA, FA, CCA gui
Sunday: Python PCA, FA, CCA gui

**/
