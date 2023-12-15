#ifndef _SERVER_H_
#define _SERVER_H_
#include "route.h"
typedef struct Server Server;
#include "../include/logger.h"
#include <stdlib.h>
struct Server {
    int port;
    int socket;
    Logger *logger;
    Router *router;
    void (*config_router)(Server *self);
    const char *(*render_static_route)(Server *self, const char *endpoint);
    void (*send_ok_response)(int client_socket, const char *body);
    void (*send_not_found_response)(int client_socket);
    void (*get_client_request)(int client_socket, char **client_req,
                               size_t *req_len);
    char *(*handle_get_requests)(Server *self, const char *route);
    const char *(*get_file_content)(Server *self, const char *const client_req,
                                    size_t *raw_data_len);
    void (*parse_url_params)(Server *self, const char *url, char *out_file,
                             char *service_type);
};
void init_server(Server **self, int port);
#endif
