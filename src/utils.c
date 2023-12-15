#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *must_calloc(size_t count, size_t nmemb)
{
    void *ptr = calloc(count, nmemb);
    if (!ptr) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

char *read_file(const char *filename, size_t *filelen)
{
    FILE *file;
    char *buffer;

    file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // get filename
    fseek(file, 0, SEEK_END);
    *filelen = (size_t)ftell(file);
    rewind(file);

    // store data
    buffer = (char *)malloc(*filelen * sizeof(char));
    fread(buffer, *filelen, 1, file);
    fclose(file);
    return buffer;
}

void write_data(const char *filename, const char *mode, const char *data,
                const size_t data_len)
{
    FILE *fd = fopen(filename, mode);
    for (size_t i = 0; i < data_len; i++)
        fputc(data[i], fd);
    fclose(fd);
}

void write_header(const char *filename, const char **header,
                  const size_t encoded_len, const size_t raw_len,
                  const size_t header_num)
{
    int header_len = 0;
    FILE *fd = fopen(filename, "wb");
    for (size_t i = 0; i < header_num; i++) {
        fprintf(fd, "%s\n", header[i]);
        header_len += strlen(header[i]) + 1;
    }
    int p_org_len = fprintf(fd, "Uncompressed Length: %zu\n", raw_len);
    int p_enc_len = fprintf(fd, "Compressed Length: %zu\n", encoded_len);
    fprintf(fd, "Compression Ratio: %f\n",
            (double)raw_len / (double)((int)encoded_len + header_len +
                                       p_org_len + p_enc_len + 29));
    fclose(fd);
}

void print_header(const char **header, size_t header_num)
{
    for (size_t i = 0; i < header_num; i++) {
        unsigned int byte;
        char code[257];
        sscanf(header[i], "%x=%s", &byte, code);
        printf("%c=%s\n", byte, code);
    }
}
