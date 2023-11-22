#include "../include/config.h"
#include "../include/utils.h"
#include <stdio.h>

int main(const int argc, const char **argv)
{
    autofree_config Config *config = new_config(argc, argv + 1);
    (config->server_mode) ? puts("Starting server mode")
                          : puts("Starting CLI mode");
    return 0;
}
