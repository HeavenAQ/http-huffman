#ifndef _NODE_H_
#define _NODE_H_
#include <stdint.h>
typedef struct Node Node;
#include <stdlib.h>

struct Node {
    char data;
    int freq;
    Node *p;
    Node *left;
    Node *right;
};
Node *create_node(char data);
void init_node_arr(Node **arr, const size_t len);
#endif
