// Implementation file for Parser.h

#include "../Parser.h"

/***
 * 
 * Static Constant Variables and Definitions
 * 
 * ***/ 

char *RESERVEDWORDS[] = {"cos", "sin", "tan", "acos", "asin", "atan", "log", "ln", "e", "PI"};
char *FUNCTIONNAMES[] = {"naiveGauss", "t", "houseHolder", "LU", "QR", "eigen", "integrate", "gamma", 
                            "qchisq", "pchisq", "newton", "bisection", "getRow", "getCol", 
                            "cov", "cor", "zscore", "mean", "backSub", "forwSub", "qnorm", "pnorm"};
char *FUNCTIONNAMES_STATIC[] = {"isSquare", "isSymmetric", "isPosDef", "setRow", "setCol", "delRow", "delCol"};

/***
 * 
 * Data Structures only Below:
 * 
 * ***/ 

union VarValue {
    LUReturn *lu;
    Eigen *e;
    GrahmSchmidt *qr;
    Matrix *mat;
    double *ind;
    char *var;
    double number;
};
typedef union VarValue VarValue;

struct Variable {
    char *ID;
    VarValue value;
    int type; // 0 for number, 1 for matrix, 
};
typedef struct Variable Variable;

union Input {
    double number;
    char operand;
    Variable *var;
};
typedef union Input Input;

struct Entry { // Node for scanner Linked List
    int index; // The Key index
    Input value; // The Value
};
typedef struct Entry Entry;


// Errors:
// 0 : Accept
// 1 : Semantic Error, Unexpected token: '+', ex: 4*+3 (Maybe use 100 values)
// 2 : Missing '(' par
// 3 : Missing ')' par
// 4 : Missing '[' brack
// 5 : Missing ']' brack
// 6 : Missing ',' comma
// 7 : Missing '$' obtain
// 8 : expecting double value
// 9 : expecting matrix value
// 10 : expecting bool value
// 11 : Newton, Derivative option must be NULL
// 12 : unrecognized token 
// 13 : Bisection error : Method requires change of signs over interval
// 14 : Expecting Variable


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

struct Context {
    int error; // not yet implemented
    int isNull;
    VarValue value;
    int type; // 0 for number, 1 for matrix, 2 for bool, 3 for LU, 4 for QR, 5 for Eigen
    char operand;
};
typedef struct Context Context; // Used to hold current value and operands when parsing

struct Parser {
    AVLTree *variables;
};

/****
 * 
 * Static Function declarations and implementations below:
 * 
 * ***/

// Compare function for AVL tree
static int compareAVL(const void *left, const void *right) {
    Variable *l = (Variable*) left;
    Variable *r = (Variable*) right;
    int temp = strcmp(l->ID, r->ID);
    if(temp > 0) return 1;
    else if(temp < 0) return -1;
    else return 0;
}

// Free value function AVL tree
static void freeValueAVL(void *value) {
    Variable *var = (Variable*) value;
    free(var->ID);
    if(var->type==1) {
        matrix_destructor(&var->value.mat);
    }
    free(var);
}

// Replaces all ID's in scanner tokens with their variable values
// Returns NULL if succesful; anything else is an unrecognized variable
static char* updateScanID(Scanner *myScan, Parser *myParser) {
    int iter = 0;
    int before = 0;
    while(scanner_hasNext((myScan))) {
        Entry *e = scanner_next((myScan));
        if(iter==0) {
            if(e->index==0) { // ID
                if(scanner_getSize(myScan) > 2) { // need to make sure before accessing next variable
                    Entry *temp = scanner_next((myScan));
                    if(temp->index==7) { // assignment
                        iter++;
                        continue;
                    }
                }
            }
        }
        iter++;
        if(e->index==0) { // ID
            Variable *t = (Variable*) malloc(sizeof(Variable));
            t->ID = e->value.var->ID;
            Variable *r = avltree_find(myParser->variables, t);
            free(t);
            Entry *temp = scanner_peek(myScan);
            if(r==NULL) { // ID is not found in variables  
                if(temp->index==7 && iter>1) { // this is iteration 1 and next value is =
                    iter++;
                    continue;
                }
                scanner_resetEnumerator(myScan);
                return e->value.var->ID;
            }
            else {
                free(e->value.var->ID);
                free(e->value.var);
                e->index = 1; // set to number
                e->value.number = r->value.number;
            }
        }
    }
    scanner_resetEnumerator(myScan);
    return NULL;
}

static void context_destructor(Context **l) {
    switch((*l)->type) {
        case 1:
        matrix_destructor(&(*l)->value.mat);
        break;
        case 3:
        lu_destructor(&(*l)->value.lu);
        break;
        case 4:
        grahmschmidt_destructor(&(*l)->value.qr);
        break;
        case 5:
        eigen_destructor(&(*l)->value.e);
        break;
    }
    free(*l);
}

// Static Recursive Parsing funcitons

int divZero = 0;
// int error = 0;
static Context* matrix(Scanner *myScan, Parser *myParse, int parse);
static Context* statement(Scanner *myScan, Parser *myParse, int parse);
static Context* factor(Scanner *myScan, Parser *myParse, int parse);
static Context* T2(Scanner *myScan, Parser *myParse, Context *p, int parse);
static Context* term(Scanner *myScan, Parser *myParse, int parse);
static Context* T1(Scanner *myScan, Parser *myParse, Context *p, int parse);
static Context* temp(Scanner *myScan, Parser *myParse, int parse);
static Context *T3(Scanner *myScan, Parser *myParse, Context *p, int parse);
static Context *Reserved(Scanner *myScan,Parser *myParse,  int parse);
static Context * matrixIndex(Scanner *myScan, Parser *myParse, int parse);

static Context* newton(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4)  { // error
        Context *l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->error = 2;
        return l;
    } 
    int size = scanner_getSize(myScan);
    int maxLength = size*3;
    char *s = malloc(sizeof(char)*maxLength);
    for(int i = 0; i < maxLength; i++) {
        s[i] = '\0';
    }
    int index = 0;
    int lpar = 1;
    int derivNULL = 0;
    Context *a;
    while(lpar>0) {
        e = (Entry*) scanner_peek(myScan); // (
        if(e->index==4) lpar++;
        else if(e->index==5) lpar--;
        if(lpar==1 && e->index==14) { // comma
            e = (Entry*) scanner_next(myScan); // consume
            e = (Entry*) scanner_peek(myScan); // deriv or NULL
            if(e->index==0) {
                if(strcmp(e->value.var->ID, "NULL")!=0) {
                    a = (Context*) malloc(sizeof(Context));
                    a->error = 11;
                    a->type = 0;
                    free(s);
                    return a;
                } // error: var must be NULL name
                e = (Entry*) scanner_next(myScan); // consume
                derivNULL=1;
                e = (Entry*) scanner_next(myScan); // COMMA
                if(e->index!=14) {
                    a = (Context*) malloc(sizeof(Context));
                    a->error = 6;
                    a->type = 0;
                    free(s);
                    return a;
                }
                a = statement(myScan, myParse, parse);
                if(a->error > 0) {
                    free(s);
                    return a;
                }

                if(a->type!=0) {
                    free(s);
                    a->error = 8;
                    return a;
                }
                e = (Entry*) scanner_next(myScan); // )
                if(e->index==5) lpar--;
                else { // error: should be a ) here
                    a->error = 3;
                    free(s);
                    return a;
                } 
                if(lpar!=0) { // error: mismatch on number of parenthesis
                    if(lpar>0) a->error = 3;
                    else a->error = 2;
                    free(s);
                    return a;
                }
                continue;
            }
            else { // deriv
                /**
                int comma = 0; 
                while(1) { 
                    e = (Entry*) scanner_peek(myScan); // (
                    if(e->index==14) lpar++;
                    else if(e->index==5) lpar--;
                    if(lpar!=0) {
                        e = (Entry*) scanner_next(myScan); // consume
                        if(e->index==2 || e->index==3 ||e->index==4 ||e->index==5 ||e->index==6 ||
                        e->index==8 ||e->index==9 ||e->index==11 ||e->index==12 ||e->index==13 ||
                        e->index==14) {
                            s[index] = e->value.operand;
                            index++;
                        }
                        else if(e->index==0 || e->index==10 || e->index==15 ||e->index==16) {
                            char *var;
                            int varSize;
                            if(e->index==0) {
                                var = e->value.var->ID;
                                varSize = strlen(var);
                            }
                            else if(e->index==10) {
                                varSize = strlen(RESERVEDWORDS[(int)e->value.number]);
                                var = malloc(sizeof(char)*(varSize+1));
                                strcpy(var, RESERVEDWORDS[(int)e->value.number]);
                            }
                            else if(e->index==15) {
                                varSize = strlen(FUNCTIONNAMES[(int)e->value.number]);
                                var = malloc(sizeof(char)*(varSize+1));
                                strcpy(var, FUNCTIONNAMES[(int)e->value.number]);
                            }
                            else if(e->index==16) {
                                varSize = strlen(FUNCTIONNAMES_STATIC[(int)e->value.number]);
                                var = malloc(sizeof(char)*(varSize+1));
                                strcpy(var, FUNCTIONNAMES_STATIC[(int)e->value.number]);
                            }
                            if(varSize+index >= maxLength-1) {
                                
                                s=realloc(s, maxLength*=2);
                                for(int i = index; i < maxLength; i++) {
                                    s[i] = '\0';
                                }
                            }
                            strcat(s, var);
                            index += varSize;
                            printf("%s\n", s);
                            // add variable
                        }
                        else if(e->index==1) {
                            char *number = malloc(sizeof(char)*sizeof(e->value.number));
                            if((int) e->value.number ==e->value.number) sprintf(number, "%d", (int) e->value.number);
                            else sprintf(number,"%.10f", e->value.number);
                            int numSize = strlen(number);
                            if(numSize+index >= maxLength-1) {
                                
                                s=realloc(s, maxLength*=2);
                                for(int i = index; i < maxLength; i++) {
                                    s[i] = '\0';
                                }
                            }
                            strcat(s, number);
                            free(number);
                            index += numSize;
                        }
                        else ; 
                    }
                }
                **/
            }
        }
        if(lpar!=0) {
            e = (Entry*) scanner_next(myScan); // consume
            if(e->index==2 || e->index==3 ||e->index==4 ||e->index==5 ||e->index==6 ||
            e->index==8 ||e->index==9 ||e->index==11 ||e->index==12 ||e->index==13 ||
            e->index==14) {
                s[index] = e->value.operand;
                index++;
            }
            else if(e->index==0 || e->index==10 || e->index==15 ||e->index==16) {
                char *var;
                int varSize;
                if(e->index==0) {
                    var = e->value.var->ID;
                    varSize = strlen(var);
                }
                else if(e->index==10) {
                    varSize = strlen(RESERVEDWORDS[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, RESERVEDWORDS[(int)e->value.number]);
                }
                else if(e->index==15) {
                    varSize = strlen(FUNCTIONNAMES[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, FUNCTIONNAMES[(int)e->value.number]);
                }
                else if(e->index==16) {
                    varSize = strlen(FUNCTIONNAMES_STATIC[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, FUNCTIONNAMES_STATIC[(int)e->value.number]);
                }
                if(varSize+index >= maxLength-1) {
                    
                    s=realloc(s, maxLength*=2);
                    for(int i = index; i < maxLength; i++) {
                        s[i] = '\0';
                    }
                }
                strcat(s, var);
                index += varSize;
                // add variable
            }
            else if(e->index==1) {
                char *number = malloc(sizeof(char)*sizeof(e->value.number));
                if((int) e->value.number ==e->value.number) sprintf(number, "%d", (int) e->value.number);
                else sprintf(number,"%.10f", e->value.number);
                int numSize = strlen(number);
                if(numSize+index >= maxLength-1) {
                    
                    s=realloc(s, maxLength*=2);
                    for(int i = index; i < maxLength; i++) {
                        s[i] = '\0';
                    }
                }
                strcat(s, number);
                free(number);
                index += numSize;
            }
            else ; 
        }
    }
    Scanner *tempScan;
    scanner_initialize(&tempScan);
    int unrec = scanner_scan(tempScan, s);
    if(unrec != 1) {
        a->error = 12;
        a->operand = (char) unrec;
        free(s);
        scanner_destructor(&tempScan);
        return a;
    }
    Parser *tempParse;
    parser_initialize(&tempParse);
    ParseReturn *root;
    if(derivNULL) root = root_newton(tempScan, NULL, tempParse, NULL, a->value.number);
  //  else root = root_bisection(tempScan, tempParse, a->value.number, 3);
    
    scanner_destructor(&tempScan);
    parser_destructor(&tempParse);
    free(a);
    Context *l = (Context*) malloc(sizeof(Context));
    l->isNull = 0;
    l->type = root->type;
    if(root->error==0) l->value.number = parseReturn_getValue(root);
    l->error = root->error;
    parseReturn_destructor(&root);
    free(s);
    return l;
}


static Context* bisection(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4)  { // error
        Context *l = (Context*) malloc(sizeof(Context));
        l->error = 2;
        l->type = 0;
        return l;
    } 
    int size = scanner_getSize(myScan);
    int maxLength = size*3;
    char *s = malloc(sizeof(char)*maxLength);
    for(int i = 0; i < maxLength; i++) {
        s[i] = '\0';
    }
    int index = 0;
    int lpar = 1;
    Context *a;
    Context *b;
    while(lpar>0) {
        e = (Entry*) scanner_peek(myScan); // (
        if(e->index==4) lpar++;
        else if(e->index==5) lpar--;
        if(lpar==1 && e->index==14) { // comma
            e = (Entry*) scanner_next(myScan); // consume
            a = statement(myScan, myParse, parse);
            if(a->error > 0) {
                free(s);
                return a;
            }
            if(a->type!=0) {
                a->error = 8;
                free(s);
                return a;
            }
            e = (Entry*) scanner_next(myScan); // comma
            if(e->index!=14) {
                a->error = 6;
                free(s);
                return a;
            }
            b = statement(myScan, myParse, parse);
            if(b->error > 0) {
                // a should be of type double
                free(a);
                free(s);
                return b;
            }
            if(b->type!=0) {
                b->error = 8;
                // a should be of type double
                free(a);
                free(s);
                return b;
            }
            e = (Entry*) scanner_next(myScan); // )
            if(e->index==5) lpar--;
            else { // error: should be a ) here
                free(a);
                free(s);
                b->error = 3;
                return b;
            } 
            if(lpar!=0) { // error: mismatch on number of parenthesis
                free(a);
                if(lpar>0) b->error = 3;
                else b->error = 2;
                free(s);
                return b;
            }
            continue;
        }
        if(lpar!=0) {
            e = (Entry*) scanner_next(myScan); // consume
            if(e->index==2 || e->index==3 ||e->index==4 ||e->index==5 ||e->index==6 ||
            e->index==8 ||e->index==9 ||e->index==11 ||e->index==12 ||e->index==13 ||
            e->index==14) {
                s[index] = e->value.operand;
                index++;
            }
            else if(e->index==0 || e->index==10 || e->index==15 ||e->index==16) {
                char *var;
                int varSize;
                if(e->index==0) {
                    var = e->value.var->ID;
                    varSize = strlen(var);
                }
                else if(e->index==10) {
                    varSize = strlen(RESERVEDWORDS[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, RESERVEDWORDS[(int)e->value.number]);
                }
                else if(e->index==15) {
                    varSize = strlen(FUNCTIONNAMES[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, FUNCTIONNAMES[(int)e->value.number]);
                }
                else if(e->index==16) {
                    varSize = strlen(FUNCTIONNAMES_STATIC[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, FUNCTIONNAMES_STATIC[(int)e->value.number]);
                }
                if(varSize+index >= maxLength-1) {
                    
                    s=realloc(s, maxLength*=2);
                    for(int i = index; i < maxLength; i++) {
                        s[i] = '\0';
                    }
                }
                strcat(s, var);
                index += varSize;
                // add variable
            }
            else if(e->index==1) {
                char *number = malloc(sizeof(char)*sizeof(e->value.number));
                if((int) e->value.number ==e->value.number) sprintf(number, "%d", (int) e->value.number);
                else sprintf(number,"%.10f", e->value.number);
                int numSize = strlen(number);
                if(numSize+index >= maxLength-1) {
                    
                    s=realloc(s, maxLength*=2);
                    for(int i = index; i < maxLength; i++) {
                        s[i] = '\0';
                    }
                }
                strcat(s, number);
                free(number);
                index += numSize;
            }
            else ; 
        }
    }
    Scanner *tempScan;
    scanner_initialize(&tempScan);
    int unrec = scanner_scan(tempScan, s);
    if(unrec != 1) {
        free(a);
        b->error = 12;
        b->operand = (char) unrec;
        free(s);
        scanner_destructor(&tempScan);
        return b;
    }
    Parser *tempParse;
    parser_initialize(&tempParse);
    ParseReturn *root = root_bisection(tempScan, tempParse, a->value.number, b->value.number);
    scanner_destructor(&tempScan);
    parser_destructor(&tempParse);
    free(a);
    free(b);
    Context *l = (Context*) malloc(sizeof(Context));
    l->isNull = 0;
     l->type = root->type;
    l->error = root->error;
    if(root->error==0) l->value.number = parseReturn_getValue(root);
    parseReturn_destructor(&root);
    free(s);
    return l;
}


static Context* integrate(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4)  {
        Context *l = (Context*) malloc(sizeof(Context));
        l->error = 2;
         l->type = 0;
        return l;
    } 
    int size = scanner_getSize(myScan);
    int maxLength = size*3;
    char *s = malloc(sizeof(char)*maxLength);
    for(int i = 0; i < maxLength; i++) {
        s[i] = '\0';
    }
    int index = 0;
    int lpar = 1;
    int infty = 0;
    Context *a;
    Context *b;
    while(lpar>0) {
        e = (Entry*) scanner_peek(myScan); // (
        if(e->index==4) lpar++;
        else if(e->index==5) lpar--;
        if(lpar==1 && e->index==14) { // comma
            e = (Entry*) scanner_next(myScan); // consume
            a = statement(myScan, myParse, parse);
            if(a->error >0) {
                free(s);
                return a;
            }
            if(a->type!=0) {
                a->error = 8;
                free(s);
                return a;
            }
            e = (Entry*) scanner_next(myScan); // comma
            if(e->index!=14) {
                a->error = 6;
                free(s);
                return a;
            }
            e = (Entry*) scanner_peek(myScan); // is statement or infnty?
            if(e->index==0) {
                if(strcmp(e->value.var->ID,"infty")==0) {
                    infty = 1;
                    e = (Entry*) scanner_next(myScan); // consume
                    e = (Entry*) scanner_next(myScan); // consume
                    if(e->index==5) lpar--;
                    else { // error: should be a ) here
                        a->error = 3;
                        free(s);
                        return a;
                    } 
                    if(lpar!=0) { // error: mismatch on number of parenthesis
                        if(lpar>0) a->error = 3;
                        else a->error = 2;
                        free(s);
                        return a;
                    }
                }
                else {
                	a->error = 8;
                	free(s);
                	return a;
				}
            }
            else { // statement
                b = statement(myScan, myParse, parse);
                if(b->error > 0) {
                    free(s);
                    return b;
                }
                if(b->type!=0) {
                    free(a);
                    b->error = 8;
                    free(s);
                    return b;
                }
                e = (Entry*) scanner_next(myScan); // )
                if(e->index==5) lpar--;
                else { // error: should be a ) here
                    free(a);
                    b->error = 3;
                    free(s);
                    return b;
                } 
                if(lpar!=0) { // error: mismatch on number of parenthesis
                    free(a);
                    if(lpar>0) b->error = 3;
                    else b->error = 2;
                    free(s);
                    return b;
                }
            }
            continue;
        }
        if(lpar!=0) {
            e = (Entry*) scanner_next(myScan); // consume
            if(e->index==2 || e->index==3 ||e->index==4 ||e->index==5 ||e->index==6 ||
            e->index==8 ||e->index==9 ||e->index==11 ||e->index==12 ||e->index==13 ||
            e->index==14) {
                s[index] = e->value.operand;
                index++;
            }
            else if(e->index==0 || e->index==10 || e->index==15 ||e->index==16) {
                char *var;
                int varSize;
                if(e->index==0) {
                    var = e->value.var->ID;
                    varSize = strlen(var);
                }
                else if(e->index==10) {
                    varSize = strlen(RESERVEDWORDS[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, RESERVEDWORDS[(int)e->value.number]);
                }
                else if(e->index==15) {
                    varSize = strlen(FUNCTIONNAMES[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, FUNCTIONNAMES[(int)e->value.number]);
                }
                else if(e->index==16) {
                    varSize = strlen(FUNCTIONNAMES_STATIC[(int)e->value.number]);
                    var = malloc(sizeof(char)*(varSize+1));
                    strcpy(var, FUNCTIONNAMES_STATIC[(int)e->value.number]);
                }
                if(varSize+index >= maxLength-1) {
                    
                    s=realloc(s, maxLength*=2);
                    for(int i = index; i < maxLength; i++) {
                        s[i] = '\0';
                    }
                }
                strcat(s, var);
                index += varSize;
                // add variable
            }
            else if(e->index==1) {
                char *number = malloc(sizeof(char)*sizeof(e->value.number));
                if((int) e->value.number ==e->value.number) sprintf(number, "%d", (int) e->value.number);
                else sprintf(number,"%.10f", e->value.number);
                int numSize = strlen(number);
                if(numSize+index >= maxLength-1) {
                    
                    s=realloc(s, maxLength*=2);
                    for(int i = index; i < maxLength; i++) {
                        s[i] = '\0';
                    }
                }
                strcat(s, number);
                free(number);
                index += numSize;
            }
            else ; // error
            // char 2-6, 8-9 11-14, 
            // string: 0, 7, 15, 16
            // number: 1 
        }
    }
    Scanner *tempScan;
    scanner_initialize(&tempScan);
    int unrec = scanner_scan(tempScan, s);
    if(unrec != 1) {
        if(!infty) {
            free(b);
        }
        a->error = 12;
        a->operand = (char) unrec;
        free(s);
        return a;
    }
    Parser *tempParse;
    parser_initialize(&tempParse);
    ParseReturn *val;
    if(infty) val = integration_infinityBounds(tempScan, tempParse, a->value.number);
    else val = integration_modifiedAdaptiveSimpQuadrature(tempScan, tempParse, a->value.number, b->value.number);
    scanner_destructor(&tempScan);
    parser_destructor(&tempParse);
    Context *l = (Context*) malloc(sizeof(Context));
    l->isNull = 0;
    l->type = val->type;
    free(a);
    if(!infty) free(b);
    if(val->error==0) l->value.number = val->value;
    l->error = val->error;
    parseReturn_destructor(&val);
    free(s);
    return l;
}

static Context* zscore(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        Context *l = (Context*) malloc(sizeof(Context));
        l->error = 2;
        l->type = 0;
        return l;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan); // )
    if(e->index==5) {
        Matrix *myCor = multi_zscores(l->value.mat, NULL, NULL);
        matrix_destructor(&l->value.mat);
        l->value.mat = myCor;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* cor(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        Context *l = (Context*) malloc(sizeof(Context));
        l->error = 2;
        l->type = 0;
        return l;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan); // )
    if(e->index==5) {
        Matrix *myCor = multi_correlation(l->value.mat, NULL);
        matrix_destructor(&l->value.mat);
        l->value.mat = myCor;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* forwSub(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        Context *l = (Context*) malloc(sizeof(Context));
        l->error = 2;
        l->type = 0;
        return l;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan); // Comma
    if(e->index == 14) {
        e = (Entry*) scanner_peek(myScan); // ? NULL ?
        if(e->index ==0) {
            if(strcmp(e->value.var->ID, "NULL")==0) {
                e = (Entry*) scanner_next(myScan); // consume
                e = (Entry*) scanner_next(myScan); // )
                if(e->index==5) {
                    Matrix *sol = LAalgorithms_backSub(l->value.mat, NULL);
                    if(sol==NULL) {
                        l->error = 41;
                        return l;
                    }
                    matrix_destructor(&l->value.mat);
                    l->value.mat = sol;
                    return l;
                }
                else {
                    l->error = 3;
                    return l;
                }
            }
        }
        Context *r = statement(myScan, myParse, parse); // arg 2
        if(r->error > 0) {
            context_destructor(&l);
            return r;
        }
        if(r->type!=1) {
            context_destructor(&l);
            r->error = 9;
            return r;
        }
        e = (Entry*) scanner_next(myScan); // )
        if(e->index==5) {
            Matrix *sol = LAalgorithms_forwSub(l->value.mat, r->value.mat);
            context_destructor(&l);
            if(sol==NULL) {
                r->error = 41;
                return r;
            }
            matrix_destructor(&r->value.mat);
            r->value.mat = sol;
            return r;
        }
        else {
            context_destructor(&l);
            r->error = 3;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}
// backSub(x, y)
// backSub(x, NULL)

static Context* backSub(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        Context *l = (Context*) malloc(sizeof(Context));
        l->error = 2;
        l->type = 0;
        return l;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan); // Comma
    if(e->index == 14) {
        e = (Entry*) scanner_peek(myScan); // ? NULL ?
        if(e->index ==0) {
            if(strcmp(e->value.var->ID, "NULL")==0) {
                e = (Entry*) scanner_next(myScan); // consume
                e = (Entry*) scanner_next(myScan); // )
                if(e->index==5) {
                    Matrix *sol = LAalgorithms_backSub(l->value.mat, NULL);
                    if(sol==NULL) {
                        l->error = 41;
                        return l;
                    }
                    matrix_destructor(&l->value.mat);
                    l->value.mat = sol;
                    return l;
                }
                else {
                    l->error = 3;
                    return l;
                }
            }
        }
        Context *r = statement(myScan, myParse, parse); // arg 2
        if(r->error > 0) {
            context_destructor(&l);
            return r;
        }
        if(r->type!=1) {
            context_destructor(&l);
            r->error = 9;
            return r;
        }
        e = (Entry*) scanner_next(myScan); // )
        if(e->index==5) {
            Matrix *sol = LAalgorithms_backSub(l->value.mat, r->value.mat);
            context_destructor(&l);
            if(sol==NULL) {
                r->error = 41;
                return r;
            }
            matrix_destructor(&r->value.mat);
            r->value.mat = sol;
            return r;
        }
        else {
            context_destructor(&l);
            r->error = 3;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}

static Context* mean(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        Context *l = (Context*) malloc(sizeof(Context));
        l->error = 2;
        l->type = 0;
        return l;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan); // )
    if(e->index==5) {
        Matrix *myCor = multi_meanVector(l->value.mat);
        matrix_destructor(&l->value.mat);
        l->value.mat = myCor;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* cov(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        Context *l = (Context*) malloc(sizeof(Context));
        l->error = 2;
        l->type = 0;
        return l;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan); // )
    if(e->index==5) {
        Matrix *myCov = multi_covariance(l->value.mat, NULL);
        matrix_destructor(&l->value.mat);
        l->value.mat = myCov;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

struct parser_EigenReturn {
    Eigen *e;
    int type;  // 0 for complete Eigen, 1 for values, 2 for vectors
    int error; // same as context
};
typedef struct parser_EigenReturn parser_EigenReturn;

static parser_EigenReturn* eigen(Scanner *myScan, Parser *myParse, int parse) {
    parser_EigenReturn *ret = (parser_EigenReturn*) malloc(sizeof(parser_EigenReturn));
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        ret->error = 2; 
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) {
        ret->error = l->error; 
        context_destructor(&l);
        return ret;
    }
    if(l->type!=1) {
        ret->error = 9;
        context_destructor(&l);
        return ret;
    }
    e = (Entry*) scanner_next(myScan); // $ or )
   
    if(e->index==5) { // )
        Eigen *myE = eigen_Solve(l->value.mat);
        matrix_destructor(&l->value.mat);
        free(l);
        if(myE==NULL) {
            ret->error = 15;
            return ret;
        }
        e = (Entry*) scanner_peek(myScan); // $ or nothing
        if(e->index==13) {
            e = (Entry*) scanner_next(myScan); //  consume $
            e = (Entry*) scanner_next(myScan); // values or vectors
            if(e->index!=0) {
                ret->error = 20;
                eigen_destructor(&myE);
                return ret;
            }
            if(strcmp("values", e->value.var->ID)==0) {
                ret->e = myE;
                ret->type = 1;
                ret->error = 0;
                return ret;
            }
            else if(strcmp("vectors", e->value.var->ID)==0) {
                ret->e = myE;
                ret->type = 2;
                ret->error = 0;
                return ret;
            }
            else {
                ret->error = 16;
                eigen_destructor(&myE);
                return ret;
            }
        }
        else {
            ret->e = myE;
            ret->type = 0;
            ret->error = 0;
            return ret;
        }
    }
    else {
        ret->error = 3;
        context_destructor(&l);
        return ret;
    }
   
}

struct parser_QRReturn {
    GrahmSchmidt *qr;
    int type;  // 0 for complete QR, 1 for Q, 2 for R, 3 for solved 
    int error;
};
typedef struct parser_QRReturn parser_QRReturn;

static  parser_QRReturn* QR(Scanner *myScan, Parser *myParse, int parse) {
    parser_QRReturn *ret = ( parser_QRReturn*) malloc(sizeof( parser_QRReturn));
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        ret->error = 2; 
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) {
        ret->error = l->error; 
        context_destructor(&l);
        return ret;
    }
    if(l->type!=1) {
        ret->error = 9;
        context_destructor(&l);
        return ret;
    }
    e = (Entry*) scanner_next(myScan); //  Comma or $ or )
    GrahmSchmidt *myQR;
    grahmschmidt_initialize(&myQR);
   
    if(e->index==14) { // Comma
        Context *r = statement(myScan, myParse, parse); // arg 2
        if(r->error >0) {
            ret->error = r->error;
            matrix_destructor(&l->value.mat);
            free(l);
            free(r);
            free(myQR);
            return ret;
        }
        if(r->type!=1) {
            ret->error = 9;
            matrix_destructor(&l->value.mat);
            free(l);
            free(r);
            free(myQR);
            return ret;
        }
        e = (Entry*) scanner_next(myScan); // )
        if(e->index==5) {
            grahmschmidt_QR(myQR, l->value.mat, r->value.mat);
            ret->qr = myQR;
            ret->type = 3;
            matrix_destructor(&l->value.mat);
            free(l);
            matrix_destructor(&r->value.mat);
            free(r);
            ret->error = 0;
            return ret;
        }
        else {
            matrix_destructor(&l->value.mat);
            free(l);
            matrix_destructor(&r->value.mat);
            free(r);
            ret->error = 3;
            free(myQR);
            return ret;
        }
    }
    else if(e->index==5) { // )
        e = (Entry*) scanner_peek(myScan); // $
        grahmschmidt_QR(myQR, l->value.mat, NULL);
        matrix_destructor(&l->value.mat);
        free(l);
        
        if(e->index==13) { // $
            e = (Entry*) scanner_next(myScan); //  consume $
            e = (Entry*) scanner_next(myScan); // Q or R
            if(e->index!=0) {
                ret->error = 20;
                grahmschmidt_destructor(&myQR);
                return ret;
            }
            if(strcmp("Q", e->value.var->ID)==0) {
                ret->qr = myQR;
                ret->type = 1;
                ret->error = 0;
                return ret;
            }
            else if(strcmp("R", e->value.var->ID)==0) {
                ret->qr = myQR;
                ret->type = 2;
                ret->error = 0;
                return ret;
            }
            else {
                grahmschmidt_destructor(&myQR);
                ret->error = 17;
                return ret;
            }
        }
        else {
            ret->qr = myQR;
            ret->type = 0;
            ret->error = 0;
            return ret;
        }
    }
    else {
        ret->error = 3;
        free(l);
        free(myQR);
        return ret;
    }
}

struct parser_LUReturn {
    LUReturn *lu;
    int type; // 0 for complete LU, 1 for L, 2 for U, 3 for solved 
    int error;
};
typedef struct parser_LUReturn parser_LUReturn;

static parser_LUReturn* LU(Scanner *myScan, Parser *myParse, int parse) {
    parser_LUReturn *ret = (parser_LUReturn*) malloc(sizeof(parser_LUReturn));
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) { // error
        ret->error = 2; 
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) {
        ret->error = l->error; 
        context_destructor(&l);
        return ret;
    }
    if(l->type!=1) {
        ret->error = 9;
        context_destructor(&l);
        return ret;
    }
    e = (Entry*) scanner_next(myScan); //  Comma or $ or )
    LUReturn *myLU;
     // LU(m) LU(m)$L LU(m)$U LU(m,b)
    if(e->index==14) { // Comma
        Context *r = statement(myScan, myParse, parse); // arg 2
        if(r->error >0) {
            ret->error = r->error;
            matrix_destructor(&l->value.mat);
            free(l);
            free(r);
            return ret;
        }
        if(r->type!=1) {
            ret->error = 9;
            matrix_destructor(&l->value.mat);
            free(l);
            free(r);
            return ret;
        }
        e = (Entry*) scanner_next(myScan); // )
        if(e->index==5) {
            myLU = lu_decompose(l->value.mat, r->value.mat);
            if(myLU==NULL) {
                ret->error = 19;
                free(r);
                return ret;
            }
            ret->lu = myLU;
            ret->type = 3;
            ret->error = 0;
            matrix_destructor(&l->value.mat);
            matrix_destructor(&r->value.mat);
            free(l);
            free(r);
            return ret;
        }
        else {
            ret->error = 3;
            matrix_destructor(&l->value.mat);
            free(l);
            matrix_destructor(&r->value.mat);
            free(r);
            return ret;
        }
    }
    else if(e->index==5) { // )
        Entry* s = (Entry*) scanner_peek(myScan); // $
        myLU = lu_decompose(l->value.mat, NULL);
        matrix_destructor(&l->value.mat);
        free(l);
        if(myLU==NULL) {
            ret->error = 19;
            return ret;
        }
        if(s->index==13) { // $
            e = (Entry*) scanner_next(myScan); //  consume $
            e = (Entry*) scanner_next(myScan); // L or U
            if(e->index!=0) {
                ret->error = 20;
                lu_destructor(&myLU);
                return ret;
            }
            if(strcmp("L", e->value.var->ID)==0) {
                ret->lu = myLU;
                ret->type = 1;
                ret->error = 0;
                return ret;
            }
            else if(strcmp("U", e->value.var->ID)==0) {
                ret->lu = myLU;
                ret->type = 2;
                ret->error = 0;
                return ret;
            }
            else {
                ret->error = 18;
                lu_destructor(&myLU);
                return ret;
            }
        }
        else {
            ret->lu = myLU;
            ret->type = 0;
            ret->error = 0;
            return ret;
        }
        
    }
    else {
        ret->error = 3;
        free(l);
        return ret;
    }
   
}


static Context* deleteRow(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    Context *ret = (Context*) malloc(sizeof(Context));
    ret->type = 0;
    if(e->index != 4) {
        ret->error = 2;
        return ret;
    }
    e = (Entry*) scanner_next(myScan); // (
    if(e->index!=0) {
        ret->error = 14;
        return ret;
    }
    Variable *temp = avltree_find(myParse->variables, e->value.var);
    if(temp==NULL) {
        ret->error = 37;
        return ret;
    }
    if(temp->type != 1) {
        ret->error = 9;
        return ret;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan); // )
        if(r->type!=0) {
            context_destructor(&ret);
            r->error = 8;
            return r;
        }
        if(e->index==5) {
            Matrix *ng = matrix_deleteRow(&temp->value.mat, r->value.number, 1);
            free(ret);
            r->value.number = !(ng==NULL);
            if(r->value.number==0) r->value.number = -1;
            if(r->value.number==1) r->value.number = 2;
            return r;    
        }
        else {
            context_destructor(&ret);
            r->error = 3;
            return r;
        }
    }
    else {
        ret->error = 6;
        return ret;
    }
}

static Context* deleteCol(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    Context *ret = (Context*) malloc(sizeof(Context));
    ret->type = 0;
    if(e->index != 4) {
        ret->error = 2;
        return ret;
    }
    e = (Entry*) scanner_next(myScan); // (
    if(e->index!=0) {
        ret->error = 14;
        return ret;
    }
    Variable *temp = avltree_find(myParse->variables, e->value.var);
    if(temp==NULL) {
        ret->error = 37;
        return ret;
    }
    if(temp->type != 1) {
        ret->error = 9;
        return ret;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan); // )
        if(r->type!=0) {
            context_destructor(&ret);
            r->error = 8;
            return r;
        }
        if(e->index==5) {
            Matrix *ng = matrix_deleteCol(&temp->value.mat, r->value.number, 1);
            free(ret);
            r->value.number = !(ng==NULL);
            if(r->value.number==0) r->value.number = -1;
            if(r->value.number==1) r->value.number = 2;
            return r;    
        }
        else {
            context_destructor(&ret);
            r->error = 3;
            return r;
        }
    }
    else {
        ret->error = 6;
        return ret;
    }
}

static Context* setRow(Scanner *myScan, Parser *myParse, int parse) {
Entry *e = (Entry*) scanner_next(myScan); // (
    Context *ret = (Context*) malloc(sizeof(Context));
    ret->type = 0;
    if(e->index != 4) {
        ret->error = 2;
        return ret;
    }
    e = (Entry*) scanner_next(myScan); // (
    if(e->index!=0) {
        ret->error = 14;
        return ret;
    }
    Variable *temp = avltree_find(myParse->variables, e->value.var);
    if(temp==NULL) {
        ret->error = 37;
        return ret;
    }
    if(temp->type != 1) {
        ret->error = 9;
        return ret;
    }
    e = (Entry*) scanner_next(myScan); // comma
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan); // comma
        if(r->type!=1) {
            free(ret);
            r->error = 9;
            return r;
        }
        if(e->index==14) {
            Context *i = statement(myScan, myParse, parse); // arg 3
            e = (Entry*) scanner_next(myScan); // )
            if(e->index==5) {
                int myBool = matrix_setRow(temp->value.mat, r->value.mat, i->value.number);
                free(ret);
                context_destructor(&r);
                if(myBool==0) myBool = -1;
                if(myBool==1) myBool = 2;
                i->value.number = myBool;
                return i;  
            }
            else {
                free(ret);
                context_destructor(&r);
                i->error = 3;
                return i;
            }
        }
        else {
            free(ret);
            r->error = 6;
            return r;
        }
    }
    else {
        ret->error = 6;
        return ret;
    }
}

static Context* setCol(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    Context *ret = (Context*) malloc(sizeof(Context));
    ret->type = 0;
    if(e->index != 4) {
        ret->error = 2;
        return ret;
    }
    e = (Entry*) scanner_next(myScan); // (
    if(e->index!=0) {
        ret->error = 14;
        return ret;
    }
    Variable *temp = avltree_find(myParse->variables, e->value.var);
    if(temp==NULL) {
        ret->error = 37;
        return ret;
    }
    if(temp->type != 1) {
        ret->error = 9;
        return ret;
    }
    e = (Entry*) scanner_next(myScan); // comma
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan); // comma
        if(r->type!=1) {
            free(ret);
            r->error = 9;
            return r;
        }
        if(e->index==14) {
            Context *i = statement(myScan, myParse, parse); // arg 3
            e = (Entry*) scanner_next(myScan); // )
            if(e->index==5) {
                int myBool = matrix_setCol(temp->value.mat, r->value.mat, i->value.number);
                free(ret);
                context_destructor(&r);
                if(myBool==0) myBool = -1;
                if(myBool==1) myBool = 2;
                i->value.number = myBool;
                return i;  
            }
            else {
                free(ret);
                context_destructor(&r);
                i->error = 3;
                return i;
            }
        }
        else {
            free(ret);
            r->error = 6;
            return r;
        }
    }
    else {
        ret->error = 6;
        return ret;
    }
}

static Context* isPosDef(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1 
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==5) { 
        int ret = LAalgorithms_isPositiveDefinite(l->value.mat);
        context_destructor(&l);
        l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->value.number = ret;
        l->error = 0;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* isSquare(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1 
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==5) { 
        int ret = matrix_isSquare(l->value.mat);
        context_destructor(&l);
        l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->error = 0;
        l->value.number = ret;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* isSymmetric(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1 
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==5) { 
        int ret = matrix_isSymmetric(l->value.mat);
        context_destructor(&l);
        l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->error = 0;
        l->value.number = ret;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* staticFunction(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // staticFunction
    int index = e->value.number;
    e = (Entry*) scanner_peek(myScan); // (
    if(e->index==4) {
        switch(index) {
            case 0:
                return isSquare(myScan, myParse, parse);
            break;
            case 1:
                return isSymmetric(myScan, myParse, parse);
            break;
            case 2:
                return isPosDef(myScan, myParse, parse);
            break;
            case 3:
                return setRow(myScan, myParse, parse);
            break;
            case 4:
                return setCol(myScan, myParse, parse);
            break;
            case 5:
                return deleteRow(myScan, myParse, parse);
            break;
            case 6:
                return deleteCol(myScan, myParse, parse);
            break;
        }
    }
    else ; // error
}

// start here tonight or tommorow

static Context* pchisq(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1 
    if(l->error > 0) return l;
    if(l->type!=0) {
        l->error = 8;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan); // )
        if(r->error > 0) {
            context_destructor(&l);
            return r;
        }
        if(r->type!=0) {
            context_destructor(&l);
            r->error = 8;
            return r;
        }
        if(e->index==5) {
            l->value.number = distributions_pchisq(l->value.number, r->value.number);
            free(r);
            return l;    
        }
        else {
            context_destructor(&l);
            r->error = 3;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}

static Context* pnorm(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1 
    if(l->error > 0) return l;
    if(l->type!=0) {
        l->error = 8;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan); // comma
        if(r->error > 0) {
            context_destructor(&l);
            return r;
        }
        if(r->type!=0) {
            context_destructor(&l);
            r->error = 8;
            return r;
        }
        if(e->index==14) {
            Context *m = statement(myScan, myParse, parse); // arg 3
            e = (Entry*) scanner_next(myScan); // )
            if(m->error > 0) {
                context_destructor(&l);
                context_destructor(&r);
                return m;
            }
            if(m->type!=0) {
                context_destructor(&l);
                context_destructor(&r);
                m->error = 8;
                return m;
            }
            if(e->index==5) {
            	if(m->value.number ==0) {
            		context_destructor(&l);
	                context_destructor(&r);
	                m->error = 45;
	                return m;
				}
                m->value.number = distributions_pnorm(l->value.number, r->value.number, m->value.number);
                free(r);
                free(l);
                return m; 
            }
            else {
                context_destructor(&l);
                context_destructor(&r);
                m->error = 3;
                return m;
            }   
        }
        else {
            context_destructor(&l);
            r->error = 6;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}

static Context* qnorm(Scanner *myScan, Parser *myParse, int parse) {
     Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1 
    if(l->error > 0) return l;
    if(l->type!=0) {
        l->error = 8;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan); // comma
        if(r->error > 0) {
            context_destructor(&l);
            return r;
        }
        if(r->type!=0) {
            context_destructor(&l);
            r->error = 8;
            return r;
        }
        if(e->index==14) {
            Context *m = statement(myScan, myParse, parse); // arg 3
            e = (Entry*) scanner_next(myScan); // )
            if(m->error > 0) {
                context_destructor(&l);
                context_destructor(&r);
                return m;
            }
            if(m->type!=0) {
                context_destructor(&l);
                context_destructor(&r);
                m->error = 8;
                return m;
            }
            if(e->index==5) {
            	if(m->value.number ==0) {
            		context_destructor(&l);
	                context_destructor(&r);
	                m->error = 45;
	                return m;
				}
                m->value.number = distributions_qnorm(l->value.number, r->value.number, m->value.number);
                free(r);
                free(l);
                return m; 
            }
            else {
                context_destructor(&l);
                context_destructor(&r);
                m->error = 3;
                return m;
            }   
        }
        else {
            context_destructor(&l);
            r->error = 6;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}

static Context* qchisq(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1 
    if(l->error > 0) return l;
    if(l->type!=0) {
        l->error = 8;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan); // )
        if(r->error > 0) {
            context_destructor(&l);
            return r;
        }
        if(r->type!=0) {
            context_destructor(&l);
            r->error = 8;
            return r;
        }
        if(e->index==5) {
            l->value.number = distributions_qchisq(l->value.number, r->value.number);
            if(l->value.number==-1) {
                context_destructor(&l);
                r->error = 21;
                return r;
            } // error => first arg is not between 0 and 1 for probability
            free(r);
            return l;    
        }
        else {
            context_destructor(&l);
            r->error = 3;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}

static Context* gammaValue(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=0) {
        l->error = 8;
        return l;
    }
    e = (Entry*) scanner_next(myScan); // )
    if(e->index==5) {
       l->value.number = distributions_gammaValue(l->value.number);
       return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* getCol(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    Matrix *ng;
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        if(r->error >0) {
            context_destructor(&l);
            return r;
        }
        e = (Entry*) scanner_next(myScan); // )
        if(r->type!=0) {
            context_destructor(&l);
            r->error = 8;
            return r;
        }
        if(e->index==5) {
            ng = matrix_getCol(l->value.mat, r->value.number);
            if(ng==NULL) {
                context_destructor(&l);
                r->error = 22;
                return r;
            }
            free(r);
            matrix_destructor(&l->value.mat);
            l->value.mat = ng;
            return l;    
        }
        else {
            context_destructor(&l);
            r->error = 3;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}

static Context* getRow(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    Matrix *ng;
    if(e->index==14) {
        Context *r = statement(myScan, myParse, parse); // arg 2
        if(r->error >0) {
            context_destructor(&l);
            return r;
        }
        e = (Entry*) scanner_next(myScan); // )
        if(r->type!=0) {
            context_destructor(&l);
            r->error = 8;
            return r;
        }
        if(e->index==5) {
            ng = matrix_getRow(l->value.mat, r->value.number);
            if(ng==NULL) {
                context_destructor(&l);
                r->error = 23;
                return r;
            }
            free(r);
            matrix_destructor(&l->value.mat);
            l->value.mat = ng;
            return l;    
        }
        else {
            context_destructor(&l);
            r->error = 3;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}

static Context* transpose(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    Matrix *ng;
    if(e->index ==5) {
        ng = matrix_transpose(l->value.mat, 0);
        matrix_destructor(&l->value.mat);
        l->value.mat = ng;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* houseHolder(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    Matrix *ng;
    if(e->index ==5) {
        ng = LAalgorithms_HouseHoldersMethod(l->value.mat);
        if(ng==NULL) {
            l->error = 24;
            return l;
        }
        matrix_destructor(&l->value.mat);
        l->value.mat = ng;
        return l;
    }
    else {
        l->error = 3;
        return l;
    }
}

static Context* naiveGauss(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // (
    if(e->index != 4) {
        Context *ret = (Context*) malloc(sizeof(Context));
        ret->error = 2;
        ret->type = 0;
        return ret;
    }
    Context *l = statement(myScan, myParse, parse); // arg 1
    if(l->error > 0) return l;
    if(l->type!=1) {
        l->error = 9;
        return l;
    }
    e = (Entry*) scanner_next(myScan);
    Matrix *ng;
    if(e->index==14) {
        e = (Entry*) scanner_peek(myScan); // ? NULL ?
        if(e->index ==0) {
            if(strcmp(e->value.var->ID, "NULL")==0) {
                e = (Entry*) scanner_next(myScan); // consume
                e = (Entry*) scanner_next(myScan); // )
                if(e->index==5) {
                    Matrix *sol = LAalgorithms_naiveGaussElim(l->value.mat, NULL);
                    if(sol==NULL) {
                        l->error = 25;
                        return l;
                    }
                    matrix_destructor(&l->value.mat);
                    l->value.mat = sol;
                    return l;
                }
                else {
                    l->error = 3;
                    return l;
                }
            }
        }
        Context *r = statement(myScan, myParse, parse); // arg 2
        e = (Entry*) scanner_next(myScan);
        if(r->error >0) {
            context_destructor(&l);
            return r;
        }
        if(r->type!=1) {
            context_destructor(&l);
            r->error = 9;
            return r;
        }
        if(e->index==5) {
            ng = LAalgorithms_naiveGaussElim(l->value.mat, r->value.mat);
            if(ng==NULL) {
                context_destructor(&l);
                r->error = 25;
                return r;
            }
            matrix_destructor(&r->value.mat);
            free(r);
            matrix_destructor(&l->value.mat);
            l->value.mat = ng;
            return l;    
        }
        else {
            context_destructor(&l);
            r->error = 3;
            return r;
        }
    }
    else {
        l->error = 6;
        return l;
    }
}

static Context* function(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan);
    Context *l;
    if(e->index==15) { // function
        int index = (int) e->value.number;
        parser_LUReturn *myLU;
        parser_EigenReturn *myE;
        parser_QRReturn *myQR;
        switch(index) {
            case 0:
                return naiveGauss(myScan, myParse, parse);
            break;
            case 1:
                return transpose(myScan, myParse, parse);
            break;
            case 2:
                return houseHolder(myScan, myParse, parse);
            break;
            case 3:
                myLU = LU(myScan, myParse, parse);
            break;
            case 4:
                myQR = QR(myScan, myParse, parse);
            break;
            case 5: 
                myE = eigen(myScan, myParse, parse);
            break;
            case 6: // integrate
                return integrate(myScan, myParse, parse);
            break;
            case 7: // gamma
                return gammaValue(myScan, myParse, parse);
            break;
            case 8: 
                return qchisq(myScan, myParse, parse);
            break;
            case 9:
                return pchisq(myScan, myParse, parse);
            break;
            case 10:
                // newton
                return newton(myScan, myParse, parse);
            break;
            case 11: 
                // bisection
                return bisection(myScan, myParse, parse);
            break;
            case 12: // getRow
                return getRow(myScan, myParse, parse);
            break;
            case 13:// getCol
                return getCol(myScan, myParse, parse);
            break;
            case 14: // cov
                return cov(myScan, myParse, parse);
            break;
            case 15: // cor
                return cor(myScan, myParse, parse);
            break;
            case 16: // zscore
                return zscore(myScan, myParse, parse);
            break;
            case 17: // mean
                return mean(myScan, myParse, parse);
            break;
            case 18: // backSub
                return backSub(myScan, myParse, parse);
            break;
            case 19: // forwSub
                return forwSub(myScan, myParse, parse);
            break;
            case 20: // qnorm
                return qnorm(myScan, myParse, parse);
            break;
            case 21: // pnorm
                return pnorm(myScan, myParse, parse);
            break;
        }
        l = (Context*) malloc(sizeof(Context));
        l->error = 0;
        l->type = 0;
        l->isNull=0;
        switch(index) {
            case 3: // LU
                if(myLU->error>0) {
                    l->error = myLU->error;
                    free(myLU);
                    return l;
                }
            // 0 for complete LU, 1 for L, 2 for U, 3 for solved 
                if(myLU->type == 0 ) {
                    l->type = 3;
                    l->value.lu = myLU->lu;
                }
                else if(myLU->type==1) {
                    l->type = 1;
                    l->value.mat = lu_getL(myLU->lu);
                    lu_destructor(&myLU->lu);
                }
                else if(myLU->type ==2) {
                    l->type = 1;
                    l->value.mat = lu_getU(myLU->lu);
                    lu_destructor(&myLU->lu);
                }
                else if(myLU->type==3) {
                    l->type = 1;
                    l->value.mat = lu_getSol(myLU->lu);
                    lu_destructor(&myLU->lu);
                }
                free(myLU);
            break;
            case 4: // QR
                if(myQR->error>0) {
                    l->error = myQR->error;
                    free(myQR);
                    return l;
                }
                if(myQR->type == 0 ) {
                    l->type = 4;
                    l->value.qr = myQR->qr;
                }
                else if(myQR->type==1) {
                    l->type = 1;
                    l->value.mat = grahmschmidt_getQ(myQR->qr);
                    grahmschmidt_destructor(&myQR->qr);
                }
                else if(myQR->type ==2) {
                    l->type = 1;
                    l->value.mat = grahmschmidt_getR(myQR->qr);
                    grahmschmidt_destructor(&myQR->qr);
                }
                else if(myQR->type==3) {
                    l->type = 1;
                    l->value.mat = grahmschmidt_getSol(myQR->qr);
                    grahmschmidt_destructor(&myQR->qr);
                }
                free(myQR);
            break;
            case 5: //eigen
                if(myE->error>0) {
                    l->error = myE->error;
                    free(myE);
                    return l;
                }
                if(myE->type == 0 ) {
                    l->type=5;
                    l->value.e = myE->e;
                }
                else if(myE->type==1) {
                    l->type = 1;
                    l->value.mat = eigen_getValue(myE->e);
                    eigen_destructor(&myE->e);
                }
                else if(myE->type ==2) {
                    l->type = 1;
                    l->value.mat = eigen_getVector(myE->e);
                    eigen_destructor(&myE->e);
                }
                free(myE);
            break;
        }
        return l;
    }
    else ; // error
}

static Context *Reserved(Scanner *myScan, Parser *myParse, int parse) {
     Entry *e = (Entry*) scanner_next(myScan);
     Entry *s = (Entry*) scanner_next(myScan);
     if(s->index ==4) {
        int index = (int) e->value.number;
        Context *v = statement(myScan, myParse,  parse);
        if(v->error>0) return v;
        s = (Entry*) scanner_next(myScan);
        if(!parse) {
            if(v->type==1)  {
                switch(index) {
                    case 0:
                        for(int i = 0; i < matrix_nrow(v->value.mat); i++) {
                            for(int j = 0; j < matrix_ncol(v->value.mat); j++) {
                                matrix_set(v->value.mat, i, j, cos(matrix_get(v->value.mat, i, j)));
                            }
                        }
                        break;
                    case 1:
                        for(int i = 0; i < matrix_nrow(v->value.mat); i++) {
                            for(int j = 0; j < matrix_ncol(v->value.mat); j++) {
                                matrix_set(v->value.mat, i, j, sin(matrix_get(v->value.mat, i, j)));
                            }
                        }
                        break;
                    case 2:
                        for(int i = 0; i < matrix_nrow(v->value.mat); i++) {
                            for(int j = 0; j < matrix_ncol(v->value.mat); j++) {
                                matrix_set(v->value.mat, i, j, tan(matrix_get(v->value.mat, i, j)));
                            }
                        }
                        break;
                    case 3:
                        for(int i = 0; i < matrix_nrow(v->value.mat); i++) {
                            for(int j = 0; j < matrix_ncol(v->value.mat); j++) {
                                matrix_set(v->value.mat, i, j, acos(matrix_get(v->value.mat, i, j)));
                            }
                        }
                        break;
                    case 4:
                        for(int i = 0; i < matrix_nrow(v->value.mat); i++) {
                            for(int j = 0; j < matrix_ncol(v->value.mat); j++) {
                                matrix_set(v->value.mat, i, j, asin(matrix_get(v->value.mat, i, j)));
                            }
                        }
                        break;
                    case 5:
                        for(int i = 0; i < matrix_nrow(v->value.mat); i++) {
                            for(int j = 0; j < matrix_ncol(v->value.mat); j++) {
                                matrix_set(v->value.mat, i, j, atan(matrix_get(v->value.mat, i, j)));
                            }
                        }
                        break;
                    case 6:
                        for(int i = 0; i < matrix_nrow(v->value.mat); i++) {
                            for(int j = 0; j < matrix_ncol(v->value.mat); j++) {
                                matrix_set(v->value.mat, i, j, log10(matrix_get(v->value.mat, i, j)));
                            }
                        }
                        break;
                    case 7:
                        for(int i = 0; i < matrix_nrow(v->value.mat); i++) {
                            for(int j = 0; j < matrix_ncol(v->value.mat); j++) {
                                if(matrix_get(v->value.mat, i, j)==0) {
                                    v->error = 43;
                                    return v;
                                }
                                matrix_set(v->value.mat, i, j, log(matrix_get(v->value.mat, i, j)));
                            }
                        }
                        break;
                }
            }
            else if(v->type==0) {
                switch(index) {
                case 0:
                    v->value.number = cos(v->value.number);
                    break;
                case 1:
                    v->value.number = sin(v->value.number);
                    break;
                case 2:
                    v->value.number = tan(v->value.number);
                    break;
                case 3:
                    v->value.number = acos(v->value.number);
                    break;
                case 4:
                    v->value.number = asin(v->value.number);
                    break;
                case 5:
                    v->value.number = atan(v->value.number);
                    break;
                case 6:
                    v->value.number = log10(v->value.number);
                    break;
                case 7:
                    if(v->value.number==0) {
                        v->error = 43;
                        return v;
                    }
                    v->value.number = log(v->value.number); // ln()
                    break;    
                default:
                    //error =1;
                    printf("Error in operation?\n");
                    break;
                }
            }
            else {
                v->error = 8;
                return v;
            }
        }
        if(s->index==5) {
            if(parse) return NULL;
            return v; // RETURNING V HERE
        }
        else {
            v->error = 3;
            return v;
        }
     }
}

static void performOperation(Context *l, Context *r, int parse) {
    if(parse) return;
    if(r==NULL) return;
    else if(r->isNull) free(r);
    else {
        if(l->type==0 && r->type==0) {
            switch (r->operand)
            {
            case '+':
                l->value.number = l->value.number + r->value.number;  
                break;
            case '-':
                l->value.number = l->value.number-r->value.number;  
                break;
            case '*':
                l->value.number = l->value.number * r->value.number;  
                break;
            case '/':
                if(r->value.number==0) {
                    context_destructor(&r);
                    l->error = 42;
                    return;
                }
                l->value.number = l->value.number / r->value.number;  
                break;
            case '^':
                l->value.number = pow(l->value.number, r->value.number);  
                break;
            default:
                printf("Error in operation pt 2?\n");
                //error =1; // should never happen as these are all the operands allowed
                break;
            }
        }
        else if(l->type==1 && r->type==1) {
            Matrix *temp;
            switch(r->operand) 
            {
            case '+':
                temp = matrix_addMatrices(l->value.mat, r->value.mat);
                if(temp==NULL) {
                    context_destructor(&r);
                    l->error = 26;
                    return;
                }
                matrix_destructor(&l->value.mat);
                matrix_destructor(&r->value.mat);
                l->value.mat = temp;
                if(!temp) ; // error 
                break;
            case '-':
                temp = matrix_minusMatrices(l->value.mat, r->value.mat);
                if(temp==NULL) {
                    context_destructor(&r);
                    l->error = 27;
                    return;
                }
                matrix_destructor(&l->value.mat);
                matrix_destructor(&r->value.mat);
                l->value.mat = temp;
                if(!temp) ; // error 
                break;
            case '*':
                temp = matrix_multiplyMatrices(l->value.mat, r->value.mat);
                if(temp==NULL) {
                    context_destructor(&r);
                    l->error = 28;
                    return;
                }
                matrix_destructor(&l->value.mat);
                matrix_destructor(&r->value.mat);
                l->value.mat = temp;
                if(!temp) ; // error 
                break;
            case '/':
                // error cannot divide matrices 
                context_destructor(&r);
                l->error = 29;
                return;
                break;
            case '^':
                 // error cannot exponent matrices 
                context_destructor(&r);
                l->error = 29;
                return;
                break;
            default:
            printf("Error in operation pt 3?\n");
                //error =1;
                break;
            }
           // matrix_destructor(&r->value.mat);
        }
        else if(l->type==1 && r->type==0) {
            switch(r->operand) 
            {
            case '+':
                matrix_addConstant(l->value.mat, r->value.number, 1);
                break;
            case '-':
                matrix_addConstant(l->value.mat, -r->value.number, 1);
                break;
            case '*':
                matrix_multiplyConstant(l->value.mat, r->value.number, 1);
                break;
            case '/':
                if(r->value.number==0) {
                    context_destructor(&r);
                    l->error = 42;
                    return;
                }
                matrix_multiplyConstant(l->value.mat, 1/r->value.number, 1);
                break;
            case '^':
                for(int i = 0; i < matrix_nrow(l->value.mat); i++) {
                    for(int j = 0; j < matrix_ncol(l->value.mat); j++) {
                        matrix_set(l->value.mat, i, j, pow(matrix_get(l->value.mat, i, j), r->value.number));
                    }
                }
                break;
            default:
            printf("Error in operation pt 4?\n");
              //  error =1;
                break;
            }
        }
        else if(l->type==0 && r->type ==1) {
            switch(r->operand) 
            {
            case '+':
                matrix_addConstant(r->value.mat, l->value.number, 1);
                l->value.mat = r->value.mat;
                l->type = 1;
                break;
            case '-':
               // error: cannot 3 - matrix...
                context_destructor(&r);
                l->error = 30;
                return;
                break;
            case '*':
                matrix_multiplyConstant(r->value.mat, l->value.number, 1);
                l->value.mat = r->value.mat;
                l->type = 1;
                break;
            case '/':
                // error cannot divide matrices 
                context_destructor(&r);
                l->error = 31;
                return;
                break;
            case '^':
                 // error cannot exponent matrices 
                context_destructor(&r);
                l->error = 32;
                return;
                break;
            default:
            printf("Error in operation pt 5?\n");
               // error =1;
                break;
            }
        }
        else ;
        free(r);
    }
}

// start here tomorrow

static Context * T3(Scanner *myScan, Parser *myParse, Context *p, int parse) {
    Entry *e = (Entry*) scanner_peek(myScan);
    if(e->index==8) {
        Entry *s = (Entry*) scanner_next(myScan); // consume symbol
        char operand = e->value.operand;
        Context *l = factor(myScan, myParse, parse);
        if(l->error>0) {
            context_destructor(&p);
            return l;
        }
        if(!parse) l->operand = operand;
        Context *r = T3(myScan, myParse, l, parse); 
        if(r->error>0) {
            context_destructor(&p);
            return r;
        }
        performOperation(p, r,parse);  // NOTE: ^ is right associative 
        return p;
    }
    else {
        return p; // epsilon
    } 
}

static Context* temp(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_peek(myScan);
    if(e->index==0 || e->index==10 || e->index==1 ||e->index==9 ||e->index==4 || e->index==11||e->index==15) {
        Context *l = factor(myScan, myParse, parse);
        if(l->error>0) return l;
        Context *r = T3(myScan, myParse, l, parse);
        if(r->error>0) return r;
        return r;
    }
    else {
        Context *l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->error = 1;
        return l;
    }
}

static Context* statement(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_peek(myScan);
    if(e->index==0 || e->index==1 ||e->index==9 ||e->index==4 || e->index==10| e->index==11||e->index==15) {
       
        Context *l = term(myScan, myParse,parse);
        if(l->error>0) return l;
        Context *r = T1(myScan, myParse,l, parse);
        if(r->error>0) return r;
        return r;
    }
    else {
        Context *l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->error = 1;
        return l;
    }
}

static Context* T_1(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_peek(myScan);
    if(e->index == 0 || e->index == 1) {
        Entry *s = (Entry*) scanner_next(myScan);
      
        if(parse) return NULL;
        Context *v = (Context*) malloc(sizeof(Context));
        v->isNull = 0;
        v->error = 0;
        v->operand = '\0';
        if(e->index==0) {
            Variable *temp;
            temp = avltree_find(myParse->variables, s->value.var);
            if(temp->type==0) {
                v->type = 0;
                v->value.number = -temp->value.number; // we want previous value
            }
            else if(temp->type==1) {
                v->type = 1;
                matrix_initializeFromMatrix(&v->value.mat, temp->value.mat);
                matrix_multiplyConstant(v->value.mat, -1, 1);
            }
            else {
                v->type = 0;
                v->error = 34;
                return v;
            }
        }
        else if(e->index==1) {
            v->type = 0;
            v->value.number = - e->value.number; // we want previous value
        }
        else {
            v->type = 0;
            v->error = 35;
            return v;
        }
        return v;
    }
    else if(e->index == 4 || e->index==11) {
        e = (Entry*) scanner_next(myScan);
        Context *v = statement(myScan, myParse,parse);
        if(v->error > 0) return v;
        e = (Entry*) scanner_next(myScan);
        if(e->index==5) {
            if(parse) return NULL;
            if(v->type==0) v->value.number = - v->value.number; // negate
            else if(v->type==1) matrix_multiplyConstant(v->value.mat, -1, 1);
            return v; 
        }
        else {
            v->error = 3;
            return v;
        }
    }
    else if(e->index==10) {
        Context *v = Reserved(myScan, myParse,parse);
        if(v->error > 0) return v;
        if(parse) return NULL;
        if(v->type==0) v->value.number = - v->value.number; // negate
        else if(v->type==1) matrix_multiplyConstant(v->value.mat, -1, 1);
        else {
            v->error = 34;
            return v;
        }
        return v;
    }
    else {
        Context *v = (Context*) malloc(sizeof(Context));
        v->error=1;
        v->type = 0;
        return v;
    }
}

// [ [ 2 , 3 , 3, 3 ] , [ 3 , 3, 3, 3] ]

// rowStart = [ [ entryList ] | [ [ entryList COMMA [ row 
// row = entryList COMMA [ row | entryList ]
// entryList = NUMBER COMMA entryList | NUMBER ]
// NUMBER -> statement

struct Row {
    double *row;
    int maxSize;
    int index;
};
typedef struct Row Row;

struct RowArray {
    Row **rows;
    int maxSize;
    int index;
};
typedef struct RowArray RowArray;

static Context* entryList(Scanner *myScan, Parser *myParse, int parse, Row *row) {
    Context *l = statement(myScan, myParse, parse);
    if(l->error>0) {
        free(row->row);
        free(row);
        return l;
    }
    if(l->type != 0) {
        free(row->row);
        free(row);
        l->error = 8;
        return l;
    }
    Entry *s = (Entry*) scanner_next(myScan); // ?
    if(sizeof(row) / sizeof(double) >= row->maxSize) {
        row->maxSize *= 1.5;
        row->row = realloc(row->row, sizeof(double) * row->maxSize);
    }
    if(l->type!=0) ; // error
    row->row[row->index] = l->value.number;
    
    row->index++;
    if(s->index==14) { // Comma
        context_destructor(&l);
        return entryList(myScan, myParse, parse, row);
    }
    else if(s->index==12) { // ]
        return l;
    }
    else {
        free(row->row);
        free(row);
        l->error = 5;
        return l;
    }
}

static Context* rowParse(Scanner *myScan, Parser *myParse, int parse, RowArray *rows) {
    Entry *e = (Entry*) scanner_peek(myScan); // Num
    if(e->index==0 || e->index==1 ||e->index==9 ||e->index==4 || e->index==10) { // Number
        Row *row = (Row*) malloc(sizeof(Row));
        row->maxSize = 20;
        row->index = 0;
        row->row = malloc(sizeof(double)*row->maxSize);
        Context *l = entryList(myScan, myParse, parse, row);
        if(l->error > 0) return l;
        // store
        if(sizeof(rows) / sizeof(Row) >= rows->maxSize) {
            rows->maxSize *= 1.5;
            rows->rows = realloc(rows->rows, sizeof(Row)*row->maxSize);
        }
        rows->rows[rows->index] = row;
        rows->index++;

        e = (Entry*) scanner_next(myScan); // ?
        if(e->index==14) { // comma
            context_destructor(&l);
            e = (Entry*) scanner_next(myScan); // [
            if(e->index==11) {
                e = (Entry*) scanner_peek(myScan); // Num
                if(e->index==0 || e->index==1 ||e->index==9 ||e->index==4 || e->index==10) return rowParse(myScan, myParse, parse, rows);
                else {
                    Context *l = (Context*) malloc(sizeof(Context));
                    l->type = 0;
                    l->error = 1;
                    return l;
                }
            }
            else {
                Context *l = (Context*) malloc(sizeof(Context));
                l->type = 0;
                l->error = 4;
                return l;
            }
        }
        else if (e->index==0 || e->index==1 ||e->index==9 ||e->index==4 || e->index==10) { // num
            Row *row2 = (Row*) malloc(sizeof(Row));
            context_destructor(&l);
            row2->maxSize = 20;
            row2->index = 0;
            row2->row = malloc(sizeof(double)*row2->maxSize);
            l = entryList(myScan, myParse, parse, row2);
            if(l->error > 0) return l;
            // store
            if(sizeof(rows) / sizeof(Row) >= rows->maxSize) {
                rows->maxSize *= 1.5;
                rows->rows = realloc(rows->rows, sizeof(Row)*row->maxSize);
            }
            rows->rows[rows->index] = row2;
            rows->index++;
            return l;
        }
        else if(e->index==12) return l;
        else {
            l->error = 1;
            return l;
        }
    }
    else {
        Context *l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->error = 1;
        return l;
    }
}

static Context* rowStart(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // [
    if(e->index==11) {
        e = (Entry*) scanner_next(myScan); // [
        if(e->index==11) {
            e = (Entry*) scanner_peek(myScan); // peek NUM
            if(e->index==0 || e->index==1 ||e->index==9 ||e->index==4 || e->index==10) {
                Row *row = (Row*) malloc(sizeof(Row));
                row->maxSize = 20;
                row->index = 0;
                row->row = malloc(sizeof(double)*row->maxSize);
                Context *l = entryList(myScan, myParse, parse, row);
                if(l->error > 0) {
                    return l;
                }
                e = (Entry*) scanner_next(myScan); //  ?
                Matrix *mat;
                if(e->index==12){ // ]
                    matrix_initialize(&mat, 1, row->index, 0);
                    for(int i = 0; i < row->index; i++) {
                        matrix_set(mat, 0, i, row->row[i]);
                    }
                    free(row->row);
                    free(row);
                    l->type = 1;
                    l->value.mat = mat;
                    return l;
                }
                else if(e->index==14) { // Comma
                    e = (Entry*) scanner_next(myScan); //  [ 
                    if(e->index==11) {
                        e = (Entry*) scanner_peek(myScan); //  num
                        if(e->index==0 || e->index==1 ||e->index==9 ||e->index==4 || e->index==10) {
                            RowArray *myArray = (RowArray*) malloc(sizeof(RowArray));
                            myArray->maxSize = 5;
                            myArray->index = 1;
                            myArray->rows = (Row**) malloc(sizeof(Row*)*20);
                            myArray->rows[0] = row;
                            context_destructor(&l);
                            l = rowParse(myScan, myParse, parse, myArray);
                            if(l->error > 0) {
                                for(int i = 0; i < myArray->index; i++) {
                                    free(myArray->rows[i]->row);
                                    free(myArray->rows[i]);
                                }
                                free(myArray->rows);
                                free(myArray);
                                return l;
                            }
                            int nrow = myArray->index;
                            int ncol = 0;
                            for(int i = 0; i < nrow; i++) {
                                if(i==0) ncol = myArray->rows[i]->index;
                                else if(myArray->rows[i]->index != ncol) {
                                    for(int j = 0; j < nrow; j++) {
                                        free(myArray->rows[j]->row);
                                        free(myArray->rows[j]);
                                    }
                                    free(myArray->rows);
                                    free(myArray);
                                    l->error = 40;
                                    return l;
                                }
                            }
                            matrix_initialize(&mat, nrow, ncol, 0);
                            for(int i = 0; i < nrow; i++) {
                                for(int j = 0; j < ncol; j++) {
                                    matrix_set(mat, i, j, myArray->rows[i]->row[j]);
                                }
                                free(myArray->rows[i]->row);
                                free(myArray->rows[i]);
                            }
                            free(myArray->rows);
                            free(myArray);
                            l->type = 1;
                            l->value.mat = mat;
                            return l;
                        }
                        else {
                            free(row->row);
                            free(row);
                            l->error = 1;
                            return l;
                        }
                    }
                    else {
                        free(row->row);
                        free(row);
                        l->error = 4;
                        return l;
                    }
                }
                else {
                    free(row->row);
                    free(row);
                    l->type = 0;
                    l->error = 39;
                    return l;
                }
            }
            else {
                Context *l = (Context*) malloc(sizeof(Context));
                l->type = 0;
                l->error = 1;
                return l;
            }
        }
        else {
            Context *l = (Context*) malloc(sizeof(Context));
            l->type = 0;
            l->error = 4;
            return l;
        }
    }
    else {
        Context *l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->error = 4;
        return l;
    }
    return NULL;
}

static Context* matrixIndex(Scanner *myScan, Parser *myParse, int parse) {
    Entry *e = (Entry*) scanner_next(myScan); // [
    if(e->index==11) {
        e = (Entry*) scanner_next(myScan); // NUM
        if(e->index == 1) {
            int row = e->value.number;
            if(row != e->value.number) { // not an int
                Context *v = (Context*) malloc(sizeof(Context));
                v->type = 0;
                v->error =  38;
                return v;
            }
            e = (Entry*) scanner_next(myScan); // Comma
            if(e->index==14) {
                e = (Entry*) scanner_next(myScan); // NUM
                if(e->index==1) {
                    int col = e->value.number;
                    if(col != e->value.number) { // not an int
                        Context *v = (Context*) malloc(sizeof(Context));
                        v->type = 0;
                        v->error =  38;
                        return v;
                    }
                    e = (Entry*) scanner_next(myScan); // ]
                    if(e->index==12) {
                        Context *v = (Context*) malloc(sizeof(Context));
                        v->type = 0;
                        v->error =  0;
                        v->value.ind = (double*) malloc(sizeof(double)*2);
                        v->value.ind[0] = row;
                        v->value.ind[1] = col;
                        return v;
                    }
                    else {
                        Context *v = (Context*) malloc(sizeof(Context));
                        v->type = 0;
                        v->error =  5;
                        return v;
                    }
                }
            }
            else {
                Context *v = (Context*) malloc(sizeof(Context));
                v->type = 0;
                v->error =  6;
                return v;
            }
        }
        else {
            Context *v = (Context*) malloc(sizeof(Context));
            v->type = 0;
            v->error =  38;
            return v;
        }
    }
    else {
        Context *v = (Context*) malloc(sizeof(Context));
        v->type = 0;
        v->error =  4;
        return v;
    }
}

static Context* matrix(Scanner *myScan, Parser *myParse,int parse) {
    // example input: [[3, 4, 5]] ; [[3]] ; [[3, 3], [3, 4]] ; 
    // reject input: [[]] ; [[3, 4], [4]]
    Entry *e = (Entry*) scanner_peek(myScan); // '['
    if(e->index==11) {
        Context *l = rowStart(myScan, myParse, parse);
        if(l->error > 0) return l;
        return l;
    }
    else {
        Context *v = (Context*) malloc(sizeof(Context));
        v->type = 0;
        v->error =  4;
        return v;
    }
}

static Context* factor(Scanner *myScan, Parser *myParse,int parse) {
    Entry *e = (Entry*) scanner_peek(myScan);
    if(e->index==15) {
        return function(myScan, myParse, parse);
    }
    if(e->index==11) {
        return matrix(myScan, myParse, parse);
    }
    else if(e->index==9) {
        e = (Entry*) scanner_next(myScan);       
        return T_1(myScan, myParse, parse);
    }
    else if(e->index==4) {
        e = (Entry*) scanner_next(myScan);
        Context *v = statement(myScan, myParse,parse);
        if(v->error >0) return v;
        e = (Entry*) scanner_next(myScan);
        if(e->index==5) {
            return v; 
        }
        else {
            v->error = 3;
            return v;
        }
    }
    else if(e->index == 0 || e->index == 1) {
        Entry *s = (Entry*) scanner_next(myScan);
        e = (Entry*) scanner_peek(myScan);
        int matrixInd = 0;
        Context *ind;
        if(s->index == 0 && e->index == 11) { // getting row index
            matrixInd = 1;
            ind = matrixIndex(myScan, myParse, parse);
            if(ind->error > 0) return ind;
        }
        if(parse) return NULL;
        Context *v = (Context*) malloc(sizeof(Context));
        v->error = 0;
        v->isNull = 0;
        v->operand = '\0';
        v->type = 0;
        if(s->index==0) {
            Variable *temp;
            temp = avltree_find(myParse->variables, s->value.var);
            if(temp==NULL) {
                v->error = 37;
                v->value.var = strdup(s->value.var->ID);
                if(matrixInd) context_destructor(&ind);
                return v;
            }
            if(matrixInd) {
                if(temp->type==1) {
                    int row = ind->value.ind[0];
                    int col = ind->value.ind[1];
                    free(ind->value.ind);
                    if(matrix_nrow(temp->value.mat) < row || matrix_ncol(temp->value.mat) < col) {
                        v->error = 36; 
                        context_destructor(&ind);
                        return v;
                    }
                    else {
                        v->value.number = matrix_get(temp->value.mat, row, col);
                        free(ind);
                    }
                }
                else {
                    context_destructor(&ind);
                    v->error = 9;
                    return v;
                }
            }
            else if(temp->type==0) {
                v->type = 0;
                v->value.number = temp->value.number; // we want previous value
            }
            else if(temp->type==1) {
                v->type = 1;
                matrix_initializeFromMatrix(&v->value.mat, temp->value.mat);
            }
            else {
                v->error = 35; 
                return v;
            }
        }
        else if(s->index==1){
            v->type = 0;
            v->value.number = s->value.number; // we want previous value
        }
        else {
            v->error = 35;
            return v;
        }
        
        return v;
    }
    else if(e->index==10) {
        return Reserved(myScan, myParse,parse);
    }
    else if(e->index==11) {
        return matrix(myScan, myParse, parse);
    }
}

static Context* T2(Scanner *myScan, Parser *myParse, Context *p, int parse) {
    Entry *e = (Entry*) scanner_peek(myScan);
    if(e->index==6) {
       Entry *s = (Entry*) scanner_next(myScan);
       
        char operand = e->value.operand;
        Context *l = temp(myScan, myParse, parse);
        if(l->error > 0) {
            context_destructor(&p);
            return l;
        }
        if(!parse) l->operand = operand;
        performOperation(p, l, parse);
        if(p->error > 0) return p;
        Context *r = T2(myScan, myParse, p, parse);
        if(r->error > 0) return r;
        return r;
    }
    else {
        return p; // epsilon
    }
}
static Context* term(Scanner *myScan, Parser *myParse,int parse) {
    Entry *e = (Entry*) scanner_peek(myScan);
    if(e->index==0 || e->index==1 ||e->index==9 || e->index==4 || e->index==10|| e->index==11||e->index==15) {
       
        Context *l = temp(myScan, myParse, parse);
        if(l->error>0) return l;
        Context *r = T2(myScan, myParse, l, parse);
        if(r->error>0) return r;
        return r;
    }
    else {
        Context *l = (Context*) malloc(sizeof(Context));
        l->type = 0;
        l->error = 1;
        return l;
    }
}

static Context* T1(Scanner *myScan, Parser *myParse, Context *p, int parse) {
    Entry *e = (Entry*) scanner_peek(myScan);
    if(e->index==2 || e->index==3) { // OP11 or // OP12
        Entry *s = (Entry*) scanner_next(myScan); // consume symbol
       
        char operand = e->value.operand;
        Context *l = term(myScan, myParse, parse);
        if(l->error>0) {
            context_destructor(&p);
            return l;
        }
        if(!parse) l->operand = operand;
        performOperation(p, l, parse);
        if(p->error > 0) return p;
        Context *r = T1(myScan, myParse, p, parse);
        if(r->error > 0) return r;
        return r;
    }
    else{
        return p; // epsilon
    } 
}

// Is the passed in variable contained within the tree
static int containID(Parser *myParse, Entry *assignVar) {
    return avltree_contains(myParse->variables, assignVar->value.var);
}

static ParseReturn* start(Scanner *myScan, Parser *myParse, int parse) {
    double sum = 0;
    Entry *e = (Entry*) scanner_peek(myScan);
    Context* l;
    ParseReturn *ret = (ParseReturn*) malloc(sizeof(ParseReturn));
    ret->val = NULL;
    if(e->index==16) {
        Context *myBool = staticFunction(myScan, myParse, parse);
        ret->type = 2;
        ret->error = myBool->error;
        if(myBool->error ==0) ret->value = myBool->value.number;
        context_destructor(&myBool);
        return ret;
    }
    if(scanner_getSize(myScan) > 1 && e->index==0) {
        Entry *s = (Entry*) scanner_getAtIndex(myScan, 1);
        if(s->index==7) {
            if(parse==0) {
                if(containID(myParse, e)) {
                    ;
                }
                else {
                    ;
                }
            }
            Entry *t1 = (Entry*) scanner_next(myScan); // used to skip first two entries
            Entry *t2 = (Entry*) scanner_next(myScan); // used to skip first two entries
            ret->type = -1;
            l=statement(myScan, myParse, parse);
            if(l->error > 0) {
                ret->error = l->error;
                context_destructor(&l);
                ret->type = 0;
                ret->value = -1;
                return ret;
            }
            if(!parse) {
                Matrix *temp;
                Variable *var = (Variable *) malloc(sizeof(Variable));
                var->ID = strdup(e->value.var->ID);
                if(l->type==0) {
                    sum = l->value.number;
                    var->value.number = sum;
                    var->type = 0;
                }
                else if(l->type==1) {
                    temp = l->value.mat; 
                    var->value.mat = temp;
                    var->type = 1;
                } 
              //  printf("Var type: %d\n", var->type);
                if(containID(myParse, e)) { // x = sum where x is already in our variable stack so we need to replace value
                    avltree_replace(myParse->variables, var);
                }
                else { // x = sum where x is no in our variable stack so we need to add x
                    avltree_add(myParse->variables, var);
                }
              //  printf("l type: %d\n", l->type);
                free(l);
            }
        }
        else goto Execute; // just a few lines below
    }
    else if(e->index==0 ||e->index==10 || e->index==1 || e->index==9 || e->index==4 || e->index==11||e->index==15) { 
        Execute:
		if(scanner_getSize(myScan) > 1 && e->index==1) {
			Entry *s = (Entry*) scanner_getAtIndex(myScan, 1);
			if(s->index==7) {
				if(e->value.number == 2.7182818284590452353602874713527) {
					ret->error = 44;
					ret->type = 0;
					return ret;
				}
			}
		}      
        l = statement(myScan, myParse, parse);
       // printf("l->error: %d\n", l->error);
        if(l->error > 0) {
            ret->error = l->error;
            if(l->error ==37) {
                ret->varName = l->value.var;
                ret->type = -2;
                context_destructor(&l);
                return ret;
            }
            context_destructor(&l);
            ret->type = 0;
            ret->value = -1;
            return ret;
        }
        if(!parse) {
            if(l->type == 0)  {
                ret->type=0;
                ret->value = l->value.number;
                ret->val = NULL;
            }
            else if(l->type==1) {
                ret->type=1;
                ret->val = l->value.mat;
                ret->value = -1;
            }
            else if(l->type==2) {
                // bool, shouldn't happen
            }
            else if(l->type==3) {
                // LU
                ret->type = 3;
                ret->lu = l->value.lu;
                ret->val=NULL;
            }
            else if(l->type==4) {
                // QR
                ret->type = 4;
                ret->qr = l->value.qr;
                ret->val=NULL;
            }
            else if(l->type==5) {
                // Eigen
                ret->type = 5;
                ret->e = l->value.e;
                ret->val=NULL;
            }
            free(l);
        }
    }
    else if(e->index==-1) ;
    else {
        ret->error = 1;
        ret->type = 0;
        return ret;
    }
    ret->error = 0;
    Entry *last = (Entry*) scanner_peek(myScan);
 //   printf("Type: %d\n", ret->type);
  //  printf("Last Error: %d\n", ret->error);
    if(last->index != - 1) {
       // printf("Here");
        ret->error = 1;
        ret->type = 0;
        return ret;
    }
  //  printf("Last Error: %d\n", ret->error);
    return ret;
}


// Callable functions:

void parser_destructor(Parser **myParse) {
    avltree_destructor(&(*myParse)->variables);
    free(*myParse);
}

void parser_initialize(Parser **myParse) {
    (*myParse) = (Parser*) malloc(sizeof(Parser));
    avltree_initialize(&((*myParse)->variables), compareAVL, freeValueAVL, 0);
}

// Replaces all the tokens in scanner with variables from stored variables
// Returns NULL if succesful
// Returns string if an unrecognized variable is found; also calls destructor and re-initializes scanner
char* parser_checkVariables(Scanner **myScan, Parser *myParser) {
    char *unrecognizedVar = updateScanID(*myScan, myParser);
    if(unrecognizedVar!=NULL) {
       // printf("Unrecognized variable: %s\n", unrecognizedVar);
        scanner_destructor((myScan)); // delete stored values
        scanner_initialize((myScan)); // re-initialize scanner
        return unrecognizedVar;
    }
    return NULL;
}

// Parses the tokens for semantic errors
// returns 1 if accept; 0 for reject; and -2 for missing ')' par
int parser_parse(Scanner **myScan, Parser *myParser) {
    ParseReturn* acc = start(*myScan, myParser, 1);
    scanner_resetEnumerator(*myScan);
    int ret = acc->error;
    parseReturn_destructor(&acc);
    if(ret!=0) return 1;
    else {
        scanner_destructor((myScan)); // delete stored values
        scanner_initialize((myScan)); // re-initialize scanner
        return ret;
    }
}

// Usually called after parser_parse to check for semantic errors;
// Solves the equation and returns value
ParseReturn* parser_solve(Scanner **myScan, Parser *myParser) {
    return start(*myScan, myParser, 0);
}

// Destructor for return struct from Parser
void parseReturn_destructor(ParseReturn** ret) {
    switch((*ret)->type) {
        case -2: // unrec var name
        free((*ret)->varName);
        break;
        case -1: // assignment
        break;
        case 0: // number
        break;
        case 1:
        matrix_destructor(&(*ret)->val); 
        break;
        case 2: // bool
        break;
        case 3: // lu
        lu_destructor(&(*ret)->lu);
        break;
        case 4:
        grahmschmidt_destructor(&(*ret)->qr);
        break;
        case 5:
        eigen_destructor(&(*ret)->e);
        break;
    }
    free(*ret);
}

// Returns the Error value from parsr
int parseReturn_getError(ParseReturn *ret) {
    return ret->error;
}

// Returns the double value from parsr
double parseReturn_getValue(ParseReturn *ret) {
    return ret->value;
}

// returns the matrix L from LU decomposition; 
// will need to destruct later
Matrix* parseReturn_luGetL(ParseReturn *ret) {
    if(ret->type!=3) return NULL;
    return lu_getL(ret->lu);
}

// returns the matrix U from LU decomposition; 
// will need to destruct later
Matrix* parseReturn_luGetU(ParseReturn *ret) {
    if(ret->type!=3) return NULL;
    return lu_getU(ret->lu);
}

// returns the matrix Q from QR decomposition; 
// will need to destruct later
Matrix* parseReturn_qrGetQ(ParseReturn *ret) {
    if(ret->type!=4) return NULL;
    return grahmschmidt_getQ(ret->qr);
}

// returns the matrix R from QR decomposition; 
// will need to destruct later
Matrix* parseReturn_qrGetR(ParseReturn *ret) {
    if(ret->type!=4) return NULL;
    return grahmschmidt_getR(ret->qr);
}

// returns teh values from Eigen
// will need to destruct later
Matrix* parseReturn_eGetValues(ParseReturn *ret) {
    if(ret->type!=5) return NULL;
    return eigen_getValue(ret->e);
}

// returns teh values from Eigen
// will need to destruct later
Matrix* parseReturn_eGetVectors(ParseReturn *ret) {
    if(ret->type!=5) return NULL;
    return eigen_getVector(ret->e);
}

// Returns 0 for a double value, 1 for a matrix value, 2 for bool
int parseReturn_getType(ParseReturn *ret) {
    return ret->type;   
}

// returns unrecognized var name
char* parseReturn_getVarName(ParseReturn *ret) {
    if(ret->type != -2 ) return NULL;
    return ret->varName;
}

// returns unrecognized char 
char parseReturn_getToken(ParseReturn *ret) {
    if(ret->type != 12 ) return '\0';
    return ret->unrec;
}

// returns the matrix value; if flag is 1, returns the actual matrix pointer (do not destruct)
// if flag is 0, returns a copy (will need to destruct)
Matrix* parseReturn_getMatrix(ParseReturn *ret, int flag) {
    if(flag) {
        return ret->val;
    }
    else {
        Matrix *temp;
        matrix_initializeFromMatrix(&temp, ret->val);
        return temp;
    }
}

// Attempts to add a new variable to parser; if it is already present return 0; return 1 if succesful
int parser_addVariable(Parser *myParse, char *name, Matrix *val, double value) {
    Variable *var = (Variable*) malloc(sizeof(Variable));
    var->ID = strdup(name);
    if(val==NULL) {
        var->value.number = value;
        var->type = 0;
    }
    else {
        var->type = 1;
        matrix_initializeFromMatrix(&var->value.mat, val);
    }
   
    if(avltree_contains(myParse->variables, var)) {
        free(var->ID);
        free(var);
        return 0;
    }
    else {
        avltree_add(myParse->variables, var);
        return 1;
    }
}

// Attempts to remove and delete variable from parser; returns 1 if succesful; 0 otherwise
int parser_removeVariable(Parser *myParse, char *name) {
    Variable *var = (Variable*) malloc(sizeof(Variable));
    var->ID = strdup(name);
    int ret = avltree_remove(myParse->variables, var);
    free(var->ID);
    free(var);
    return ret;
}

// adds matrix variable; if name is already present, replace
void parser_addVariableMatrix(Parser *myParse, char *name, Matrix *mat)  {
    Variable *var = (Variable*) malloc(sizeof(Variable));
    var->ID = strdup(name);
    var->value.mat = mat;
    var->type = 1;
    if(avltree_contains(myParse->variables, var)) avltree_replace(myParse->variables, var);
    else avltree_add(myParse->variables, var);
}

// Attempts to replace old variable with new value to parser; if it not present return 0; return 1 if succesful
int parser_replaceVariable(Parser *myParse, char *name, double value) {
    Variable *var = (Variable*) malloc(sizeof(Variable));
    var->ID = strdup(name);
    var->value.number = value;
    var->type = 0;
    if(avltree_replace(myParse->variables, var)) {
        return 1;
    }
    else {
        free(var->ID);
        free(var);
        return 0;
    }
}

int parser_getVariableType(Parser *myParse, char *name) {
	Variable *var = (Variable*) malloc(sizeof(Variable));
	var->ID = strdup(name);
	Variable *ret = avltree_find(myParse->variables, var);
   // printf("Tree Var type: %d\n", ret->type);
	free(var->ID);
	free(var);
	if(ret==NULL) return -1;
	return ret->type;
}

Matrix* parser_getVariableMatrix(Parser *myParse, char *name) {
	Variable *var = (Variable*) malloc(sizeof(Variable));
	var->ID = strdup(name);
	Variable *ret = avltree_find(myParse->variables, var);
	free(var->ID);
	free(var);
	if(ret==NULL) return NULL;
	if(ret->type != 1) return NULL;
	return ret->value.mat;
}

int parser_getAssignmentType(Parser *myParse, char *name) {
	Variable *var = (Variable*) malloc(sizeof(Variable));
	var->ID = strdup(name);
	Variable *ret = avltree_find(myParse->variables, var);
   // printf("Tree Var type: %d\n", ret->type);
	free(var->ID);
	free(var);
	if(ret==NULL) return -1;
	return ret->type;
}

double parser_getAssignmentValue(Parser *myParse, char *name) {
	Variable *var = (Variable*) malloc(sizeof(Variable));
	var->ID = strdup(name);
	Variable *ret = avltree_find(myParse->variables, var);
	free(var->ID);
	free(var);
	if(ret==NULL) return -1;
	if(ret->type != 0) return -1;
	return ret->value.number;
}

Matrix* parser_getAssignmentMatrix(Parser *myParse, char *name) {
	Variable *var = (Variable*) malloc(sizeof(Variable));
	var->ID = strdup(name);
	Variable *ret = avltree_find(myParse->variables, var);
	free(var->ID);
	free(var);
	if(ret==NULL) return NULL;
	if(ret->type != 1) return NULL;
	return ret->value.mat;
}

