#ifndef _TREE_H
#define _TREE_H
#include "../include/node.h"
#include "logger.h"
#include <stdlib.h>

typedef struct HuffmanTree HuffmanTree;
struct HuffmanTree {
    Node *root;
    size_t size;
    Logger *logger;
    /**
     * Create an array of nodes from the given data
     * @param data The data to be stored in the node
     * @return The len of the array
     */
    void (*gen_freq_arr)(HuffmanTree *self, Node *freq_node_arr[],
                         const char data[], const size_t data_len);
    /**
     * Create a Huffman tree from the given data
     * @param self The Huffman tree
     * @param data The data to be stored in the tree
     * @param len The length of the data
     */
    void (*build_tree)(HuffmanTree *self, Node *arr[], const size_t len);

    /**
     * Encode the given data
     * @param self The Huffman tree
     * @param data The data to be encoded
     * @param code_table The code table to encode the data
     * @return the encoded data
     */
    char *(*encode)(HuffmanTree *self, const char *data,
                    const char **code_table, const size_t raw_len);
    /**
     * Decode the given data
     * @param self The Huffman tree
     * @param encoded_str The data to be decoded
     * @param decoded_len The length of the decoded data
     * @return the decoded data
     */
    char *(*decode)(HuffmanTree *self, char *encoded_str, size_t *decoded_len,
                    size_t raw_len);

    /**
     * Free the Huffman tree
     * @param self The Huffman tree
     */
    void (*destroy)(HuffmanTree **self);
    /**
     * Create a Huffman tree from the given data
     * @param self The Huffman tree
     * @param data The data to be stored in the tree
     * @param len The length of the data
     * @return the header of the encoded file
     */
    const char **(*cal_code_table)(HuffmanTree *self, Node *arr[]);
};
/**
 * init_huffman_tree - create a new Huffman tree object.
 * @return: A pointer to the new Huffman tree object.
 */
extern HuffmanTree *new_huffman_tree(void);
#endif
