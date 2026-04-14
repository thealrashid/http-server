#ifndef HANDLER_H
#define HANDLER_H

#include "parser.h"

void handle_client(int client_fd);
void route_request(int client_fd, http_request *req);
void handle_post_echo(int client_fd, http_request *req);
void send_simple_response(int client_fd, const char *body, size_t content_length);

#endif