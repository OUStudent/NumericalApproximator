/****
 * Author: Brandon Morgan
 * Last Updated: 5/18/2020
 * Description:
 *      Linked List Generic/Template Library implementing a Doubly Linked List capable of 
 *      storing data in a linked list fashion. A mergesort algorith is also
 *      given as well to sort the linked list.
 * 
 * Example Use in external File:
 *      // These next Two lines must always be done before using
 *          SinglyLinkedList *myLL;
 *          singlyLinkedList_initialize(&myLL, compare, freeValue); // must pass in address of pointer as it allocates memory for myLL here
 *          // Where 'compare' is a User defined function for comparint two values
 *          // Where 'freeValue' is a User defined function for freeing value of SinglyNode if value is dynamic; return NULL if value is primitive
 *      
 *      // Example function calls
 *          singlyLinkedList_add(myLL, someValue1);
 *          singlyLinkedList_addFirst(myLL, someValue2);
 *          singlyLinkedList_add(myLL, someValue3); 
 *          singlyLinkedList_mergeSort(myLL);
 *          int index = singlyLinkedList_indexOf(myLL, someValue2);
 *          singlyLinkedList_destructor(&myLL);
 *          
 * *****/


#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdlib.h>
#include <stdio.h>

#include "Enumerator.h"


/***
 * 
 * Data Structure Definitions only Below:
 * 
 * ***/ 

typedef struct SinglyLinkedList SinglyLinkedList;

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// SinglyLinkedList

void singlyLinkedList_addFirst(SinglyLinkedList *myLL, void* value);
void singlyLinkedList_addLast(SinglyLinkedList *myLL, void* value);
void singlyLinkedList_add(SinglyLinkedList *myLL, void* value);
int singlyLinkedList_addAtIndex(SinglyLinkedList *myLL, void *value, int index);
int singlyLinkedList_contains(SinglyLinkedList *myLL, void* value);
void* singlyLinkedList_find(SinglyLinkedList *myLL, void* value);
void* singlyLinkedList_getAtIndex(SinglyLinkedList *myLL, int index);
int singlyLinkedList_getSize(SinglyLinkedList *myLL);
void singlyLinkedList_destructor(SinglyLinkedList **myLL);
void singlyLinkedList_initialize(SinglyLinkedList **myLL, int (*compare)(const void*, const void*), void (*freeValue) (void *));
int singlyLinkedList_indexOf(SinglyLinkedList *myLL, void *value);
int singlyLinkedList_isSorted(SinglyLinkedList *myLL);
int singlyLinkedList_removeLast(SinglyLinkedList *myLL);
int singlyLinkedList_removeFirst(SinglyLinkedList *myLL);
int singlyLinkedList_removeAtIndex(SinglyLinkedList *myLL, int index);
int singlyLinkedList_removeNode(SinglyLinkedList *myLL, void *value);
int singlyLinkedList_replace(SinglyLinkedList *myLL, void *value);
void singlyLinkedList_resetEnumerator(SinglyLinkedList *myLL);

// MergeSort

void singlyLinkedList_mergeSort(SinglyLinkedList *myLL, int order); // smallest to largest

// Enumerator

int singlyLinkedList_hasNext(void* dataStructure);
void* singlyLinkedList_next(void* dataStructure);
void* singlyLinkedList_peek(void* dataStructure);
void singlyLinkedList_resetEnumerator(SinglyLinkedList *myLL);

#endif // LINKEDLIST_H
