#include "../include/config.h"
#include "../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * check_arg - print message and exit if the condition is false.
 * -----------------------------------------------------------
 * @condition: The condition to check.
 * @message: The error message to print.
 *
 * Return: Nothing.
 */
inline static void check_arg(bool condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "Error: %s\n", message);
        exit(EXIT_FAILURE);
    }
}

/**
 * print_help - print the help message.
 * ------------------------------------
 *  Return: Nothing.
 */
static void print_help(void)
{
    printf("Usage: ./main [OPTIONS]\n");
    printf("Options:\n");
    printf("  -c, --compress        Compress the input file\n");
    printf("  -d, --decompress      Decompress the input file\n");
    printf("  -i, --input <file>    The input file\n");
    printf("  -o, --output <file>   The output file\n");
    printf("  -h, --help            Print this message\n");
    printf("  -s, --server          Run in server mode\n");
    exit(EXIT_SUCCESS);
}

/**
 * init_config - initialize the config object.
 * ------------------------------------------
 *  Return: A pointer to the config object.
 */

static Config *init_config(void)
{
    Config *config = (Config *)must_calloc(1, sizeof(Config));
    config->input_file = NULL;
    config->output_file = NULL;
    config->using_server = false;
    return config;
}

inline static void chk_config(Config *config)
{
    if (!config->using_server) {
        check_arg(config->input_file, "No input file");
        check_arg(config->output_file, "No output file");
    }
}

/**
 * new_config - create a new config object.
 * ----------------------------------------
 * @argc: The number of arguments.
 * @argv: The arguments.
 *
 * Return: A pointer to the new config object.
 */
Config *new_config(const int argc, const char *argv[])
{
    Config *config = init_config();
    for (int i = 0; i < argc; i++) {
        if (!argv[i]) {
            break;
        }

        bool is_mode = strcmp(argv[i], "-c") == 0 ||
                       strcmp(argv[i], "-d") == 0 ||
                       strcmp(argv[i], "--compress") == 0 ||
                       strcmp(argv[i], "--decompress") == 0;
        bool is_input =
            strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0;
        bool is_server =
            strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--server") == 0;
        bool is_help =
            strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0;
        bool is_output =
            strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0;

        if (is_mode) {
            bool is_compress = strcmp(argv[i], "-c") == 0 ||
                               strcmp(argv[i], "--compress") == 0;
            config->mode = is_compress ? COMPRESS : DECOMPRESS;
        } else if (is_input) {
            check_arg(argv[i + 1], "-i/--input requires a file name");
            config->input_file = argv[++i];
        } else if (is_output) {
            check_arg(argv[i + 1], "-o/--output requires a file name");
            config->output_file = argv[++i];
        } else if (is_help) {
            free_config(&config);
            print_help();
        } else if (is_server) {
            config->using_server = true;
        } else {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            free_config(&config);
            exit(EXIT_FAILURE);
        }
    }
    chk_config(config);
    return config;
}
