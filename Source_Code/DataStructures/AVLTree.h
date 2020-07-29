
/****
 * Author: Brandon Morgan
 * Last Updated: 5/18/2020
 * Description:
 *      Generic/Templated AVL Tree. An AVL Tree is a binary search tree that 'balances' itself
 *      out using a rebalance function.
 * 
 * Example Use in external File:
 *      // These next Two lines must always be done before using
 *          AVLTree *myTree;
 *          avltree_initialize(&myTree, compare, freeValue); // must pass in address of pointer as it allocates memory for myTree here
 *          // Where 'compare' is a User defined function for comparint two values
 *          // Where 'freeValue' is a User defined function for freeing value of Node if value is dynamic; return NULL if value is primitive       
 * 
 *      // Example function calls
 *          avltree_add(myTree, someValue1);
 *          avltree_replace(myTree, someValue2);
 *          printf("Found: %d\n", avltree_find(myTree, someValue3)); // Note: printf() requires <stdio.h> library
 *          avltree_destructor(&myTree); // free allocted memory
 * *****/

#ifndef AVLTREE_H
#define AVLTREE_H

#include "Stack.h"
#include "Enumerator.h"
#include <stdlib.h>
#include <stdio.h>


/***
 * 
 * Data Structures Definitions only Below:
 * 
 * ***/ 

typedef struct AVLTree AVLTree;


/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// AVL Tree

int avltree_add(AVLTree *tree, void* value);
int avltree_contains(AVLTree *tree, void* value) ;
void avltree_destructor(AVLTree **myTree);
void* avltree_find(AVLTree *tree, void* value);
void avltree_initialize(AVLTree **tree, int (*compare)(const void*, const void*), void (*freeValue) (void *), int order);
int avltree_replace(AVLTree *tree, void* value);
int avltree_remove(AVLTree *tree, void* value);
void avltree_setOrder(AVLTree *tree, int order);

// Enumerator

int avltree_hasNext(void* dataStructure);
void* avltree_peek(void* dataStructure);
void* avltree_next(void* dataStructure);
void avltree_resetEnumerator(AVLTree *myTree);


#endif // AVLTREE_H
