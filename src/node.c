#include "../include/node.h"
#include "../include/utils.h"

/**
 * Create a new node with the given data
 * @param data The data to be stored in the node
 * @return The new node
 */
Node *create_node(char data)
{
    Node *newnode = must_calloc(1, sizeof(Node));
    newnode->data = data;
    newnode->freq = 1;
    newnode->p = 0;
    newnode->left = NULL;
    newnode->right = NULL;
    return newnode;
}

/**
 * Initialize an array of nodes to NULL
 * @param arr The array of nodes
 * @param len The length of the array
 */
void init_node_arr(Node **arr, const size_t len)
{
    for (size_t i = 0; i < len; i++)
        arr[i] = NULL;
}
