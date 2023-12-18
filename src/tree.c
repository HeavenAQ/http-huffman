#include "../include/tree.h"
#include "../include/utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ALLOC_SIZE 256

/**
 * Swap two nodes
 * @param a The first node
 * @param b The second node
 * @param width The size of the node
 */
static void swap(void *a, void *b, size_t width)
{
    void *tmp = must_calloc(1, width);
    memcpy(tmp, a, width);
    memcpy(a, b, width);
    memcpy(b, tmp, width);
    free(tmp);
}

/**
 * Create an array of nodes from the given data
 * @param data The data to be stored in the node
 * @return The len of the array
 */
static void gen_freq_arr(HuffmanTree *self, Node *freq_node_arr[],
                         const char data[], const size_t data_len)
{
    self->logger->info_log("Reading file and generating frequency table",
                           __FILE__, __LINE__);
    char cur_datum = data[0];
    init_node_arr(freq_node_arr, data_len);

    size_t freq_arr_len = 0;
    for (size_t i = 0; i < data_len; cur_datum = data[++i]) {
        bool is_datum_in_arr = false;
        for (size_t j = 0; j < freq_arr_len; j++) {
            if (freq_node_arr[j]->data == cur_datum) {
                freq_node_arr[j]->freq++;
                is_datum_in_arr = true;
                break;
            }
        }
        if (!is_datum_in_arr) {
            freq_node_arr[freq_arr_len++] = create_node(cur_datum);
        }
    }
    self->size = freq_arr_len;
}

/**
 * Compare two nodes by their frequency if frequency is the same then compare
 * their data
 * @param a The first node
 * @param b The second node
 * @return The difference between the two nodes
 */
int compare_node(const void *a, const void *b)
{
    Node *node_a = *(Node **)a;
    Node *node_b = *(Node **)b;
    int freq_cmp = node_a->freq - node_b->freq;
    int data_cmp = node_a->data - node_b->data;
    return freq_cmp != 0 ? freq_cmp : data_cmp;
}

int compare_char(const void *a, const void *b)
{
    Node *node_a = *((Node **)a);
    Node *node_b = *((Node **)b);
    return node_a->data - node_b->data;
}

/**
 * Merge two nodes
 * @param a The first node
 * @param b The second node
 * @return The parent of the two nodes
 */
Node *merge_node(Node *a, Node *b)
{
    Node *newnode = create_node('\0');
    bool a_smaller = a->data < b->data;
    newnode->left = a_smaller ? a : b;
    newnode->right = a_smaller ? b : a;
    newnode->freq = a->freq + b->freq;
    newnode->data = newnode->left->data;
    a->p = newnode;
    b->p = newnode;
    return newnode;
}

/**
 * Create a Huffman tree from the given data
 * @param self The Huffman tree
 * @param data The data to be stored in the tree
 * @param len The length of the data
 */
static void build_tree(HuffmanTree *self, Node *arr[], const size_t len)
{
    self->logger->info_log("Building tree", __FILE__, __LINE__);
    if (self->size <= 0)
        return;

    Node **tmp = must_calloc(len, sizeof(Node *));
    memcpy(tmp, arr, sizeof(Node *) * len);

    size_t end = self->size;
    for (size_t i = 0; tmp[i] != NULL; i += 2) {
        qsort(tmp + i, end - i, sizeof(Node *), compare_node);
        if (tmp[i + 1] == NULL) {
            self->root = tmp[i];
            break;
        }
        tmp[end++] = merge_node(tmp[i], tmp[i + 1]);
    }
    self->logger->info_log("Tree built", __FILE__, __LINE__);
    free(tmp);
}

/**
 * Get the Huffman code
 * @param self the Huffman tree
 * @param node to be encoded
 */
static char *get_code(const HuffmanTree *self, Node *cur_node)
{
    if (!cur_node)
        return NULL;

    size_t i = 0, j = 1;
    char *code = must_calloc(ALLOC_SIZE, sizeof(char));
    while (cur_node != self->root) {
        if (i >= ALLOC_SIZE * j) {
            code = realloc(code, sizeof(char) * ALLOC_SIZE * ++j);
        }
        code[i++] = cur_node->p->left == cur_node ? '0' : '1';
        cur_node = cur_node->p;
    }
    code[i] = '\0';

    // reverse the code
    for (size_t i = 0, j = strlen(code) - 1; i < j; i++, j--)
        swap(&code[i], &code[j], sizeof(char));

    return code;
}

/**
 * Create a Huffman tree from the given data
 * @param self The Huffman tree
 * @param data The data to be stored in the tree
 * @param len The length of the data
 * @return the header of the encoded file
 */
static const char **cal_code_table(HuffmanTree *self, Node *data[])
{

    self->logger->info_log("Calculating code", __FILE__, __LINE__);
    char **header = must_calloc(sizeof(char *), self->size);
    qsort(data, self->size, sizeof(Node *), compare_char);

    for (size_t i = 0; i < self->size; i++) {
        char *code = get_code(self, data[i]);
        header[i] = must_calloc(ALLOC_SIZE, sizeof(char));
        snprintf(header[i], ALLOC_SIZE, "%x=%s", data[i]->data, code);
        free(code);
    }
    return (const char **)header;
}

/**
 * Get the Huffman code from raw data string
 * @param self The Huffman tree
 * @param header_str the raw data string
 * @return the header of the encoded file
 */
static const char **get_header(HuffmanTree *self, const char *encoded_str)
{
    self->logger->info_log("Extracting header", __FILE__, __LINE__);
    char **header = must_calloc(ALLOC_SIZE, sizeof(char *));
    int cur_idx = 0;

    char line[ALLOC_SIZE];
    while (sscanf(encoded_str + cur_idx, "%s", line)) {
        if (strncmp(line, "Uncompressed", 12) == 0) {
            self->size--;
            break;
        }

        size_t len = strlen(line);
        header[self->size] = must_calloc(len + 1, sizeof(char));
        strcpy(header[self->size++], line);
        cur_idx += len + 1;
    }
    self->size += 1;
    self->logger->info_log("Header extracted", __FILE__, __LINE__);
    return (const char **)header;
}

/**
 * Get the encoded data from raw encoded data string
 * @param self The Huffman tree
 * @param header_str the raw data string
 * @return the encoded data
 */
static const char *get_encoded_data(HuffmanTree *self, const char *encoded_str)
{
    self->logger->info_log("Extracting encoded data", __FILE__, __LINE__);
    int cur_idx = 0;
    char line[ALLOC_SIZE];

    while (sscanf(encoded_str + cur_idx, "%s", line) == 1) {
        cur_idx += strlen(line) + 1;
        if (strncmp(line, "Compression", 11) == 0) {
            break;
        }
    }

    self->logger->info_log("Encoded data extracted", __FILE__, __LINE__);
    return encoded_str + cur_idx + 16;
}

/**
 * Encode the given data
 * @param self The Huffman tree
 * @param data The data to be encoded
 * @param code_table The code table to encode the data
 * @return the encoded data
 */
static char *encode(HuffmanTree *self, const char *data,
                    const char **code_table, const size_t raw_len)
{
    size_t ext_len = 0;
    size_t encoded_data_len = 0;
    char *encoded_data = must_calloc(ALLOC_SIZE, sizeof(char));

    if (!encoded_data) {
        return NULL;
    }

    for (size_t i = 0; i < raw_len; ++i) {
        for (size_t j = 0; j < self->size; ++j) {
            unsigned int byte;
            char code[ALLOC_SIZE];
            if (sscanf(code_table[j], "%x=%s", &byte, code) != 2) {
                // Handle invalid code table entry
                free(encoded_data);
                return NULL;
            }

            if (data[i] == (char)byte) {
                // Ensure there is enough space in encoded_data
                size_t code_len = strlen(code);
                if (encoded_data_len + code_len >= ext_len * ALLOC_SIZE) {
                    uint8_t *temp = (uint8_t *)realloc(encoded_data,
                                                       ALLOC_SIZE * ++ext_len);
                    if (!temp) {
                        free(encoded_data);
                        return NULL;
                    }
                    encoded_data = (char *)temp;
                }
                memcpy(encoded_data + encoded_data_len, code, code_len);
                encoded_data_len += code_len;
                break;
            }
        }
    }

    // Null-terminate the encoded string
    if (encoded_data_len < ext_len * ALLOC_SIZE) {
        encoded_data[encoded_data_len] = '\0';
    } else {
        // Handle rare case where encoded_data is exactly full
        uint8_t *temp = (uint8_t *)realloc(encoded_data, encoded_data_len + 1);
        if (!temp) {
            free(encoded_data);
            return NULL;
        }
        temp[encoded_data_len] = '\0';
        encoded_data = (char *)temp;
    }
    return encoded_data;
}
/**
 * Build a Huffman tree from the given header
 * @param self The Huffman tree
 * @param header The header of the encoded file
 */
static size_t build_tree_from_header(HuffmanTree *self, const char **header)
{
    self->logger->info_log("Building tree from header", __FILE__, __LINE__);

    size_t decoded_len = 0;
    Node *cur_node = self->root = create_node('\0');

    for (size_t i = 0; i < self->size; i++) {
        unsigned int byte;
        char code[ALLOC_SIZE];
        sscanf(header[i], "%x=%s", &byte, code);

        for (size_t j = 0; j < strlen(code); j++) {
            if (code[j] == '0') {
                if (!cur_node->left) {
                    cur_node->left = create_node('\0');
                    cur_node->left->p = cur_node;
                }
                cur_node = cur_node->left;
            } else {
                if (!cur_node->right) {
                    cur_node->right = create_node('\0');
                    cur_node->right->p = cur_node;
                }
                cur_node = cur_node->right;
            }
        }
        cur_node->data = (char)byte;
        cur_node = self->root;
        decoded_len++;
    }
    self->logger->info_log("Tree built from header", __FILE__, __LINE__);
    return decoded_len;
}

/**
 * Decode the given data (helper function)
 * @param self The Huffman tree
 * @param encoded_data The encoded data
 *
 * @return the decoded data
 */
static char *_decode(const HuffmanTree *self, const char *encoded_data,
                     size_t *decoded_len, size_t encoded_len)
{
    self->logger->info_log("Decoding", __FILE__, __LINE__);
    if (!self->root) {
        self->logger->error_log("Tree is empty", __FILE__, __LINE__);
        return NULL;
    }

    // decode the data
    Node *cur_node = self->root;
    size_t i = 0, j = 1;
    char *decoded_data = must_calloc(ALLOC_SIZE, sizeof(char));

    while (i < encoded_len) {
        while (cur_node->left != NULL && cur_node->right != NULL) {
            cur_node =
                encoded_data[i++] == '0' ? cur_node->left : cur_node->right;
        }
        if (*decoded_len >= ALLOC_SIZE * j) {
            char *tmp = realloc(decoded_data, sizeof(char) * 256 * ++j);
            if (!tmp) {
                free(decoded_data);
                return NULL;
            }
            decoded_data = tmp;
        }
        decoded_data[(*decoded_len)++] = cur_node->data;
        cur_node = self->root;
    }

    self->logger->info_log("Decoded", __FILE__, __LINE__);
    return decoded_data;
}

/**
 * destroy header - free the header
 * @param header The header of the encoded file
 */
static void destroy_header(const char **header)
{
    for (size_t i = 0; i < ALLOC_SIZE; i++) {
        if (header[i] == NULL)
            break;
        free((void *)header[i]);
    }
    free(header);
}

/**
 * Decode the given data
 * @param self The Huffman tree
 *
 * @return the decoded data
 */
static char *decode(HuffmanTree *self, char *encoded_str, size_t *decoded_len,
                    size_t encoded_len)
{
    const char **header = get_header(self, encoded_str);
    const char *encoded_data = get_encoded_data(self, encoded_str);
    encoded_len -= (size_t)(encoded_data - encoded_str);
    build_tree_from_header(self, header);
    destroy_header(header);
    return _decode(self, encoded_data, decoded_len, encoded_len);
}

/**
 * Free the Huffman tree node
 * @param self The Huffman tree node
 */
static void destroy_node(Node **self)
{
    if (!self || !*self)
        return;

    destroy_node(&(*self)->left);
    destroy_node(&(*self)->right);
    free(*self);
    *self = NULL;
}

/**
 * Free the Huffman tree
 * @param self The Huffman tree
 */
static void destroy(HuffmanTree **self)
{
    if (!self || !*self)
        return;

    destroy_node(&(*self)->root);
}

/**
 * init_huffman_tree - create a new Huffman tree object.
 * @return: A pointer to the new Huffman tree object.
 */
HuffmanTree *new_huffman_tree(void)
{
    HuffmanTree *self = must_calloc(1, sizeof(HuffmanTree));
    self->root = NULL;
    self->size = 0;
    self->gen_freq_arr = &gen_freq_arr;
    self->build_tree = &build_tree;
    self->cal_code_table = &cal_code_table;
    self->destroy = &destroy;
    self->encode = &encode;
    self->decode = &decode;
    init_logger(&self->logger);
    self->logger->info_log("Huffman tree initialized", __FILE__, __LINE__);
    return self;
}
