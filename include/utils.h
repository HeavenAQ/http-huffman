#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdlib.h>

/**
 * checked_malloc - Allocate memory and check for errors.
 * @param size The size of the memory to allocate.
 * @return A pointer to the allocated memory.
 */
extern void *must_malloc(size_t size);

#endif
