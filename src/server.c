#include "../include/server.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define TEMPLATE_SIZE 8192
#define BUFFER_SIZE 8192

/**
 * config_router - Configure all the endpoints for the server
 * @param self Server object
 */
static void config_router(Server *self)
{
    self->router->add_route(&self->router->root, "/", "index.html");
}

/**
 * handle_get_request - Handle GET request
 * @param server Server object
 * @param route Route to handle
 */
static char *handle_get_requests(Server *self, const char *route)
{
    self->logger->info_log("Handling GET request", __FILE__, __LINE__);
    struct Route *destination =
        self->router->search_route(self->router->root, route);
    char template[100] = "templates/";
    !destination ? strcat(template, "404.html")
                 : strcat(template, destination->value);
    return (char *)self->render_static_route(self, template);
}

/**
 * get_file_content - Get file content from POST request
 * @param self Server object
 * @param chunk http request to parse
 * @param content_len length of the returned content
 * @return File content
 */
static const char *get_file_content(Server *self, const char *const chunk,
                                    const size_t chunk_len, long *content_len)
{
    // get boundary
    self->logger->info_log("Getting file content", __FILE__, __LINE__);
    char boundary_header[] = "Content-Type: multipart/form-data; boundary=";
    char *boundary = strstr(chunk, boundary_header) + strlen(boundary_header);
    char *end_boundary = strstr(boundary, "\r\n");
    size_t boundary_len = (size_t)(end_boundary - boundary);
    char b[boundary_len];
    strncpy(b, boundary, boundary_len);

    // get file content
    char *start = strstr(chunk, b) + boundary_len + 2;
    start = strstr(start, b) + boundary_len + 2;
    start = strstr(start, "\r\n\r\n") + 4;
    char *end = strstr(start, b);

    // if there is no end boundary calculate the length by subtracting start of
    // form with the beginning of the chunk
    *content_len = (end == NULL) ? (long)chunk_len - (start - chunk)
                                 : end - start - 4; // 2 extra -- and  \r\n
    return start;
}

/** get_content_length - Get http request content length
 * @param client_req Client request
 * @param total_size Total size of the request (where to store the result)
 */
static void get_content_length(const char *client_req, ssize_t *total_size)
{
    int cur_line = 0;
    char *content_len = malloc(1000);
    for (int i = 0; i < 9; i++) {
        sscanf(client_req + cur_line, "%s", content_len);
        cur_line += strlen(content_len) + 2; // skip \n
        if (strcmp(content_len, "Content-Length:") == 0) {
            sscanf(client_req + cur_line - 2, "%s", content_len);
            *total_size = atoi(content_len);
            free(content_len);
            break;
        }
    }
}

/**
 * get_client_request
 * @param client_socket Client socket
 * @return Client request
 */
static void get_client_request(int client_socket, char **client_req,
                               size_t *req_len)
{
    ssize_t total_size = BUFFER_SIZE;
    ssize_t size_recv = 0;
    size_t ext_len = 1;
    while (1) {
        size_recv = recv(client_socket, *client_req + *req_len, BUFFER_SIZE,
                         MSG_DONTWAIT);

        // go ahead and respond to client if it is a GET request
        if (*req_len == 0) {
            char method[100];
            sscanf(*client_req, "%s", method);
            if (strcmp(method, "GET") == 0) {
                break;
            }
        }

        if (size_recv < 0) {
            continue;
        }

        *req_len += (size_t)size_recv;
        if (*req_len >= (BUFFER_SIZE * ext_len - 1)) {
            ext_len += 2;
            char *new_buffer = realloc(*client_req, BUFFER_SIZE * ext_len);
            *client_req = new_buffer;
        }

        get_content_length(*client_req, &total_size);
        if (*req_len >= (size_t)total_size) {
            break;
        }
    }
}

/**
 * parse_url_params - Parse URL parameters
 * @param self Server object
 * @param url URL to parse
 * @return Parsed URL parameters
 */

static void parse_url_params(Server *self __attribute__((unused)),
                             const char *url, char *out_file,
                             char *service_type)
{
    /*self->logger->info_log("Parsing URL parameters", __FILE__, __LINE__);*/
    char *start = strchr(url, '?');
    sscanf(start + 1, "out_file=%[^&]&service_type=%s", out_file, service_type);
}

/**
 * render_static_route - Render a static route
 * @param self Server object
 * @param endpoint Endpoint to render
 * @return Rendered template data
 */
static const char *render_static_route(Server *self, const char *endpoint)
{
    self->logger->info_log("Rendering...", __FILE__, __LINE__);
    FILE *file;
    char *buffer;

    file = fopen(endpoint, "r");
    if (file == NULL) {
        perror("Error opening file");
        self->logger->error_log("Error opening file", __FILE__, __LINE__);
    }

    // get filename
    fseek(file, 0, SEEK_END);
    size_t filelen = (size_t)ftell(file);
    rewind(file);

    // store data
    buffer = (char *)malloc((filelen + 1) * sizeof(char));
    fread(buffer, filelen, 1, file);
    fclose(file);
    return buffer;
}

/**
 * send_not_found_response - Send a 404 Not Found response
 * @param client_socket Client socket
 */
static void send_not_found_response(int client_socket)
{
    char http_header[TEMPLATE_SIZE] = "HTTP/1.1 404 Not Found\r\n\r\n";
    send(client_socket, http_header, sizeof(http_header), 0);
    close(client_socket);
}

/**
 * send_ok_response - Send a 200 OK response
 * @param client_socket Client socket
 * @param content_type Content type of the response
 * @param content_length Content length of the response
 */
static void send_ok_response(int client_socket, const char *response_data)
{
    char http_header[TEMPLATE_SIZE] = "HTTP/1.1 200 OK\r\n\r\n";
    strcat(http_header, response_data);
    strcat(http_header, "\r\n\r\n");
    send(client_socket, http_header, sizeof(http_header), 0);
    close(client_socket);
}

/**
 * init_server - Initialize server
 * @param self Server object
 * @param port Port to listen on
 */
void init_server(Server **self, int port)
{
    if ((*self = (Server *)malloc(sizeof(Server))) == NULL) {
        perror("Error allocating memory");
        exit(1);
    }

    (*self)->port = port;
    init_logger(&(*self)->logger);
    init_router(&(*self)->router);
    (*self)->config_router = &config_router;
    (*self)->render_static_route = &render_static_route;
    (*self)->send_ok_response = &send_ok_response;
    (*self)->send_not_found_response = &send_not_found_response;
    (*self)->handle_get_requests = &handle_get_requests;
    (*self)->get_file_content = &get_file_content;
    (*self)->parse_url_params = &parse_url_params;
    (*self)->get_client_request = &get_client_request;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind port
    if (bind(server_socket, (struct sockaddr *)&server_address,
             sizeof(server_address)) != 0) {
        (*self)->logger->error_log("Failed to bind socket", __FILE__, __LINE__);
        exit(1);
    } else {
        (*self)->logger->info_log("Socket bound", __FILE__, __LINE__);
    }

    char msg[100];
    snprintf(msg, 100, "Server listening on port %d", port);
    listen(server_socket, 5);
    (*self)->socket = server_socket;
    (*self)->logger->info_log(msg, __FILE__, __LINE__);
}
