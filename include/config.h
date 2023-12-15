#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <stdbool.h>
#include <stdlib.h>
#define autofree_config __attribute__((cleanup(free_config)))

enum MODE { COMPRESS, DECOMPRESS };

typedef struct Config Config;
struct Config {
    const char *input_file;
    const char *output_file;
    enum MODE mode;
    bool using_server;
};

extern Config *new_config(const int argc, const char **argv);

__attribute__((always_inline)) inline void free_config(Config **config)
{
    free((*config));
}
#endif
