#ifndef HANDLER_H
#define HANDLER_H

#include "parser.h"

void handle_client(int client_fd);
void route_request(int client_fd, http_request *req);

#endif