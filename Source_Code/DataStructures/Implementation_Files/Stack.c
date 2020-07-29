// Implementation file for Stack.h

#include "../Stack.h"

/***
 * 
 * Data Structures only Below:
 * 
 * ***/ 

struct StackNode {
    void *value;
    struct StackNode *prev;
};
typedef struct StackNode StackNode; 
struct Stack {
    StackNode *top;
    Enumerator enumerator;
    int size;
    int (*compare)(const void*, const void*); // for l & r: return -1 if l < r, 0 if l==r, 1 if l > r
    void (*freeValue) (void *); // User defined function for freeing value of node
};

/****
 * 
 * Function declarations and implementations below:
 * 
 * ***/

// Destructor for the Stack
void stack_destructor(Stack **myStack) {
    while((*myStack)->size > 0) {
        void *value = stack_pop(*myStack);
        (*myStack)->freeValue(value);
    }
    free(*myStack);
}

// frees the nodes but not the values inside of nodes
void stack_freeNodes(Stack *myStack) {
    if(myStack->size==0)return;
    StackNode *temp = myStack->top;
    StackNode *t2;
    while(temp!=NULL) {
        t2 = temp->prev;
        free(temp);
        temp = t2;
    }
}

static void stack_intializeHelper(Stack** myStack) {
    *myStack = (Stack*) malloc(sizeof(Stack));
}

// Initializes the Stack fields
void stack_initialize(Stack **myStack, int (*compare)(const void*, const void*), void (*freeValue) (void *)) {
    *myStack = (Stack*) malloc(sizeof(Stack));
    (*myStack)->top = NULL;
    (*myStack)->size = 0;
    (*myStack)->compare = compare;
    (*myStack)->freeValue = freeValue;
    (*myStack)->enumerator.hasNext = stack_hasNext;
    (*myStack)->enumerator.next = stack_next;
    (*myStack)->enumerator.peek = stack_peek;
    (*myStack)->enumerator.start = NULL;

}

// Returns size of stack
int stack_getSize(Stack *myStack) {
    return myStack->size;
}

// Returns a value and removes it from the top of the stack
void* stack_pop(Stack *myStack) {
    void* value = myStack->top->value;
    StackNode *temp = myStack->top;
    myStack->top = myStack->top->prev;
    free(temp);
    myStack->size--;
    return value;
}

// Places a value onto the top of the stack
void stack_push(Stack *myStack, void* value) {
    StackNode *node = (StackNode*) malloc(sizeof(StackNode));
    node->value = value;
    myStack->size++;
    if(myStack->size-1==0) { // first entry
        node->prev = NULL;
        myStack->top = node;
    }
    else {
        node->prev = myStack->top;
        myStack->top = node;
    }
    myStack->enumerator.start = myStack->top;
}

/***
 * 
 * Enumerator Functions only Below:
 * 
 * ***/

// If the enumerator has a next value, return 1; otherwise 0
int stack_hasNext(void* dataStructure) {
    Stack *myStack = (Stack*) dataStructure;
    StackNode *temp = (StackNode*) myStack->enumerator.start;
    if(temp == NULL) return 0;
    else return 1;
}

// Returns the current value from the enumerator and sets it to the next value
void* stack_next(void* dataStructure) {
    Stack *myStack = (Stack*) dataStructure;
    StackNode *temp = (StackNode*) myStack->enumerator.start;
    myStack->enumerator.start = temp->prev;
    return temp->value;
}

// Returns the current value from the enumerator
void* stack_peek(void* dataStructure) {
    Stack *myStack = (Stack*) dataStructure;
    StackNode *temp = (StackNode*) myStack->enumerator.start;
    return temp->value;
}

// Resets the Enumerator to start at the head of the linked list
void stack_resetEnumerator(Stack *myStack) {
   myStack->enumerator.start = myStack->top;
}
