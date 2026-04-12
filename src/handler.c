/* Orchestrates request lifecycle */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "handler.h"
#include "parser.h"
#include "file.h"
#include "response.h"

void route_request(int client_fd, http_request *req) {
    if (strcmp(req->method, "GET") == 0) {
        serve_static_file(client_fd, req->path);
    } else {
        send_404(client_fd);
    }
}

void handle_client(int client_fd) {
    http_request req = {0};

    if (parse_request(client_fd, &req) < 0) {
        close(client_fd);
        return;
    }

    route_request(client_fd, &req);

    if (req.body) {
        free(req.body);
    }
    
    close(client_fd);
}