/****
 * Author: Brandon Morgan
 * Last Updated: 5/18/2020
 * Description:
 *      Generic/Templated Stack data structure. Stores data in a Last-in First-Out methodology.
 * 
 * Example Use in external File:
 *      // These next Two lines must always be done before using
 *          Stack *myStack; 
 *          stack_initialize(&myStack, compare, freeValue); // must pass in address of pointer
 *          // Where 'compare' is a User defined function for comparint two values
 *          // Where 'freeValue' is a User defined function for freeing value of Node if value is dynamic; return NULL if value is primitive
 *      
 *      // Example function calls
 *          stack_push(myStack, someValue1);
 *          stack_push(myStack, someValue2);
 *          printMethod(stack_pop(myStack)); // where printMethod() is a user defined method on how to print the data type
 *          stack_destructor(&myStack); // free allocted memory & must pass in address of pointer
 * *****/

#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stdio.h>

#include "Enumerator.h"

/***
 * 
 * Data Structure Definitions only Below:
 * 
 * ***/ 


typedef struct Stack Stack;

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// Stack

void stack_destructor(Stack **myStack);
void stack_freeNodes(Stack *myStack); // free only Nodes, not values
int stack_getSize(Stack *myStack);
void stack_initialize(Stack **myStack, int (*compare)(const void*, const void*), void (*freeValue) (void *));
void* stack_pop(Stack *myStack);
void stack_push(Stack *myStack, void* value);

// Enumerator

int stack_hasNext(void* dataStructure);
void* stack_next(void* dataStructure);
void* stack_peek(void* dataStructure);
void stack_resetEnumerator(Stack *myStack);

#endif // STACK_H
