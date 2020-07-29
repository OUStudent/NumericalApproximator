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
 *          DoublyLinkedList *myLL;
 *          doublyLinkedList_initialize(&myLL, compare, freeValue);  // must pass in address of pointer as it allocates memory for myLL here
 *          // Where 'compare' is a User defined function for comparint two values
 *          // Where 'freeValue' is a User defined function for freeing value of DoublyNode if value is dynamic; return NULL if value is primitive
 *      
 *      // Example function calls
 *          doublyLinkedList_add(myLL, someValue1);
 *          doublyLinkedList_addFirst(myLL, someValue2);
 *          doublyLinkedList_add(myLL, someValue3); 
 *          doublyLinkedList_mergeSort(myLL);
 *          int index = doublyLinkedList_indexOf(myLL, someValue2);
 *          doublyLinkedList_destructor(&myLL); // free allocted memory
 *          
 * *****/


#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include <stdlib.h>
#include <stdio.h>

#include "Enumerator.h"


/***
 * 
 * Data Structure Definitions only Below:
 * 
 * ***/ 

typedef struct DoublyLinkedList DoublyLinkedList;

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// DoublyLinkedList

void doublyLinkedList_addFirst(DoublyLinkedList *myLL, void* value);
void doublyLinkedList_addLast(DoublyLinkedList *myLL, void* value);
void doublyLinkedList_add(DoublyLinkedList *myLL, void* value);
int doublyLinkedList_addAtIndex(DoublyLinkedList *myLL, void *value, int index);
int doublyLinkedList_contains(DoublyLinkedList *myLL, void* value);
void* doublyLinkedList_find(DoublyLinkedList *myLL, void* value);
void* doublyLinkedList_getAtIndex(DoublyLinkedList *myLL, int index);
int doublyLinkedList_getSize(DoublyLinkedList *myLL);
void doublyLinkedList_destructor(DoublyLinkedList **myLL);
void doublyLinkedList_initialize(DoublyLinkedList **myLL, int (*compare)(const void*, const void*), void (*freeValue) (void *));
int doublyLinkedList_indexOf(DoublyLinkedList *myLL, void *value);
int doublyLinkedList_isSorted(DoublyLinkedList *myLL);
int doublyLinkedList_removeLast(DoublyLinkedList *myLL);
int doublyLinkedList_removeFirst(DoublyLinkedList *myLL);
int doublyLinkedList_removeAtIndex(DoublyLinkedList *myLL, int index);
int doublyLinkedList_removeNode(DoublyLinkedList *myLL, void *value);
int doublyLinkedList_replace(DoublyLinkedList *myLL, void *value);
void doublyLinkedList_resetEnumerator(DoublyLinkedList *myLL);

// MergeSort

void doublyLinkedList_mergeSort(DoublyLinkedList *myLL, int order); // smallest to largest

// Enumerator

int doublyLinkedList_hasNext(void* dataStructure);
void* doublyLinkedList_next(void* dataStructure);
void* doublyLinkedList_peek(void* dataStructure);
void doublyLinkedList_resetEnumerator(DoublyLinkedList *myLL);

#endif // DOUBLYLINKEDLIST_H
