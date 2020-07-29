#include "../Scanner.h"

/***
 * 
 * Static Constant Variables and Definitions
 * 
 * ***/ 

const int NUMBER_TOKENS = 17;
static char *TOKENS[] = {"ID", "NUMBER", "OP11", "OP12", "L_PAR", "R_PAR", "OP2", "ASSIGN", "OP3", "NEG", "KEY", 
                    "L_BRACK", "R_BRACK", "OBTAIN", "COMMA", "FUNCTION", "STATICFUNCTION"};
// OP11: +, OP12: -, OP2: * /, OP3: ^:
// key: trig/ reserved words

const int NUMBER_RESERVEDWORDS = 10; // include e and PI
static char *RESERVEDWORDS[] = {"cos", "sin", "tan", "acos", "asin", "atan", "log", "ln", "e", "PI"};

const int NUMBER_FUNCTIONS = 22; // t is for transpose
static char *FUNCTIONNAMES[] = {"naiveGauss", "t", "houseHolder", "LU", "QR", "eigen", "integrate", "gamma", 
                            "qchisq", "pchisq", "newton", "bisection", "getRow", "getCol", 
                            "cov", "cor", "zscore", "mean", "backSub", "forwSub", "qnorm", "pnorm"};

const int STATIC_FUNCTION_NUMBER = 7;
static char *FUNCTIONNAMES_STATIC[] = {"isSquare", "isSymmetric", "isPosDef", "setRow", "setCol", "delRow", "delCol"};

static const double e = 2.7182818284590452353602874713527; // Euler's Number
static const double PI = 3.14159265358979323846264338327;
/***
 * 
 * Data Structure Declarations only Below:
 * 
 * ***/ 

union VarValue {
    Matrix *mat;
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

struct Scanner {
    SinglyLinkedList *myLL; // tokens: Linked List of Entries
    int size; // number of tokens
    char* original; // the original input text
};


/***
 * 
 * Static Function Definitions only Below:
 * 
 * ***/

// Returns index at which keyword is found; else -1
static int isKeyWord(char *s) {
    for(int i = 0; i < NUMBER_RESERVEDWORDS; i++) {
        if(strcmp(s, RESERVEDWORDS[i])==0) return i;
    }
    return -1;
}

// Returns index at which function name is found; else -1
static int isFunction(char *s) {
    for(int i = 0; i < NUMBER_FUNCTIONS; i++) {
        if(strcmp(s, FUNCTIONNAMES[i])==0) return i;
    }
    return -1;
}

// Returns index at which function name is found; else -1
static int isStaticFunction(char *s) {
    for(int i = 0; i < STATIC_FUNCTION_NUMBER; i++) {
        if(strcmp(s, FUNCTIONNAMES_STATIC[i])==0) return i;
    }
    return -1;
}

// The Free function for Singly Linked List
static void freeValueLL(void *value) {
    Entry *e = (Entry*) value;
    if(e->index==0) { free(e->value.var->ID); free(e->value.var); }
    free(value);
}

// The Compare function for Singly Linked List
static int compareLL(const void *left, const void *right) {
    Entry *l = (Entry*) left;
    Entry *r = (Entry*) right;
    if(l->index==r->index) {
        if(l->index==0) { // matching ID
            if(l->value.var->ID==NULL) return 0;
            return strcmp(l->value.var->ID, r->value.var->ID);
        }
        else return 0;
    }
    else if (l->index < r->index) return -1;
    else return 1;
}

// Helper function that adds a new entry to the linked list
static void addEntry(Scanner *myScan, int *index, char *c, double num, char *ID, int token) {
    Entry *entry = (Entry*) malloc(sizeof(Entry));
    Input input;
    if(token==10) input.number = num;
    else if(c==NULL && ID==NULL) input.number = num;
    else if(ID==NULL) input.operand = *c;
    else { 
        input.var = (Variable *) malloc(sizeof(Variable)); 
        input.var->ID = ID; 
        input.var->value.number = 0;
        input.var->type = 0;
    }
    entry->value = input;
    entry->index = token;
    myScan->size++;
    singlyLinkedList_add(myScan->myLL, entry);
    *index += 1;
}

// Helper function for returning value in scan function
// If char unrecognized holds a value return the ascii value
// If an scanner error occured return 0
// If no error occured return 1
static int scanReturnHelper(Scanner *myScan, int error, char unrecognized) {
    if(unrecognized != '\0' || error) {
        singlyLinkedList_destructor(&(myScan)->myLL); // delete stored values
        singlyLinkedList_initialize(&(myScan)->myLL, compareLL, freeValueLL); // re-initialize scanner
        
        if(unrecognized != '\0') return (int) unrecognized;
        else return 0;
    }
    return 1;
}


/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// Destructor
void scanner_destructor(Scanner **myScan) {
    singlyLinkedList_destructor(&(*myScan)->myLL);
    if((*myScan)->original !=NULL) free((*myScan)->original);
    free((*myScan));
}

// Returns size (number of tokens) in scanner
int scanner_getSize(Scanner *myScan) {
    return myScan->size;
}

// Returns the original input that was given 
// will need to be freed later
char* scanner_getInput(Scanner *myScan) {
    char *temp = strdup(myScan->original);
    return temp;
}

// Returns the Entry node at the index of the scanner
void* scanner_getAtIndex(Scanner *myScan, int index) {
    return singlyLinkedList_getAtIndex(myScan->myLL, index);
}

// Initiailizer
void scanner_initialize(Scanner **myScan) {
    (*myScan) = (Scanner *) malloc(sizeof(Scanner));
    singlyLinkedList_initialize(&(*myScan)->myLL, compareLL, freeValueLL);
    (*myScan)->size = 0;
    (*myScan)->original=NULL;
}

// Initializes the tokens from oldScan into newScan
void scanner_initializeFromTokens(Scanner **newScan, Scanner *oldScan) {
    scanner_initialize(newScan);
    scanner_scan(*newScan, oldScan->original);
}

// Scans the input and breaks it down into tokens;
// Returns an int value signifying whether or not an error occured:
// If an scanner error occured return 0
// If no error occured return 1
// If a different int value is returned, it is an asci value of an unrecognized char (so cast return value to char)
// If a scanner error or unrecognized symbol occured Scanner, tokens are deleted
int scanner_scan(Scanner *myScan, char *input) {
    char c;
    int SIZE = 10; //15
    char *myString;
    int index = 0;
    int lineIndex = 0;
    int errorScanner = 0;
    char unrecognized = '\0';
    myScan->original = strdup(input);
    while(input[lineIndex]!='\0') {
        c = input[lineIndex];
        Loop: // {"ID", "NUMBER", "OP11", "OP12", "L_PAR", "R_PAR", "OP2", "ASSIGN", "OP3"};
        if(c==' ' || c=='\n') {lineIndex++; continue; }
        if(c=='+') {
            addEntry(myScan, &index, &c, -1, NULL, 2);
        }
        else if( c=='-') {
            addEntry(myScan, &index, &c, -1, NULL, 3);
        }
        else if(c=='(') {
            addEntry(myScan, &index, &c, -1, NULL, 4);
        }
        else if(c==')') {
            addEntry(myScan, &index, &c, -1, NULL, 5);
        }
        else if(c=='*' || c=='/') {
            addEntry(myScan, &index, &c, -1, NULL, 6);
        }
        else if(c=='=') {
            addEntry(myScan, &index, &c, -1, NULL, 7);
        }
        else if(c=='^') {
            addEntry(myScan, &index, &c, -1, NULL, 8);
        }
        else if( c=='~') {
            addEntry(myScan, &index, &c, -1, NULL, 9);
        }
        else if( c=='[') {
            addEntry(myScan, &index, &c, -1, NULL, 11);
        }
        else if(c==']') {
            addEntry(myScan, &index, &c, -1, NULL, 12);
        }
        else if(c=='$') {
            addEntry(myScan, &index, &c, -1, NULL, 13);
        }
        else if(c==',') {
            addEntry(myScan, &index, &c, -1, NULL, 14);
        }
        else if(isdigit(c)||c=='.') {
            int firstDecimal = 1;
            if(c=='.') firstDecimal = 0;
            myString = (char*) malloc(sizeof(char) * SIZE);
            for(int i = 0; i < SIZE; i++) {
                myString[i] = '\0';
            }
            int iter = 0;
            int inFile;
            do {
                if(iter == SIZE-1) { // resize
                    int prevSize = SIZE;
                    myString = realloc(myString, sizeof(char)*(SIZE*=1.5));
                    for(int i = prevSize; i < SIZE; i++) {
                        myString[i] = '\0';
                    }
                }
                myString[iter++] = c; 
                inFile = input[++lineIndex];

                if(inFile =='\0') { // end of file reached
                    addEntry(myScan, &index, NULL, atof(myString), NULL, 1);
                    free(myString);    
                    addEntry(myScan, &index, NULL, -1, NULL, -1);  // last item , $$            
                    return scanReturnHelper(myScan, errorScanner, unrecognized);
                }

                c = inFile;
                if(c=='.') {
                    if(firstDecimal) firstDecimal=0;
                    else {
                        printf("ERROR: Two decimals found\n");
                        errorScanner = 1;
                        unrecognized = c;
                        return scanReturnHelper(myScan, errorScanner, unrecognized);
                    }
                }
                 
            } while(isdigit(c)||c=='.');
            addEntry(myScan, &index, NULL, atof(myString), NULL, 1);
            free(myString);

            goto Loop;
        }
        else if(isalpha(c)||c=='_') {
            myString = (char*) malloc(sizeof(char) * SIZE);
            for(int i = 0; i < SIZE; i++) {
                myString[i] = '\0';
            }
            int iter = 0;
            int firstDecimal = 1;
            int inFile;
            int key;
            int function;
            int staticFunction;
            do {
                if(iter == SIZE-1) { // resize
                    int prevSize = SIZE;
                    myString = realloc(myString, sizeof(char)*(SIZE*=1.5));
                    for(int i = prevSize; i < SIZE; i++) {
                        myString[i] = '\0';
                    }
                }
                myString[iter++] = c; 
                inFile = input[++lineIndex];

               if(inFile =='\0') { // end of file reached
                    key = isKeyWord(myString);
                    function = isFunction(myString);
                    staticFunction = isStaticFunction(myString);
                    if(key >= 0 && key != 8 && key != 9 || function >= 0 || staticFunction >= 0 && lineIndex) {
                        addEntry(myScan, &index, NULL, -1, myString, 0);
                    }
                    else {
                        if(key==-1 && function==-1 && staticFunction==-1) addEntry(myScan, &index, NULL, -1, myString, 0);
                        else if(function==-1 && staticFunction==-1){ // key
                            if(key==8) addEntry(myScan, &index, NULL, e, NULL, 1);
                            else if(key==9) addEntry(myScan, &index, NULL, PI, NULL, 1);
                            else addEntry(myScan, &index, NULL, key, NULL, 10);
                            free(myString);
                        }  
                        else if(key==-1 && staticFunction==-1) { // function
                            addEntry(myScan, &index, NULL, function, NULL, 15);
                            free(myString);
                        }
                        else {
                            addEntry(myScan, &index, NULL, staticFunction, NULL, 16);
                            free(myString);
                        }
                    }
                    addEntry(myScan, &index, NULL, -1, NULL, -1); // last item , $$          
                    return scanReturnHelper(myScan, errorScanner, unrecognized);
                }

                c = inFile;
                if(c=='.') {
                    if(firstDecimal) firstDecimal=0;
                    else {
                        printf("ERROR: Two decimals found\n");
                        errorScanner = 1;
                        unrecognized = c;
                        return scanReturnHelper(myScan, errorScanner, unrecognized);
                    }
                }
                 
            } while(isalpha(c)||c=='_');
            key = isKeyWord(myString);
            function = isFunction(myString);
            staticFunction = isStaticFunction(myString);
            if(key >= 0 && key != 8 && key != 9 || function >= 0 || staticFunction >= 0) {
                inFile = input[lineIndex];
                if(inFile != '(') {
                    addEntry(myScan, &index, NULL, -1, myString, 0);
                    goto Loop;
                }
                else goto ADD; // few lines below
            }
            else {
                ADD:
                if(key==-1 && function==-1 && staticFunction==-1) addEntry(myScan, &index, NULL, -1, myString, 0);
                else if(function==-1 && staticFunction==-1){ // key
                    if(key==8) addEntry(myScan, &index, NULL, e, NULL, 1);
                    else if(key==9) addEntry(myScan, &index, NULL, PI, NULL, 1);
                    else addEntry(myScan, &index, NULL, key, NULL, 10);
                    free(myString);
                }  
                else if(key==-1 && staticFunction==-1) { // function
                    addEntry(myScan, &index, NULL, function, NULL, 15);
                    free(myString);
                }
                else {
                    addEntry(myScan, &index, NULL, staticFunction, NULL, 16);
                    free(myString);
                }
                goto Loop;
            }
        }
        else {
            unrecognized = c;
            errorScanner = 1;
            return scanReturnHelper(myScan, errorScanner, unrecognized);
        }
        lineIndex++;
    }
    addEntry(myScan, &index, NULL, -1, NULL, -1);  // last item , $$
    return scanReturnHelper(myScan, errorScanner, unrecognized);
}

// Prints out the tokens to stdout
void scanner_print(Scanner *myScan) {
    while(singlyLinkedList_hasNext(myScan->myLL)) {
        Entry *e = singlyLinkedList_next(myScan->myLL);
        if(e->index==-1) continue;
        printf("%s ", TOKENS[e->index]);
    }
    printf("\n");
    singlyLinkedList_resetEnumerator(myScan->myLL);
        while(singlyLinkedList_hasNext(myScan->myLL)) {
        Entry *e = singlyLinkedList_next(myScan->myLL);
        if(e->index==-1) continue;
        else if(e->index==0) printf("%s ", e->value.var->ID);
        else if (e->index==10) printf("%s ", RESERVEDWORDS[(int) e->value.number]);
        else if(e->index==1) printf("%f ", e->value.number);
        else printf("%c ", e->value.operand);
    }
    printf("\n");
    singlyLinkedList_resetEnumerator(myScan->myLL);
}


/***
 * 
 * Enumerator Functions only Below:
 * 
 * ***/

// If the enumerator has a next value, return 1; otherwise 0
int scanner_hasNext(Scanner *myScan) {
    return singlyLinkedList_hasNext(myScan->myLL);
}

// Returns the current value from the enumerator
void* scanner_peek(Scanner *myScan) {
    return singlyLinkedList_peek(myScan->myLL);
}

// Returns the current value from the enumerator and sets it to the next value
void* scanner_next(Scanner *myScan) {
    return singlyLinkedList_next(myScan->myLL);
}

// Resets the enumerator
void scanner_resetEnumerator(Scanner *myScan) {
    singlyLinkedList_resetEnumerator(myScan->myLL);
}

