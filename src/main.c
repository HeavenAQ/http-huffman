#include "../include/config.h"
#include "../include/node.h"
#include "../include/server.h"
#include "../include/tree.h"
#include "../include/utils.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8000
#define BUFFER_SIZE 8192
#define MAX_CLIENT_MSG_SIZE 4096
#define MAX_HEADER_LINE_SIZE 1024

void compress(HuffmanTree *tree, const char *const output_file, char *raw_data,
              size_t raw_len)
{
    // read file and generate frequency array
    tree->logger->info_log("Start compressing", __FILE__, __LINE__);
    Node **tree_node_arr = must_calloc(raw_len, sizeof(Node *));

    // build tree and calculate code table
    tree->gen_freq_arr(tree, tree_node_arr, raw_data, raw_len);
    tree->build_tree(tree, tree_node_arr, raw_len);

    const char **code_table = tree->cal_code_table(tree, tree_node_arr);
    char *encoded_data = tree->encode(tree, raw_data, code_table, raw_len);
    tree->logger->info_log("Done compressing", __FILE__, __LINE__);

    // write encoded header and encoded file to output file
    size_t encoded_len = strlen(encoded_data);
    write_header(output_file, code_table, encoded_len, raw_len, tree->size);
    write_data(output_file, "ab", encoded_data, encoded_len);

    // print header
    tree->logger->info_log("Done", __FILE__, __LINE__);

    // clean up
    free(encoded_data);
    for (size_t i = 0; i < tree->size; i++)
        free((void *)code_table[i]);
}

void decompress(HuffmanTree *tree, const char *const output_file,
                char *raw_data, const size_t raw_len)
{
    tree->logger->info_log("Start decompressing", __FILE__, __LINE__);
    size_t decoded_len = 0;
    char *decoded_data = tree->decode(tree, raw_data, &decoded_len, raw_len);
    write_data(output_file, "wb", decoded_data, decoded_len);
    tree->logger->info_log("Done decompressing", __FILE__, __LINE__);
}

/**
 * cli_mode - run in cli mode
 * @config: The config object
 */
static void cli_mode(Config *config)
{
    HuffmanTree *tree = new_huffman_tree();
    size_t raw_data_len = 0;
    char *raw_data = read_file(config->input_file, &raw_data_len);
    tree->logger->info_log("Starting CLI mode", __FILE__, __LINE__);
    (config->mode == COMPRESS)
        ? compress(tree, config->output_file, raw_data, raw_data_len)
        : decompress(tree, config->output_file, raw_data, raw_data_len);
    tree->destroy(&tree);
    free(tree);
}

/**
 * handle_upload - Handle file upload (Compress or Decompress)
 * @param server Server object
 * @param client_socket Client socket
 */
static void handle_upload(Server *server, const char *const chunk,
                          size_t chunk_len)
{
    char output_file[100];
    char service_type[100];
    HuffmanTree *tree = new_huffman_tree();
    server->logger->info_log("Handling upload request", __FILE__, __LINE__);
    server->logger->info_log("Parsing url params", __FILE__, __LINE__);
    server->parse_url_params(server, chunk, output_file, service_type);

    // get file content and length
    long len = 0;
    char *content =
        (char *)server->get_file_content(server, chunk, chunk_len, &len);

    // compress or decompress the file
    char path[100] = "downloads/";
    strcat(path, output_file);
    (strcmp(service_type, "compress") == 0)
        ? compress(tree, path, content, (size_t)len)
        : decompress(tree, path, content, (size_t)len);

    tree->destroy(&tree);
}

/**
 * handle_download - Handle file download (Compress or Decompress)
 * @param server Server object
 * @param url URL
 * @param client_socket Client socket
 */
static void handle_download(Server *server, const char *const url,
                            int client_socket)
{
    server->logger->info_log("Handling download request", __FILE__, __LINE__);
    char output_file[100];
    sscanf(url, "/download?out_file=%s", output_file);
    char download_path[100] = "downloads/";
    strcat(download_path, output_file);

    server->logger->info_log("Opening file", __FILE__, __LINE__);
    server->logger->info_log(download_path, __FILE__, __LINE__);
    FILE *file = fopen(download_path, "rb");
    if (file == NULL) {
        server->logger->error_log("File not found", __FILE__, __LINE__);
        server->send_not_found_response(client_socket);
        return;
    }

    // get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    // Send HTTP Headers
    server->logger->info_log("Sending HTTP Headers", __FILE__, __LINE__);
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
             "HTTP/1.1 200 OK\r\n"
             "Access-Control-Expose-Headers: Content-Disposition\r\n"
             "Content-Type: application/octet-stream\r\n"
             "Content-Disposition: attachment; filename=\"%s\"\r\n"
             "Content-Length: %ld\r\n"
             "\r\n",
             output_file, file_size);
    send(client_socket, buffer, strlen(buffer), 0);

    // Send the file content
    server->logger->info_log("Sending file content", __FILE__, __LINE__);
    while (!feof(file)) {
        size_t read = fread(buffer, 1, sizeof(buffer), file);
        send(client_socket, buffer, read, 0);
    }

    fclose(file);
}

/**
 * handle_client_request - Handle client request
 * @param server Server object
 * @param client_socket Client socket
 */
static void handle_client_request(Server *server, int client_socket)
{
    /*server->logger->info_log("Handling client request", __FILE__, __LINE__);*/
    size_t req_len = 0;
    char *chunk = (char *)must_calloc(INT_MAX, sizeof(char));
    server->get_client_request(client_socket, &chunk, &req_len);

    // parsing client socket header to get HTTP method, route
    char method[100];
    char route[100];
    sscanf(chunk, "%s %s HTTP/1.1", method, route);

    // render static file
    if (strcmp(method, "GET") == 0) {
        if (strncmp(route, "/download", 9) == 0) {
            handle_download(server, route, client_socket);
        } else {
            char *response_data = server->handle_get_requests(server, route);
            server->send_ok_response(client_socket, response_data);
            free(response_data);
        }
    } else if (strcmp(method, "POST") == 0) {
        if (strncmp(route, "/upload", 7) == 0) {
            handle_upload(server, chunk, req_len);
        } else {
            server->send_not_found_response(client_socket);
        }
        server->send_ok_response(client_socket, "Done");
    } else {
        server->send_not_found_response(client_socket);
    }
    free(chunk);
}

/**
 * server_mode - run in server mode
 * @config: The config object
 */
static void server_mode(Config *config __attribute__((unused)))
{
    // setup server
    Server *server;
    init_server(&server, 8000);
    server->logger->info_log("Starting server mode", __FILE__, __LINE__);
    server->config_router(server);

    // list routes
    server->router->list_routes(server->router->root);
    while (1) {
        int client_socket = accept(server->socket, NULL, NULL);
        if (client_socket == -1) {
            continue;
        }

        // fork a child process to handle client request
        pid_t pid = fork();
        if (pid == 0) {
            // child process
            close(server->socket);
            handle_client_request(server, client_socket);
            close(client_socket);
            exit(0);
        } else {
            close(client_socket);
        }
    }
}

/**
 * main - the entry point of the program
 * @argc: The number of arguments
 * @argv: The array of arguments
 *
 * Return: 0 on success, 1 otherwise
 */
int main(const int argc, const char **argv)
{
    typedef void (*mode_func)(Config *);
    autofree_config Config *config = new_config(argc, argv + 1);
    mode_func mode_funcs[] = {cli_mode, server_mode};
    mode_funcs[config->using_server](config);
    return 0;
}
