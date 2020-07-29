
#include "../HashTable.h"

/***
 * 
 * Static Constant Variables and Definitions
 * 
 * ***/ 

#define SCHEDULE_SIZE 25
#define DEFAULT_SCHEDULE_INDEX 3

static const int SCHEDULE[SCHEDULE_SIZE] = { 1, 2, 5, 11, 23, 53, 107, 223, 449, 907, 1823, 3659, 7309, 14621, 29243, 58511, 117023, 234067, 468157, 936319, 1872667, 3745283, 7490573, 14981147, 29962343 };
static const long DEFAULT_BASE_CAPACITY = 11;
static const float DEFAULT_MAX_LOAD_FACTOR = 0.8f;
static const float DEFAULT_MIN_LOAD_FACTOR = 0.3f;


/***
 * 
 * Data Structure Declarations only Below:
 * 
 * ***/ 

struct HashNode { // static ?
    void *value;
    struct HashNode *next;
};
typedef struct HashNode HashNode;

struct HashList {
    Enumerator enumerator;
    HashNode *head;
    int size;
    void (*freeValue) (void *); // User defined function for freeing value of node
};
typedef struct HashList HashList;

struct HashTable {
    Enumerator enumerator;
    int enumIndex;
    int firstBucketIndex;
    float maxLoadFactor;
    float minLoadFactor;
    unsigned long size; // number of elements
    int scheduleIndex;
    long baseCapacity; // size of the array
    long totalCapacity; // size of the array with the sizes of each array index aka collisions
    HashList** table;
    int (*compare)(const void*, const void*); // for l & r: return -1 if l < r, 0 if l==r, 1 if l > r
    void (*freeValue) (void *); // User defined function for freeing value of node
    int (*hashFunction) (const void *); // Hash function for placing values inside of table
};

/***
 * 
 * Function Definitions only Below:
 * 
 * ***/

// Hash List  ALL MUST BE STATIC

static void hashList_addFirst(HashList*myLL, void* value);
static void hashList_addLast(HashList*myLL, void* value);
static void hashList_add(HashList*myLL, void* value);
static int hashList_contains(HashTable* myTable, HashList *myLL, void* value);
static void hashList_destructor(HashList **myLL);
static void* hashList_find(HashTable* myTable, HashList *myLL, void* value);
static void hashList_initialize(HashList **myLL, void (*freeValue) (void *));
static int hashList_removeLast(HashList*myLL);
static int hashList_removeFirst(HashList*myLL);
static int hashList_removeNode(HashTable* myTable, HashList *myLL, void *value);
static int hashList_replace(HashTable* myTable, HashList *myLL, void* value);

// Enumerator

static int hashList_hasNext(void* dataStructure);
static void* hashList_next(void* dataStructure); 
static void* hashList_peek(void* dataStructure); 
static void hashList_resetEnumerator(HashList *myLL); 

// Hash Table

static HashList** initializeTable(HashList **table, int baseCapacity, void (*freeValue) (void *));
static void hashTableInitializeHelper(HashTable *myHashTable, int (*compare)(const void*, const void*), void (*freeValue) (void *), int (*hashFunction) (const void *));
static void resizeUp(HashTable *myTable);
static void resizeDown(HashTable *myTable);


/****
 * 
 * Function declarations and implementations below:
 * 
 * ***/

/**
 * 
 * HASH LIST
 * 
 *  ***/

// Adds a value to the front of the list ; also checks for sortedness
static void hashList_addFirst(HashList *myLL, void* value) {
    myLL->size++;
    HashNode *newNode = (HashNode*) malloc(sizeof(HashNode));
    newNode->value = value;
    newNode->next = NULL;
    if(myLL->head==NULL) {
        myLL->head = newNode;
        return;
    }
    else {
        newNode->next = myLL->head;
        myLL->head = newNode;
    }
}

// Adds a value to the end of the list ; also checks for sortedness
static void hashList_addLast(HashList *myLL, void* value) {
    myLL->size++;
    HashNode *newNode = (HashNode*) malloc(sizeof(HashNode));
    newNode->value = value;
    newNode->next = NULL;
    HashNode *temp = myLL->head;
    while(temp->next !=NULL) {
        temp = temp->next;
    }
    temp->next = newNode;
}

// Adds a value at the first of the list by calling another function
static void hashList_add(HashList *myLL, void* value) {
    hashList_addFirst(myLL, value);
}

// Adds a value at a specified index; 1 if succesful; 0 otherwise
static int hashList_addAtIndex(HashList *myLL, void *value, int index) {
    // index is above bounds: include index because add at end of list 
    // Example: LL size is 2 so indices are 0,1 but add at index 2, AKA the end of the list
    if(myLL->size < index) return 0; 
    
    if(index == 0) hashList_addFirst(myLL, value); // if adding at head
    else if(index== myLL->size) hashList_addLast(myLL, value); // adding at tail
    else { // Index is between head and tail
        HashNode *temp = myLL->head;
        HashNode *prev = NULL;
        // iterate through index
        for(int i = 0; i < index; i++) {
            if(temp==NULL) return 0; // error checking
            prev = temp;
            temp = temp->next;
        }
        HashNode *newNode = (HashNode*) malloc(sizeof(HashNode));
        newNode->value = value;
        newNode->next = temp;
        prev->next = newNode;
       
        myLL->size++;
    }
    return 1;
}

// The destructor of the HashList
static void hashList_destructor(HashList **myLL) {
    HashNode *temp = (*myLL)->head;
    HashNode *t2;
    while(temp!=NULL) {
        (*myLL)->freeValue(temp->value);
        t2 = temp->next;
        free(temp);
        temp = t2;
    }
    free(*myLL);
}

// Returns 0 if a specified value is not present; 1 if the value is found
static int hashList_contains(HashTable* myTable, HashList *myLL, void* value) {
    HashNode *temp = myLL->head;
    while(temp!=NULL) {
        if(myTable->compare(temp->value, value)==0) return 1;
        temp = temp->next;
    }
    return 0;
}

// Returns NULL if a specified value is not present; the implemented value if found
static void* hashList_find(HashTable* myTable, HashList *myLL, void* value) {
    HashNode *temp = myLL->head;
    while(temp!=NULL) {
        if(myTable->compare(temp->value, value)==0) return temp->value;
        temp = temp->next;
    }
    return 0;
}


// The initializer/constructor that declares all the values
static void hashList_initialize(HashList **myLL, void (*freeValue) (void *)) {
    (*myLL) = (HashList*) malloc(sizeof(HashList));
    (*myLL)->size = 0;
    (*myLL)->head = NULL;
    (*myLL)->freeValue = freeValue;
}

// Replaces the 'original' value of the node with 'value' ; returns 1 if succesful; 0 otherwise
// WARNING!!!! The passed in value cannot be the same pointer to a previous value passed in; but must have same 'value' for comparator
static int hashList_replace(HashTable* myTable, HashList *myLL, void* value) {
     HashNode  *temp = myLL->head;
    while(temp!=NULL) {
        if(myTable->compare(temp->value, value)==0) {
            myTable->freeValue(temp->value); // WARNING: Read above, if 'value' is the same pointer to temp->value, then this free will cause an error
            temp->value = value;
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

// Removes the first HashNode in the linked list; returns 1 if succesful; 0 otherwise
static int hashList_removeFirst(HashList *myLL) {
    if(myLL->size==0) return 0;
    else if(myLL->size ==1) { // Only one node present in Linked List
        myLL->freeValue(myLL->head->value);
        free(myLL->head);
        myLL->head = NULL;
        myLL->size--;
    }
    else { // More than one node
        HashNode* temp = myLL->head;
        myLL->head = temp->next;
        myLL->freeValue(temp->value);
        free(temp);
        myLL->size--;
    }
    return 1;
}

// Removes the last HashNode in the linked list; returns 1 if succesful; 0 otherwise
static int hashList_removeLast(HashList *myLL) {
    if(myLL->size==0) return 0;
    else if(myLL->size ==1) { // Only one node present in Linked List
        myLL->freeValue(myLL->head->value);
        free(myLL->head);
        myLL->head = NULL;
        myLL->size--;
    }
    else { // More than one node
        HashNode* temp = myLL->head;
        while(temp->next!=NULL) {
            temp = temp->next;
        }
        myLL->freeValue(temp->value);
        free(temp);
        myLL->size--;
    }
    return 1;
}

// Removes a specified HashNode given a value; returns 1 if successful; 0 otherwise
static int hashList_removeNode(HashTable* myTable, HashList *myLL, void *value) {
    HashNode *temp = myLL->head;
    HashNode *prev = NULL;
    int index = 0;
    while(temp!=NULL) {
        if(myTable->compare(temp->value, value)==0) {
            if(index==0) return hashList_removeFirst(myLL); // removing first node
            if(index==myLL->size-1) return hashList_removeLast(myLL);  // removing last node
            HashNode *t2 = temp->next;
            prev->next = t2;
            myTable->freeValue(temp->value);
            free(temp);
            myLL->size--;
            return 1;
        }
        prev = temp;
        temp=temp->next;
        index++;
    }
    free(temp);
    return 0;
}


/**
 * 
 * HASH TABLE
 * 
 *  ***/

// Returns 1 if the passed in value is found; 0 otherwise
int hashTable_contains(HashTable *myTable, void *value) {
    int bucketNumber = myTable->hashFunction(value) % myTable->baseCapacity;
    return hashList_contains(myTable, myTable->table[bucketNumber], value);
}


// destructor
void hashTable_destructor(HashTable **myTable) {
    for(int i = 0; i < SCHEDULE[(*myTable)->scheduleIndex]; i++) {
        hashList_destructor(&((*myTable)->table[i]));
    }
    free((*myTable)->table);
    free(*myTable);
}

// Finds the value in the hash table and returns its value; NULL if not present
// NOTE: useful if data value has key and value where key is used in compare function
//       ex: given a data value with a key, find the associated key data structure and return its value
void* hashTable_find(HashTable *myTable, void *value) {
    int bucketNumber = myTable->hashFunction(value) % myTable->baseCapacity;
    return hashList_find(myTable, myTable->table[bucketNumber], value);
}
int hashTable_getBaseCapacity(HashTable *myTable) {
    return myTable->baseCapacity;
}
int hashTable_getTotalCapacity(HashTable *myTable) {
    return myTable->totalCapacity;
}

// Returns a 1 if the value is inserted correctly; 0 otherwise
int hashTable_insert(HashTable *myTable, void *value) {
    int bucketNumber = myTable->hashFunction(value) % myTable->baseCapacity;
    if(myTable->table[bucketNumber]->size==0) { // item to be inserted is in an empty Linked List
        hashList_add(myTable->table[bucketNumber], value);
        myTable->size++;
        resizeUp(myTable); // checks if need to resize
        return 1;
    }

    // item to be inserted is in a non-empty linked list
	// Check to see it item is already present in linked list
    if(hashList_contains(myTable, myTable->table[bucketNumber], value)) return 0;

    // item is not present, and LL is not empty
    hashList_add(myTable->table[bucketNumber], value);
    myTable->totalCapacity++;
    myTable->size++;
    resizeUp(myTable); // checks if need to resize
    return 1;
}

// The default initializer for Hash Table
void hashTable_initializeDefault(HashTable **myHashTable, int (*compare)(const void*, const void*), void (*freeValue) (void *), int (*hashFunction) (const void *)) {
    (*myHashTable) = (HashTable*) malloc(sizeof(HashTable));
    (*myHashTable)->maxLoadFactor = DEFAULT_MAX_LOAD_FACTOR;
    (*myHashTable)->minLoadFactor = DEFAULT_MIN_LOAD_FACTOR;
    (*myHashTable)->size = 0;
    (*myHashTable)->scheduleIndex = DEFAULT_SCHEDULE_INDEX;
    (*myHashTable)->baseCapacity = DEFAULT_BASE_CAPACITY;
    hashTableInitializeHelper(*myHashTable, compare, freeValue, hashFunction);
}

// Alternate initializer for user to specify which specifications to use
// -1 on the parameters indicates default value
void hashTable_initializeParameters(HashTable **myHashTable, float maxLoad, float minLoad, int scheduleIndex, int (*compare)(const void*, const void*), void (*freeValue) (void *), int (*hashFunction) (const void *)) {
    (*myHashTable) = (HashTable*) malloc(sizeof(HashTable));
    (*myHashTable)->size = 0;
    if(maxLoad==-1) (*myHashTable)->maxLoadFactor = DEFAULT_MAX_LOAD_FACTOR;
    else (*myHashTable)->maxLoadFactor = maxLoad;

    if(minLoad==-1) (*myHashTable)->minLoadFactor = DEFAULT_MIN_LOAD_FACTOR;
    else (*myHashTable)->minLoadFactor = minLoad;

    if(scheduleIndex==-1) {
        (*myHashTable)->scheduleIndex = DEFAULT_SCHEDULE_INDEX;
        (*myHashTable)->baseCapacity = DEFAULT_BASE_CAPACITY;
    }
    else {
        (*myHashTable)->scheduleIndex = scheduleIndex;
        (*myHashTable)->baseCapacity = SCHEDULE[scheduleIndex];
    }
    hashTableInitializeHelper(*myHashTable, compare, freeValue, hashFunction);
}

// Initializes the Table
static HashList** initializeTable(HashList **table, int baseCapacity, void (*freeValue) (void *)) {
    table = (HashList **) malloc(sizeof(HashList*)*baseCapacity);
    for(int i = 0; i < baseCapacity; i++) {
        hashList_initialize(&(table[i]), freeValue); 
    }
    return table;
}

// Helper function
static void hashTableInitializeHelper(HashTable *myHashTable, int (*compare)(const void*, const void*), void (*freeValue) (void *), int (*hashFunction) (const void *)) {
    myHashTable->totalCapacity = myHashTable->baseCapacity;
    myHashTable->firstBucketIndex = -1;
    myHashTable->enumIndex = -1;
    myHashTable->enumerator.hasNext = hashTable_hasNext;
    myHashTable->enumerator.peek = hashTable_peek;
    myHashTable->enumerator.next = hashTable_next;
    myHashTable->compare = compare;
    myHashTable->freeValue = freeValue;
    myHashTable->hashFunction = hashFunction;
    myHashTable->table = initializeTable(myHashTable->table, myHashTable->baseCapacity,myHashTable->freeValue);
}

// Calculates the load factor of the table
static float getLoadFactor(HashTable *myTable) {
    return (float) myTable->size / (float) myTable->totalCapacity;
}

// Returns a 1 if the data value is present; 0 otherwise
int hashTable_remove(HashTable *myTable, void *value) {
    int bucketNumber = myTable->hashFunction(value) % myTable->baseCapacity;

    if(myTable->table[bucketNumber]->size==0) return 0; // item to be removed is in an empty Linked List

    if(hashList_removeNode(myTable, myTable->table[bucketNumber], value)) {
        myTable->size--;
        myTable->totalCapacity--;
        resizeDown(myTable);
    }
    else return 0; // value was not present in linked list
}

// Returns a 1 if the value was replaced correctly; 0 otherwise
// NOTE: useful if data value has key and value where key is used in compare function
//       ex: a given a data value with a key, if the associated key data structure is found, replace its 'data' with the new 'data'
int hashTable_replace(HashTable *myTable, void *value) {
    int bucketNumber = myTable->hashFunction(value) % myTable->baseCapacity;
    if(myTable->table[bucketNumber]->size==0) return 0;
    return hashList_replace(myTable, myTable->table[bucketNumber], value);
}

// Checks to see if the hash table needs to be resized up
static void resizeUp(HashTable *myTable) {
    if(getLoadFactor(myTable) > myTable->maxLoadFactor) {
        myTable->scheduleIndex++;
        long oldBase = myTable->baseCapacity;
        myTable->baseCapacity = myTable->totalCapacity = SCHEDULE[myTable->scheduleIndex];
        HashList** newTable;
        newTable = initializeTable(newTable, myTable->baseCapacity, myTable->freeValue);
        // loops over every linked list and adds to new table
        for(int i = 0; i < oldBase; i++) {
            HashList *row = myTable->table[i];
            HashNode* temp = row->head;
            HashNode* t2;
            while(temp!=NULL) {
                void* item = temp->value;
                int bucketNumber = myTable->hashFunction(item) % myTable->baseCapacity;
                if(newTable[bucketNumber]->size > 0) myTable->totalCapacity++; // overflow 
                hashList_add(newTable[bucketNumber], item);
                t2 = temp->next;
                free(temp);
                temp = t2;
            }
           free(row);
        }
        free(myTable->table);
        myTable->table = newTable;
    }
    hashTable_resetEnumerator(myTable);
}

// Checks to see if the hash table needs to be resized down
static void resizeDown(HashTable *myTable) {
    if(getLoadFactor(myTable) < myTable->minLoadFactor) {
        myTable->scheduleIndex--;
        long oldBase = myTable->baseCapacity;
        myTable->baseCapacity = myTable->totalCapacity = SCHEDULE[myTable->scheduleIndex];
        HashList** newTable;
        newTable = initializeTable(newTable, myTable->baseCapacity, myTable->freeValue);
        // loops over every linked list and adds to new table
        for(int i = 0; i < oldBase; i++) {
            HashList *row = myTable->table[i];
            HashNode* temp = row->head;
            HashNode* t2;
            while(temp!=NULL) {
                void* item = temp->value;
                int bucketNumber = myTable->hashFunction(item) % myTable->baseCapacity;
                if(newTable[bucketNumber]->size > 0) myTable->totalCapacity++; // overflow 
                hashList_add(newTable[bucketNumber], item);
                t2 = temp->next;
                free(temp);
                temp = t2;
            }
           free(row);
        }
        free(myTable->table);
        myTable->table = newTable;
    }
    hashTable_resetEnumerator(myTable);
}

int hashTable_getSize(HashTable *myTable) {
    return myTable->size;
}

/***
 * 
 * Enumerator Functions only Below:
 * 
 * ***/

// If the enumerator has a next value, return 1; otherwise 0
static int hashList_hasNext(void* dataStructure) {
    HashList *myLL = (HashList*) dataStructure;
    HashNode *temp = (HashNode*) myLL->enumerator.start;
    if(temp== NULL) return 0;
    else return 1;
}

// Returns the current value from the enumerator and sets it to the next value
static void* hashList_next(void* dataStructure) {
    HashList *myLL = (HashList*) dataStructure;
    HashNode *temp = (HashNode*) myLL->enumerator.start;
    myLL->enumerator.start = temp->next;
    return temp->value;
}

// Returns the current value from the enumerator
static void* hashList_peek(void* dataStructure) {
    HashList *myLL = (HashList*) dataStructure;
    HashNode *temp = (HashNode*) myLL->enumerator.start;
    return temp->value;
}

// Resets the Enumerator to start at the head of the linked list
static void hashList_resetEnumerator(HashList *myLL) {
    myLL->enumerator.start = myLL->head;
}

// If the enumerator has a next value, return 1; otherwise 0
int hashTable_hasNext(void* dataStructure) {
    HashTable* myTable = (HashTable*) dataStructure;
    if(myTable->enumIndex==-1) return 0;
    
    if(!hashList_hasNext(myTable->table[myTable->enumIndex])) {
        for(int i = myTable->enumIndex+1; i < myTable->baseCapacity; i++) {
            if(myTable->table[i]->size > 0) {
                myTable->enumIndex = i;
                goto End;
            }
        }
        return 0;
    }
    End:
    return 1;
}

// Returns the current value from the enumerator and sets it to the next value
void* hashTable_next(void* dataStructure) {
    HashTable* myTable = (HashTable*) dataStructure;
    return hashList_next(myTable->table[myTable->enumIndex]);
}

// Returns the current value from the enumerator
void* hashTable_peek(void* dataStructure) {
    HashTable* myTable = (HashTable*) dataStructure;
    return hashList_peek(myTable->table[myTable->enumIndex]);
}

// Resets the Enumerator to start at the head of the linked list
void hashTable_resetEnumerator(HashTable* myTable) {
    int first = 1;
    for(int i = 0; i < myTable->baseCapacity; i++) {
        if(myTable->table[i]->size > 0) { // loop until non empty buckets are found
            hashList_resetEnumerator(myTable->table[i]);
            if(first) myTable->enumIndex = i;
            first = 0;
        }
    }
    if(first) myTable->enumIndex = -1;
}
