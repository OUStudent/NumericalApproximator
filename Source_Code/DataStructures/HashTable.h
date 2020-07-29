
/****
 * Author: Brandon Morgan
 * Last Updated: 5/18/2020
 * Description:
 *      Generic/Templated Hash Table data structure. Stores data using the "HashList.h" file.
 *      Handles collisions through linked list chaining. 
 * 
 * Example Use in external File:
 *      // These next Two lines must always be done before using
 *          HashTable *myTable; 
 *          hashtable_initializeDefault(&myTable, compare, freeValue, hashFunction); // must pass in address of pointer as it allocates memory for myTable here
 *          // Where 'compare' is a User defined function for comparint two values
 *          // Where 'freeValue' is a User defined function for freeing value of HashNode if value is dynamic; return NULL if value is primitive
 *          // Where 'hashFunction' is a User defined function for 'hashing' the data value         
 * 
 *      // Example function calls
 *          hashtable_insert(myTable, someValue1);
 *          hashtable_insert(myTable, someValue2);
 *          printf("Found: %d\n", hashtable_find(myTable, someValue3)); // Note: printf() requires <stdio.h>
 *          hashtable_destructor(&myTable); // free allocted memory
 * *****/

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "Enumerator.h"
#include <stdlib.h>

/***
 * 
 * Data Structure Definitions only Below:
 * 
 * ***/ 

typedef struct HashTable HashTable;

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// Hash Table

int hashTable_contains(HashTable *myTable, void *value);
void hashTable_destructor(HashTable **myTable);
void* hashTable_find(HashTable *myTable, void *value);
int hashTable_getBaseCapacity(HashTable *myTable);
int hashTable_getTotalCapacity(HashTable *myTable);
int hashTable_insert(HashTable *myTable, void *value);
void hashTable_initializeDefault(HashTable **myHashTable, int (*compare)(const void*, const void*), void (*freeValue) (void *), int (*hashFunction) (const void *));
void hashTable_initializeParameters(HashTable **myHashTable, float maxLoad, float minLoad, int scheduleIndex, int (*compare)(const void*, const void*), void (*freeValue) (void *), int (*hashFunction) (const void *));
int hashTable_remove(HashTable *myTable, void *value);
int hashTable_replace(HashTable *myTable, void *value);
int hashTable_getSize(HashTable *myTable);

// Enumerator

int hashTable_hasNext(void* dataStructure);
void* hashTable_next(void* dataStructure);
void* hashTable_peek(void* dataStructure);
void hashTable_resetEnumerator(HashTable* myTable);


#endif // HASHTABL_H
