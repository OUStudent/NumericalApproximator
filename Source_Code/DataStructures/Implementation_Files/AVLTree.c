// Implementation file for AVLTree.h

#include "../AVLTree.h"

/***
 * 
 * Data Structures Declarations only Below:
 * 
 * ***/ 






struct Order {
    int type; // 0 for inorder, 1 for preorder, 2 for postorder
};
typedef struct Order Order;

struct TreeNode {
    int diff;
    void *value;
    struct TreeNode *left;
    struct TreeNode *right;
};
typedef struct TreeNode TreeNode;

struct AVLTree {
    int stackFilled;
    TreeNode *root;
    Stack *traversalStack;
    Order order;
    unsigned long size;
    Enumerator enumerator;
    int (*compare)(const void*, const void*); // for l & r: return -1 if l < r, 0 if l==r, 1 if l > r
    void (*freeValue) (void *); // User defined function for freeing value of node
};

/***
 * 
 * Static Function Definitions only Below:
 * 
 * ***/

static void avltree_zigzag(TreeNode *node);
static void avltree_zagzig(TreeNode *node);
static void avltree_zig(TreeNode *node);
static void avltree_zag(TreeNode *node);
static int avltree_replaceHelper(AVLTree *tree, TreeNode *node, void* value);
static void avltree_rebalance(TreeNode *node);
static int avltree_maxDepth(TreeNode* node);
static int avltree_maxDepthRight(TreeNode *node);
static int avltree_maxDepthLeft(TreeNode *node);
static void* avltree_findHelper(AVLTree *tree, TreeNode *node, void* value);
static void avltreeDestructorHelper(AVLTree* myTree, TreeNode *node);
static int avltree_containsHelper(AVLTree *tree, TreeNode *node, void* value);
static int avltree_addHelper(AVLTree *tree, TreeNode *node, void* value);

/****
 * 
 * Function declarations and implementations below:
 * 
 * ***/

// Returns 1 if the value was added; 0 otherwise
int avltree_add(AVLTree *tree, void* value) {
    tree->stackFilled = 0; // need to reset traversalStack if hasNext is called
    return avltree_addHelper(tree, tree->root, value);
}

// Helper function for 'add'
static int avltree_addHelper(AVLTree *tree, TreeNode *node, void* value) {
    if(node->value==NULL) {
        node->value = value;
        if(tree->size==0) tree->root = node;
        tree->size++;
        node->diff = 0;
        return 1;
    }
    int isInserted = 1;
    if(tree->compare(value, node->value)==-1) { // value < node value
        int oldDiff;
        if(node->left ==NULL) {
            oldDiff = 0;
            node->left = (TreeNode*) malloc(sizeof(TreeNode));
            node->left->value = NULL;
            node->left->left =NULL;
            node->left->right = NULL;
            node->diff--;
        }
        else oldDiff = node->left->diff;
        isInserted = avltree_addHelper(tree, node->left, value);
        if(oldDiff != node->left->diff && node->left->diff !=0) node->diff--; // update diff if previous diff was changed and right dne 0
        avltree_rebalance(node);
    }
    else if(tree->compare(value, node->value)==1) { // value > node value
         int oldDiff;
        if(node->right ==NULL) {
            oldDiff = 0;
            node->right = (TreeNode*) malloc(sizeof(TreeNode));
            node->right->value = NULL;
            node->right->left =NULL;
            node->right->right = NULL;
            node->diff++;
        }
        else oldDiff = node->right->diff;
        isInserted = avltree_addHelper(tree, node->right, value);
        if(oldDiff != node->right->diff && node->right->diff !=0) node->diff++; // update diff if previous diff was changed and right dne 0
       avltree_rebalance(node);
    }
    else return 0; // value == node value therefore tree already contas the data
    return isInserted;
    
}

// Returns 1 if the value is contained within the tree; 0 otherwise
int avltree_contains(AVLTree *tree, void* value) {
    if(tree->size ==0) return 0;
    return avltree_containsHelper(tree, tree->root, value);
}

// Helper function for 'contains'
static int avltree_containsHelper(AVLTree *tree, TreeNode *node, void* value) {
    if(node==NULL) return 0;
    if(tree->compare(value, node->value)==-1) { // value < node value
        if(node->left==NULL) return 0; // reached leaf node
        return avltree_containsHelper(tree, node->left, value);
    }
    else if(tree->compare(value, node->value)==1) { // value > node value
        if(node->right==NULL) return 0; // reached leaf node
        return avltree_containsHelper(tree, node->right, value);
    }
    else return 1; // match
}

// destructor
void avltree_destructor(AVLTree** myTree) {
    //stack_destructor(myTree->traversalStack);
    stack_freeNodes((*myTree)->traversalStack);
   avltreeDestructorHelper((*myTree), (*myTree)->root);
   // avltree_resetEnumerator(myTree);
   // stack_destructor(myTree->traversalStack);
    (*myTree)->size = 0;
    free((*myTree)->traversalStack);
    free(*myTree);
}

// destructor helper
static void avltreeDestructorHelper(AVLTree* myTree, TreeNode *node) {
    if(node==NULL) return;
    if(node->value==NULL) {
        free(node);
        return; 
    }
    myTree->freeValue(node->value);
    avltreeDestructorHelper(myTree, node->left);
    avltreeDestructorHelper(myTree, node->right);
    free(node);
}

// returns null if not found; if found, returns the value in the tree
void* avltree_find(AVLTree *tree, void* value) {
    if(tree->size ==0) return NULL;
    return avltree_findHelper(tree, tree->root, value);
}

// Helper function for find
static void* avltree_findHelper(AVLTree *tree, TreeNode *node, void* value) {
    if(node==NULL) return NULL;
    if(tree->compare(value, node->value)==-1) { // value < node value
        if(node->left==NULL) return NULL; // reached leaf node
        return avltree_findHelper(tree, node->left, value);
    }
    else if(tree->compare(value, node->value)==1) { // value > node value
        if(node->right==NULL) return NULL; // reached leaf node
        return avltree_findHelper(tree, node->right, value);
    }
    else return node->value; // match
}

// Initializer
void avltree_initialize(AVLTree **tree, int (*compare)(const void*, const void*), void (*freeValue) (void *), int order) {
    *tree = (AVLTree*) malloc(sizeof(AVLTree));
    (*tree)->size = 0;
    (*tree)->stackFilled = 0;
    (*tree)->root = (TreeNode*) malloc(sizeof(TreeNode));
    (*tree)->root->value = NULL;
     (*tree)->root->right = NULL;
      (*tree)->root->left = NULL;
    (*tree)->compare = compare;
    (*tree)->freeValue = freeValue;
    (*tree)->order.type = order;
    (*tree)->enumerator.hasNext = avltree_hasNext;
    (*tree)->enumerator.next = avltree_next;
    (*tree)->enumerator.peek = avltree_peek;
    (*tree)->enumerator.start = NULL;
    (*tree)->traversalStack;
    stack_initialize(&((*tree)->traversalStack), compare, freeValue);
}

// calculates the max depth on the left side of the node
static int avltree_maxDepthLeft(TreeNode *node) {
    if(node==NULL || node->value==NULL) return 0;
    return avltree_maxDepthLeft(node->left) + 1;
}

// Calculates the max detph on the right side of the node
static int avltree_maxDepthRight(TreeNode *node) {
    if(node==NULL || node->value==NULL) return 0;
    return avltree_maxDepthRight(node->right) + 1;
}

// Returns the max depth from either the left or right side of the node
static int avltree_maxDepth(TreeNode* node) {
    if(node==NULL || node->value==NULL) return 0;
    int lDepth = avltree_maxDepthLeft(node->left);
    int rDepth = avltree_maxDepthRight(node->right);
    if(lDepth > rDepth) {
        return(lDepth+1);
    }
    else return(rDepth+1);

}

// Returns 1 if the given value was removed succesfully; 0 otherwise
static int avltree_removeHelper(AVLTree *tree, TreeNode **node, void* value) {
    int isDeleted;
    if(*node==NULL || (*node)->value == NULL) return 0; 
    if(tree->compare(value, (*node)->value)==-1) { // value < node
        isDeleted = avltree_removeHelper(tree, &(*node)->left, value);
        if((*node)->left==NULL) {
          //  free((*node)->left);
          //  (*node)->left->value = NULL;
        }
        (*node)->diff = avltree_maxDepth((*node)->right) - avltree_maxDepth((*node)->left);
        avltree_rebalance((*node));
    }
    else if(tree->compare(value, (*node)->value)==1) { // value > (*node)
        isDeleted = avltree_removeHelper(tree, &(*node)->right, value);
        if((*node)->right == NULL) {
          //  free((*node)->right);
           // (*node)->right->value = NULL;
        }
        (*node)->diff = avltree_maxDepth((*node)->right) - avltree_maxDepth((*node)->left);
        avltree_rebalance((*node));
    }
    else { // match
        tree->stackFilled = 0; // need to reset traversalStack if hasNext is called
        tree->size--;
        // Case 1: No children
        if((*node)->left==NULL && (*node)->right==NULL) {
            if(tree->compare((*node)->value, tree->root->value)==0) {// Removing root value
                 tree->freeValue((*node)->value); 
                 (*node)->value = NULL;
                 (*node)->diff = 0;
                // want to leave root value not NULL
            }
            else {
                tree->freeValue((*node)->value); 
                (*node)->value = NULL;
                free((*node));
                (*node)=NULL;
            }
        }
        // Case 2: Left Child only
        else if((*node)->left !=NULL && (*node)->right==NULL) {
            tree->freeValue((*node)->value);
            (*node)->value = (*node)->left->value;
            free((*node)->left);
            (*node)->left = NULL;
            (*node)->diff = avltree_maxDepth((*node)->right) - avltree_maxDepth((*node)->left);
            avltree_rebalance((*node));
        }
        // Case 3: Has a Right Child
        else {
            // right cild has no left child
            if((*node)->right->left==NULL) {
                tree->freeValue((*node)->value);
                (*node)->value = (*node)->right->value;
                TreeNode *temp = (*node)->right->right;
                free((*node)->right);
                (*node)->right = temp;
            }
            else { // left child exists
                TreeNode *temp = (*node)->right->left;
                TreeNode *temp2 = (*node)->right;
                while(temp->left!=NULL) { // while left child is not empty
                    temp = temp->left;
                    temp2 = temp2->left;
                }
                tree->freeValue((*node)->value);
                (*node)->value = temp->value;
                temp2->left = NULL;
                free(temp);
            }
            (*node)->diff = avltree_maxDepth((*node)->right) - avltree_maxDepth((*node)->left);
            avltree_rebalance((*node));
        }
        isDeleted = 1;
    }
    return isDeleted;
}

// Returns 1 if the given value was removed succesfully; 0 otherwise
int avltree_remove(AVLTree *tree, void* value) {
    return avltree_removeHelper(tree, &tree->root, value);
}

// Checks to see if the node needs to be rebalanced
static void avltree_rebalance(TreeNode *node) {
    if(node ==NULL || node->value==NULL) return;
    if ((node->diff >= -1) && (node->diff <= 1)) return; // diff is between -1 < diff < 1
	if ((node->diff < 0) && node->left !=NULL && (node->left->diff <= 0)) avltree_zig(node); // right rotation
	if ((node->diff < 0) && node->left !=NULL &&(node->left->diff > 0)) avltree_zigzag(node); // right-left
	if ((node->diff > 0) && node->right !=NULL &&(node->right->diff < 0)) avltree_zagzig(node); // left-right
	if ((node->diff > 0) && node->right !=NULL &&(node->right->diff >= 0)) avltree_zag(node); // left rotation
}

// Returns 1 if the given value has been replaced; 0 otherwise
int avltree_replace(AVLTree *tree, void* value) {
    if(tree->size ==0) return 0;
    return avltree_replaceHelper(tree, tree->root, value);
}

// Helper function for rebalance
static int avltree_replaceHelper(AVLTree *tree, TreeNode *node, void* value) {
    if(node->value==NULL) return 0;
    if(tree->compare(value, node->value)==-1) { // value < node value
        if(node->left==NULL) return 0; // reached leaf node
        return avltree_replaceHelper(tree, node->left, value);
    }
    else if(tree->compare(value, node->value)==1) { // value > node value
        if(node->right==NULL) return 0; // reached leaf node
        return avltree_replaceHelper(tree, node->right, value);
    }
    else { // match
        tree->freeValue(node->value);
        node->value = value;
    }
}

// Sets the order in which to enumerate: 0 for inorder, 1 for preorder, 2 for postorder
void avltree_setOrder(AVLTree *tree, int order) {
    tree->order.type = order; // 0 for inorder, 1 for preorder, 2 for postorder
    avltree_resetEnumerator(tree);
}

// Left rotation on Node
static void avltree_zag(TreeNode *node) { // left 
    if(node->right==NULL) return;
    int gdiff = node->diff;
    int pdiff = node->right->diff;

    TreeNode *ornl = node->right;
    node->right = ornl->right;
    ornl->right = ornl->left;
    ornl->left = node->left;
    node->left = ornl;

    void *tempValue = node->value;
    node->value = ornl->value;
    ornl->value = tempValue;
    if(pdiff > 0) {
        node->diff = gdiff -2;
        node->left->diff = gdiff-pdiff -1;
    }
    else {
        node->diff = pdiff - 1;
        node->left->diff =gdiff-1;
    }
}

// Right rotation on Node
static void avltree_zig(TreeNode *node) {
    if(node->left ==NULL) return;
    int gdiff = node->diff;
    int pdiff = node->left->diff;

    TreeNode *ornl = node->left;
    node->left = ornl->left;
    ornl->left = ornl->right;
    ornl->right = node->right;
    node->right = ornl;

    void *tempValue = node->value;
    node->value = ornl->value;
    ornl->value = tempValue;
    if(pdiff < 0) {
        node->diff = gdiff + 2;
        node->right->diff = gdiff - pdiff + 1;
    }
    else {
        node->diff = pdiff + 1;
        node->right->diff = gdiff + 1;
    } 
}

// Right - Left rotation
static void avltree_zagzig(TreeNode *node) {
    avltree_zig(node->right);
    avltree_zag(node);
}

// Left - Right rotation
static void avltree_zigzag(TreeNode *node) {
    avltree_zag(node->left);
    avltree_zig(node);
}


/***
 * 
 * Enumerator Functions only Below:
 * 
 * ***/

// Fills the traversal stack with values
static void avltree_fillTraversalStack(AVLTree *myTree, TreeNode *node) {
    // 0 for inorder, 1 for preorder, 2 for postorder
    if(myTree->order.type==0) { // inorder
        if(node==NULL || node->value == NULL) return;
        avltree_fillTraversalStack(myTree, node->right);
        stack_push(myTree->traversalStack, node->value);
        avltree_fillTraversalStack(myTree, node->left);
    }
    else if(myTree->order.type==1) { // preorder
        if(node==NULL || node->value == NULL) return;
        avltree_fillTraversalStack(myTree, node->right);
        avltree_fillTraversalStack(myTree, node->left);
        stack_push(myTree->traversalStack, node->value);
    }
    else if(myTree->order.type==2) { // postorder
        if(node==NULL || node->value == NULL) return;
        stack_push(myTree->traversalStack, node->value);
        avltree_fillTraversalStack(myTree, node->right);
        avltree_fillTraversalStack(myTree, node->left);
    }
}

// If the enumerator has a next value, return 1; otherwise 0
int avltree_hasNext(void* dataStructure) {
    AVLTree *myTree = (AVLTree*) dataStructure;
    if(myTree->stackFilled==0) {
        avltree_fillTraversalStack(myTree, myTree->root);
        myTree->stackFilled=1;
    }
    return stack_hasNext(myTree->traversalStack);
}

// Returns the current value from the enumerator
void* avltree_peek(void* dataStructure) {
    AVLTree *myTree = (AVLTree*) dataStructure;
    return stack_peek(myTree->traversalStack);
}

// Returns the current value from the enumerator and sets it to the next value
void* avltree_next(void* dataStructure) {
    AVLTree *myTree = (AVLTree*) dataStructure;
   
    return stack_next(myTree->traversalStack);
}

// Resets the Enumerator
void avltree_resetEnumerator(AVLTree *myTree) {
    stack_freeNodes(myTree->traversalStack);
    free(myTree->traversalStack);
    myTree->traversalStack;
    stack_initialize(&myTree->traversalStack, myTree->compare, myTree->freeValue);
    stack_resetEnumerator(myTree->traversalStack);
    myTree->stackFilled = 0;
}
