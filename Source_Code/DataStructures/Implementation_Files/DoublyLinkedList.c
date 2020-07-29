
#include "../DoublyLinkedList.h"

/***
 * 
 * Data Structure Declarations only Below:
 * 
 * ***/ 

struct DoublyNode {
    void *value;
    struct DoublyNode *next;
    struct DoublyNode *prev;
};
typedef struct DoublyNode DoublyNode;

struct DoublyLinkedList {
    Enumerator enumerator;
    DoublyNode *head;
    DoublyNode *tail;
    int isSorted;
    int order; // 1 is smallest to largest; 0 is largest to smallest;
    int size;
    int (*compare)(const void*, const void*); // for l & r: return -1 if l < r, 0 if l==r, 1 if l > r
    void (*freeValue) (void *); // User defined function for freeing value of node
};


/****
 * 
 * Function declarations and implementations below:
 * 
 * ***/

// Adds a value to the front of the list ; also checks for sortedness
void doublyLinkedList_addFirst(DoublyLinkedList *myLL, void* value) {
    myLL->size++;
    DoublyNode *newNode = (DoublyNode*) malloc(sizeof(DoublyNode));
    newNode->value = value;
    newNode->next = NULL;
    newNode->prev = NULL;
    if(myLL->head==NULL) {
        myLL->head = newNode;
        myLL->tail = newNode;
        myLL->isSorted = 1;
        myLL->enumerator.start = myLL->head;
        return;
    }
    else {
        newNode->next = myLL->head;
        myLL->head->prev = newNode;
        if(myLL->size==1) myLL->tail = myLL->head;
        if(myLL->compare(myLL->head->value, value)==1 && myLL->isSorted==1) myLL->isSorted = 1; // new value is less than head and was previously sorted
        else myLL->isSorted = 0;
        myLL->head = newNode;
        myLL->enumerator.start = myLL->head;
    }
}

// Adds a value to the end of the list ; also checks for sortedness
void doublyLinkedList_addLast(DoublyLinkedList *myLL, void* value) {
    myLL->size++;
    DoublyNode *newNode = (DoublyNode*) malloc(sizeof(DoublyNode));
    newNode->value = value;
    newNode->next = NULL;
    newNode->prev = NULL;
    if(myLL->tail==NULL) { // linked list is null
        myLL->head = newNode;
        myLL->tail = newNode;
        myLL->isSorted = 1;
        myLL->enumerator.start = myLL->head;
        return;
    }
    else {
        newNode->prev = myLL->tail;
        myLL->tail->next = newNode;
        if(myLL->size==1) myLL->head = myLL->tail;
        if(myLL->compare(myLL->tail->value, value)==-1 && myLL->isSorted==1) myLL->isSorted = 1; // new value is greater than tail and was previously sorted
        else myLL->isSorted = 0;
        myLL->tail = newNode;
    }
}

// Adds a value at the end of the list by calling another function
void doublyLinkedList_add(DoublyLinkedList *myLL, void* value) {
    doublyLinkedList_addLast(myLL, value);
}

// Adds a value at a specified index; 1 if succesful; 0 otherwise
int doublyLinkedList_addAtIndex(DoublyLinkedList *myLL, void *value, int index) {
    // index is above bounds: include index because add at end of list 
    // Example: LL size is 2 so indices are 0,1 but add at index 2, AKA the end of the list
    if(myLL->size < index) return 0; 
    
    if(index == 0) doublyLinkedList_addFirst(myLL, value); // if adding at head
    else if(index== myLL->size) doublyLinkedList_addLast(myLL, value); // adding at tail
    else { // Index is between head and tail
        DoublyNode *temp = myLL->head;
        // iterate through index
        for(int i = 0; i < index; i++) {
            if(temp==NULL) return 0; // error checking
            temp = temp->next;
        }
        DoublyNode *newNode = (DoublyNode*) malloc(sizeof(DoublyNode));
        newNode->value = value;
        newNode->next = temp;
        newNode->prev = temp->prev;
        temp->prev->next = newNode;
        temp->prev = newNode;
        if(myLL->compare(newNode->prev->value, value)==-1 && myLL->isSorted==1) myLL->isSorted = 1; // new value is greater than prev and was previously sorted
        else myLL->isSorted = 0;
        if(myLL->compare(newNode->next->value, value)==1 && myLL->isSorted==1) myLL->isSorted = 1; // new value is less than next and was previously sorted
        else myLL->isSorted = 0;
        myLL->size++;
    }
    return 1;
}

// Returns 0 if a specified value is not present; 1 if the value is found
int doublyLinkedList_contains(DoublyLinkedList *myLL, void* value) {
    DoublyNode *temp = myLL->head;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) return 1;
        temp = temp->next;
    }
    return 0;
}

// The destructor of the DoublyLinkedList
void doublyLinkedList_destructor(DoublyLinkedList **myLL) {
    DoublyNode *temp = (*myLL)->head;
    DoublyNode *t2;
    while(temp!=NULL) {
        (*myLL)->freeValue(temp->value);
        t2 = temp->next;
        free(temp);
        temp = t2;
    }
    free(*myLL);
}

// Returns NULL if a specified value is not present; the implemented value if found
void* doublyLinkedList_find(DoublyLinkedList *myLL, void* value) {
    DoublyNode *temp = myLL->head;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) return temp->value;
        temp = temp->next;
    }
    return 0;
}

// Returns value at a given index; returns NULL if index out of bounds or value not present
void* doublyLinkedList_getAtIndex(DoublyLinkedList *myLL, int index) {
    if(index > myLL->size) return NULL;
    DoublyNode *temp = myLL->head;
    for(int i = 0; i < index; i++) {
        if(temp==NULL) return NULL; // shouldn't happen because of previous check but myLL.size could be changed on accident
        temp = temp->next;
    }
    return temp;
}

// Returns the size of the list
int doublyLinkedList_getSize(DoublyLinkedList *myLL) {
    return myLL->size;
}

// Returns the index of a specified value; returns -1 if value is not present
int doublyLinkedList_indexOf(DoublyLinkedList *myLL, void *value) {
    DoublyNode *temp = myLL->head;
    int index = 0;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) return index; // found
        temp = temp->next;
        index++;
    }
    return -1; // not found
}

// Returns 1 if the list is sorted based unto the given order
int doublyLinkedList_isSorted(DoublyLinkedList *myLL) {
    return myLL->isSorted;
}

// The initializer/constructor that declares all the values
void doublyLinkedList_initialize(DoublyLinkedList **myLL, int (*compare)(const void*, const void*), void (*freeValue) (void *)) {
    (*myLL) = (DoublyLinkedList*) malloc(sizeof(DoublyLinkedList));
    (*myLL)->size = 0;
    (*myLL)->isSorted = 0;
    (*myLL)->head = NULL;
    (*myLL)->tail = NULL;
    (*myLL)->compare = NULL;
    (*myLL)->freeValue = NULL;
    (*myLL)->enumerator.hasNext = doublyLinkedList_hasNext;
    (*myLL)->enumerator.next = doublyLinkedList_next;
    (*myLL)->enumerator.peek = doublyLinkedList_peek;
    (*myLL)->enumerator.start = (*myLL)->head;
    (*myLL)->order = 1;
    (*myLL)->compare = compare;
    (*myLL)->freeValue = freeValue;
}

// Removes the first DoublyNode in the linked list; returns 1 if succesful; 0 otherwise
int doublyLinkedList_removeFirst(DoublyLinkedList *myLL) {
    if(myLL->size==0) return 0;
    else if(myLL->size ==1) { // Only one node present in Linked List
        myLL->freeValue(myLL->head->value);
        free(myLL->head);
        myLL->head = NULL;
        myLL->tail = NULL;
        myLL->size--;
    }
    else { // More than one node
        DoublyNode* temp = myLL->head;
        temp->next->prev = NULL;
        myLL->head = temp->next;
        myLL->freeValue(temp->value);
        free(temp);
        myLL->size--;
    }
    return 1;
}

// Removes the last DoublyNode in the linked list; returns 1 if succesful; 0 otherwise
int doublyLinkedList_removeLast(DoublyLinkedList *myLL) {
    if(myLL->size==0) return 0;
    else if(myLL->size ==1) { // Only one node present in Linked List
        myLL->freeValue(myLL->head->value);
        free(myLL->head);
        myLL->head = NULL;
        myLL->tail = NULL;
        myLL->size--;
    }
    else { // More than one node
        DoublyNode* temp = myLL->tail;
        temp->prev->next = NULL;
        myLL->tail = temp->prev;
        myLL->freeValue(temp->value);
        free(temp);
        myLL->size--;
    }
    return 1;
}

// Replaces the 'original' value of the node with 'value' ; returns 1 if succesful; 0 otherwise
// WARNING!!!! The passed in value cannot be the same pointer to a previous value passed in; but must have same 'value' for comparator
int doublyLinkedList_replace(DoublyLinkedList *myLL, void *value) {
    DoublyNode *temp = myLL->head;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) {
            myLL->freeValue(temp->value); // WARNING: Read above, if 'value' is the same pointer to temp->value, then this free will cause an error
            temp->value = value;
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

// Removes a specified DoublyNode given a value; returns 1 if successful; 0 otherwise
int doublyLinkedList_removeNode(DoublyLinkedList *myLL, void *value) {
    DoublyNode *temp = myLL->head;
    int index = 0;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) {
            if(index==0) return doublyLinkedList_removeFirst(myLL); // removing first node
            if(index==myLL->size-1) return doublyLinkedList_removeLast(myLL);  // removing last node
            DoublyNode *t2 = temp->next;
            temp->prev->next = t2;
            temp->next->prev = temp->prev;
            myLL->freeValue(temp->value);
            free(temp);
            myLL->size--;
            return 1;
        }
        temp=temp->next;
        index++;
    }
    free(temp);
    return 0;
}

// Removing a node at a specified index; returns 1 if successfu; 0 otherwise
int doublyLinkedList_removeAtIndex(DoublyLinkedList *myLL, int index) {
    if(index > myLL->size) return 0;
    if(index == 0|| myLL->size==1) return doublyLinkedList_removeFirst(myLL);
    else if(index+1== myLL->size) return doublyLinkedList_removeLast(myLL);
    else { // removing value inbetween head and tail nodes
        DoublyNode *temp = myLL->head;
        for(int i = 0; i < index; i++) {
            if(temp==NULL) return 0; // shouldn't happen because of previous check but myLL.size could be changed on accident
            temp = temp->next;
        }
        DoublyNode *t2 = temp->next;
        temp->next->prev = temp->prev;
        temp->prev->next = t2;
        myLL->freeValue(temp->value);
        free(temp);
        myLL->size--;
        return 1;
    }
}

/***
 * 
 * Enumerator Functions only Below:
 * 
 * ***/

// If the enumerator has a next value, return 1; otherwise 0
int doublyLinkedList_hasNext(void* dataStructure) {
    DoublyLinkedList *myLL = (DoublyLinkedList*) dataStructure;
    DoublyNode *temp = (DoublyNode*) myLL->enumerator.start;
    if(temp== NULL) return 0;
    else return 1;
}

// Returns the current value from the enumerator and sets it to the next value
void* doublyLinkedList_next(void* dataStructure) {
    DoublyLinkedList *myLL = (DoublyLinkedList*) dataStructure;
    DoublyNode *temp = (DoublyNode*) myLL->enumerator.start;
    myLL->enumerator.start = temp->next;
    return temp->value;
}

// Returns the current value from the enumerator
void* doublyLinkedList_peek(void* dataStructure) {
    DoublyLinkedList *myLL = (DoublyLinkedList*) dataStructure;
    DoublyNode *temp = (DoublyNode*) myLL->enumerator.start;
    return temp->value;
}

// Resets the Enumerator to start at the head of the linked list
void doublyLinkedList_resetEnumerator(DoublyLinkedList *myLL) {
    myLL->enumerator.start = myLL->head;
}


/*****
 * MergeSort Algorithm for Doubly Linked List Below Only-
 * Given an 'order' 1 for smallest to largest; 0 for largest to smallest
 * Based off of GeeksForGeeks implementation:
 *      https://www.geeksforgeeks.org/merge-sort-for-doubly-linked-list/
 * 
 ***/

static DoublyNode* doublyMergeSortHelper(DoublyLinkedList *myLL, DoublyNode *l, DoublyNode *h, int order);
static DoublyNode* getMiddleDoublyMergeSortHelper(DoublyNode *start); 
static DoublyNode* doublyMergeSortMainHelper(DoublyLinkedList *myLL, DoublyNode* head, int order);


// Performs a MergeSort algorithm onto the linked list
// If order is 1 sorts from smallest to largest value based unto the given comaprator
// If order is 0 sorts from largest to smallest value based unto the given comaprator
void doublyLinkedList_mergeSort(DoublyLinkedList *myLL, int order) {
    if(myLL->isSorted && order==myLL->order) return; // no need to waste computations
    myLL->head = doublyMergeSortMainHelper(myLL, myLL->head, order);
    DoublyNode *temp = myLL->tail; // use this starting point as it is better than starting from head
    while(temp->next!=NULL) {
        temp = temp->next;
    }
    myLL->isSorted = 1;
    myLL->tail = temp;
    myLL->enumerator.start = myLL->head;
}

// The 'merge' function in mergesort
static DoublyNode* doublyMergeSortHelper(DoublyLinkedList *myLL, DoublyNode *l, DoublyNode *h, int order) {
     if(l==NULL) return h;
     if(h==NULL) return l;
     if(order) { // smallest to largest
        if(myLL->compare(l->value, h->value)==-1) { // l < h smallest to largest
            l->next = doublyMergeSortHelper(myLL, l->next, h, order);
            l->next->prev = l;
            l->prev = NULL;
            return l;
        }
        else {
            h->next = doublyMergeSortHelper(myLL, l, h->next, order);
            h->next->prev = h;
            h->prev = NULL;
            return h;
        }
     }
     else { // largest to smallest
          if(myLL->compare(l->value, h->value)==1) { // l > h // largest to smallest
            l->next = doublyMergeSortHelper(myLL, l->next, h, order);
            l->next->prev = l;
            l->prev = NULL;
            return l;
        }
        else {
            h->next = doublyMergeSortHelper(myLL, l, h->next, order);
            h->next->prev = h;
            h->prev = NULL;
            return h;
        }
     }
     
}

// Find the middle value from a start
static DoublyNode* getMiddleDoublyMergeSortHelper(DoublyNode *start) {
    DoublyNode *fast = start;
    DoublyNode *slow = start; 
    while (fast->next && fast->next->next) 
    { 
        fast = fast->next->next; 
        slow = slow->next; 
    } 
    DoublyNode *temp = slow->next; 
    slow->next = NULL; 
    return temp; 
}

// Main helper function
static DoublyNode* doublyMergeSortMainHelper(DoublyLinkedList *myLL, DoublyNode* head, int order) {
    if(head == NULL || head->next==NULL) return head;
    DoublyNode *middle = getMiddleDoublyMergeSortHelper(head);
    head = doublyMergeSortMainHelper(myLL, head, order);
    middle = doublyMergeSortMainHelper(myLL, middle, order);
    return(doublyMergeSortHelper(myLL, head, middle, order));
}
