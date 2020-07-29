
#include "../SinglyLinkedList.h"

/***
 * 
 * Data Structure Declarations only Below:
 * 
 * ***/ 

struct SinglyNode {
    void *value;
    struct SinglyNode *next;
};
typedef struct SinglyNode SinglyNode;

struct SinglyLinkedList {
    Enumerator enumerator;
    SinglyNode *head;
    SinglyNode *tail;
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
void singlyLinkedList_addFirst(SinglyLinkedList *myLL, void* value) {
    myLL->size++;
    SinglyNode *newNode = (SinglyNode*) malloc(sizeof(SinglyNode));
    newNode->value = value;
    newNode->next = NULL;
    if(myLL->head==NULL) {
        myLL->head = newNode;
        myLL->tail = newNode;
        myLL->isSorted = 1;
        myLL->enumerator.start = myLL->head;
        return;
    }
    else {
        newNode->next = myLL->head;
        if(myLL->size==1) myLL->tail = myLL->head;
        if(myLL->compare(myLL->head->value, value)==1 && myLL->isSorted==1) myLL->isSorted = 1; // new value is less than head and was previously sorted
        else myLL->isSorted = 0;
        myLL->head = newNode;
        myLL->enumerator.start = myLL->head;
    }
}

// Adds a value to the end of the list ; also checks for sortedness
void singlyLinkedList_addLast(SinglyLinkedList *myLL, void* value) {
    myLL->size++;
    SinglyNode *newNode = (SinglyNode*) malloc(sizeof(SinglyNode));
    newNode->value = value;
    newNode->next = NULL;
    if(myLL->tail==NULL) { // linked list is null
        myLL->head = newNode;
        myLL->tail = newNode;
        myLL->isSorted = 1;
        myLL->enumerator.start = myLL->head;
        return;
    }
    else {
        myLL->tail->next = newNode;
        if(myLL->size==1) myLL->head = myLL->tail;
        if(myLL->compare(myLL->tail->value, value)==-1 && myLL->isSorted==1) myLL->isSorted = 1; // new value is greater than tail and was previously sorted
        else myLL->isSorted = 0;
        myLL->tail = newNode;
    }
}

// Adds a value at the end of the list by calling another function
void singlyLinkedList_add(SinglyLinkedList *myLL, void* value) {
    singlyLinkedList_addLast(myLL, value);
}

// Adds a value at a specified index; 1 if succesful; 0 otherwise
int singlyLinkedList_addAtIndex(SinglyLinkedList *myLL, void *value, int index) {
    // index is above bounds: include index because add at end of list 
    // Example: LL size is 2 so indices are 0,1 but add at index 2, AKA the end of the list
    if(myLL->size < index) return 0; 
    
    if(index == 0) singlyLinkedList_addFirst(myLL, value); // if adding at head
    else if(index== myLL->size) singlyLinkedList_addLast(myLL, value); // adding at tail
    else { // Index is between head and tail
        SinglyNode *temp = myLL->head;
        SinglyNode *prev = NULL;
        // iterate through index
        for(int i = 0; i < index; i++) {
            if(temp==NULL) return 0; // error checking
            prev = temp;
            temp = temp->next;
        }
        SinglyNode *newNode = (SinglyNode*) malloc(sizeof(SinglyNode));
        newNode->value = value;
        newNode->next = temp;
        prev->next = newNode;
        if(myLL->compare(prev->value, value)==-1 && myLL->isSorted==1) myLL->isSorted = 1; // new value is greater than prev and was previously sorted
        else myLL->isSorted = 0;
        if(myLL->compare(newNode->next->value, value)==1 && myLL->isSorted==1) myLL->isSorted = 1; // new value is less than next and was previously sorted
        else myLL->isSorted = 0;
        myLL->size++;
    }
    return 1;
}

// Returns 0 if a specified value is not present; 1 if the value is found
int singlyLinkedList_contains(SinglyLinkedList *myLL, void* value) {
    SinglyNode *temp = myLL->head;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) return 1;
        temp = temp->next;
    }
    return 0;
}

// The destructor of the SinglyLinkedList
void singlyLinkedList_destructor(SinglyLinkedList **myLL) {
    SinglyNode *temp = (*myLL)->head;
    SinglyNode *t2;
    while(temp!=NULL) {
        (*myLL)->freeValue(temp->value);
        t2 = temp->next;
        free(temp);
        temp = t2;
    }
    free(*myLL);
}

// Returns NULL if a specified value is not present; the implemented value if found
void* singlyLinkedList_find(SinglyLinkedList *myLL, void* value) {
    SinglyNode *temp = myLL->head;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) return temp->value;
        temp = temp->next;
    }
    return 0;
}

// Returns the size of the list
int singlyLinkedList_getSize(SinglyLinkedList *myLL) {
    return myLL->size;
}

// Returns value at a given index; returns NULL if index out of bounds or value not present
void* singlyLinkedList_getAtIndex(SinglyLinkedList *myLL, int index) {
    if(index > myLL->size) return NULL;
    SinglyNode *temp = myLL->head;
    for(int i = 0; i < index; i++) {
        if(temp==NULL) return NULL; // shouldn't happen because of previous check but myLL.size could be changed on accident
        temp = temp->next;
    }
    return temp->value;
}

// Returns the index of a specified value; returns -1 if value is not present
int singlyLinkedList_indexOf(SinglyLinkedList *myLL, void *value) {
    SinglyNode *temp = myLL->head;
    int index = 0;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) return index; // found
        temp = temp->next;
        index++;
    }
    return -1; // not found
}

// Returns 1 if the list is sorted based unto the given order
int singlyLinkedList_isSorted(SinglyLinkedList *myLL) {
    return myLL->isSorted;
}

// The initializer/constructor that declares all the values
void singlyLinkedList_initialize(SinglyLinkedList **myLL, int (*compare)(const void*, const void*), void (*freeValue) (void *)) {
    (*myLL) = (SinglyLinkedList*) malloc(sizeof(SinglyLinkedList));
    (*myLL)->size = 0;
    (*myLL)->isSorted = 0;
    (*myLL)->head = NULL;
    (*myLL)->tail = NULL;
    (*myLL)->compare = NULL;
    (*myLL)->freeValue = NULL;
    (*myLL)->enumerator.hasNext = singlyLinkedList_hasNext;
    (*myLL)->enumerator.next = singlyLinkedList_next;
    (*myLL)->enumerator.peek = singlyLinkedList_peek;
    (*myLL)->enumerator.start = (*myLL)->head;
    (*myLL)->order = 1;
    (*myLL)->compare = compare;
    (*myLL)->freeValue = freeValue;
}

// Removes the first SinglyNode in the linked list; returns 1 if succesful; 0 otherwise
int singlyLinkedList_removeFirst(SinglyLinkedList *myLL) {
    if(myLL->size==0) return 0;
    else if(myLL->size ==1) { // Only one node present in Linked List
        myLL->freeValue(myLL->head->value);
        free(myLL->head);
        myLL->head = NULL;
        myLL->tail = NULL;
        myLL->size--;
    }
    else { // More than one node
        SinglyNode* temp = myLL->head;
        myLL->head = temp->next;
        myLL->freeValue(temp->value);
        free(temp);
        myLL->size--;
    }
    return 1;
}

// Removes the last SinglyNode in the linked list; returns 1 if succesful; 0 otherwise
int singlyLinkedList_removeLast(SinglyLinkedList *myLL) {
    if(myLL->size==0) return 0;
    else if(myLL->size ==1) { // Only one node present in Linked List
        myLL->freeValue(myLL->head->value);
        free(myLL->head);
        myLL->head = NULL;
        myLL->tail = NULL;
        myLL->size--;
    }
    else { // More than one node
        SinglyNode* temp = myLL->head;
        for(int i = 0; i < myLL->size-1; i++) {
            temp = temp->next;
        }
        SinglyNode* t2 = myLL->tail;
        myLL->tail = temp;;
        myLL->freeValue(t2->value);
        free(t2);
        myLL->size--;
    }
    return 1;
}

// Replaces the 'original' value of the node with 'value' ; returns 1 if succesful; 0 otherwise
// WARNING!!!! The passed in value cannot be the same pointer to a previous value passed in; but must have same 'value' for comparator
int singlyLinkedList_replace(SinglyLinkedList *myLL, void *value) {
    SinglyNode *temp = myLL->head;
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

// Removes a specified SinglyNode given a value; returns 1 if successful; 0 otherwise
int singlyLinkedList_removeNode(SinglyLinkedList *myLL, void *value) {
    SinglyNode *temp = myLL->head;
    SinglyNode *prev = NULL;
    int index = 0;
    while(temp!=NULL) {
        if(myLL->compare(temp->value, value)==0) {
            if(index==0) return singlyLinkedList_removeFirst(myLL); // removing first node
            if(index==myLL->size-1) return singlyLinkedList_removeLast(myLL);  // removing last node
            SinglyNode *t2 = temp->next;
            prev->next = t2;
            myLL->freeValue(temp->value);
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

// Removing a node at a specified index; returns 1 if successfu; 0 otherwise
int singlyLinkedList_removeAtIndex(SinglyLinkedList *myLL, int index) {
    if(index > myLL->size) return 0;
    if(index == 0|| myLL->size==1) return singlyLinkedList_removeFirst(myLL);
    else if(index+1== myLL->size) return singlyLinkedList_removeLast(myLL);
    else { // removing value inbetween head and tail nodes
        SinglyNode *temp = myLL->head;
        SinglyNode *prev = NULL;
        for(int i = 0; i < index; i++) {
            if(temp==NULL) return 0; // shouldn't happen because of previous check but myLL.size could be changed on accident
            prev = temp;
            temp = temp->next;
        }
        prev->next = temp->next;
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
int singlyLinkedList_hasNext(void* dataStructure) {
    SinglyLinkedList *myLL = (SinglyLinkedList*) dataStructure;
    SinglyNode *temp = (SinglyNode*) myLL->enumerator.start;
    if(temp== NULL) return 0;
    else return 1;
}

// Returns the current value from the enumerator and sets it to the next value
void* singlyLinkedList_next(void* dataStructure) {
    SinglyLinkedList *myLL = (SinglyLinkedList*) dataStructure;
    SinglyNode *temp = (SinglyNode*) myLL->enumerator.start;
    myLL->enumerator.start = temp->next;
    return temp->value;
}

// Returns the current value from the enumerator
void* singlyLinkedList_peek(void* dataStructure) {
    SinglyLinkedList *myLL = (SinglyLinkedList*) dataStructure;
    SinglyNode *temp = (SinglyNode*) myLL->enumerator.start;
    return temp->value;
}

// Resets the Enumerator to start at the head of the linked list
void singlyLinkedList_resetEnumerator(SinglyLinkedList *myLL) {
    myLL->enumerator.start = myLL->head;
}

/*****
 * MergeSort Algorithm for Doubly Linked List Below Only-
 * Given an 'order' 1 for smallest to largest; 0 for largest to smallest
 * Based off of GeeksForGeeks implementation:
 *      https://www.geeksforgeeks.org/merge-sort-for-doubly-linked-list/
 * 
 ***/
static SinglyNode* singlyMergeSortMainHelper(SinglyLinkedList *myLL, SinglyNode* head, int order);
static SinglyNode* singlyMergeSortHelper(SinglyLinkedList *myLL, SinglyNode *l, SinglyNode *h, int order);
static SinglyNode* getMiddleSinglySortHelper(SinglyNode *start);

// Performs a MergeSort algorithm onto the linked list
// If order is 1 sorts from smallest to largest value based unto the given comaprator
// If order is 0 sorts from largest to smallest value based unto the given comaprator
void singlyLinkedList_mergeSort(SinglyLinkedList *myLL, int order) {
    if(myLL->isSorted && order==myLL->order) return; // no need to waste computations
    myLL->head = singlyMergeSortMainHelper(myLL, myLL->head, order);
    SinglyNode *temp = myLL->tail; // use this starting point as it is better than starting from head
    while(temp->next!=NULL) {
        temp = temp->next;
    }
    myLL->isSorted = 1;
    myLL->tail = temp;
    myLL->enumerator.start = myLL->head;
}

// The 'merge' function in mergesort
static SinglyNode* singlyMergeSortHelper(SinglyLinkedList *myLL, SinglyNode *l, SinglyNode *h, int order) {
     if(l==NULL) return h;
     if(h==NULL) return l;
     if(order) { // smallest to largest
        if(myLL->compare(l->value, h->value)==-1) { // l < h smallest to largest
            l->next = singlyMergeSortHelper(myLL, l->next, h, order);
            return l;
        }
        else {
            h->next = singlyMergeSortHelper(myLL, l, h->next, order);
            return h;
        }
     }
     else { // largest to smallest
          if(myLL->compare(l->value, h->value)==1) { // l > h // largest to smallest
            l->next = singlyMergeSortHelper(myLL, l->next, h, order);
            return l;
        }
        else {
            h->next = singlyMergeSortHelper(myLL, l, h->next, order);
            return h;
        }
     }
     
}

// Find the middle value from a start
static SinglyNode* getMiddleSinglySortHelper(SinglyNode *start) {
    SinglyNode *fast = start;
    SinglyNode *slow = start; 
    while (fast->next && fast->next->next) 
    { 
        fast = fast->next->next; 
        slow = slow->next; 
    } 
    SinglyNode *temp = slow->next; 
    slow->next = NULL; 
    return temp; 
}

// Main helper function
static SinglyNode* singlyMergeSortMainHelper(SinglyLinkedList *myLL, SinglyNode* head, int order) {
    if(head == NULL || head->next==NULL) return head;
    SinglyNode *middle = getMiddleSinglySortHelper(head);
    head = singlyMergeSortMainHelper(myLL, head, order);
    middle = singlyMergeSortMainHelper(myLL, middle, order);
    return(singlyMergeSortHelper(myLL, head, middle, order));
}

