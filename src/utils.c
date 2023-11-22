#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>

void *must_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}
