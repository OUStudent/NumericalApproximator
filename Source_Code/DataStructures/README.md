# Data Structures
The following folder consists of five main data structures: *AVLTree.h*, *DoublyLinkedList.h*, *HashTable.h*, *SinglyLinkedList.h*, and *Stack.h*. With implementation files in the respected folder.

## Description
These main five data structures were created as generic like templates for any possible data type, primitive or structural in nature. This was accomplished through the use of accepting void pointers, allowing any possible pointer type to be passed and processed. To achieve this goal, each data structure must be 'initialized' with pointer functions following proper procedure in freeing the data structures and comparing values. These pointer functions are similar to the *compare* function that must be user created for the standard **qsort()** function.

As with all of the main structures in this software, the contents of the structs are hidden behind the view of the user, forcing them to use functions in an object oriented way. To iterate over the contents of the structure, the user must use the enumerator functions provided, which all derive from the *Enumerator.h* file.
