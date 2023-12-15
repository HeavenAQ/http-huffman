#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdlib.h>

/**
 * checked_malloc - Allocate memory and check for errors.
 * @param count The number of elements to allocate.
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
extern void *must_calloc(size_t count, size_t size);

/**
 * read_file - Read a file and return its contents.
 * @param filename The name of the file to read.
 * @param filelen The length of the file.
 * @return A pointer to the contents of the file.
 */
extern char *read_file(const char *filename, size_t *filelen);

/**
 * write_data - Write data to a file.
 * @param filename The name of the file to write to.
 * @param mode The mode to open the file in.
 * @param data The data to write to the file.
 * @param data_len The length of the data to write to the file.
 */
extern void write_data(const char *filename, const char *mode, const char *data,
                       const size_t data_len);

/**
 * write_header - Write a headers to a file.
 * @param filename The name of the file to write to.
 * @param header The header to write to the file.
 * @param encoded_len The length of the encoded header.
 * @param raw_len The length of the raw data.
 * @param header_num The number of headers
 */
extern void write_header(const char *filename, const char **header,
                         const size_t encoded_len, const size_t raw_len,
                         const size_t header_num);

/**
 * print_header - Print encoded table to stdout.
 * @param header The header to print.
 * @param header_num The number of headers.
 */
extern void print_header(const char **header, size_t header_num);
#endif
