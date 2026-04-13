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
        return;
    }

    if (strcmp(req->method, "POST") == 0) {
        if (strcmp(req->path, "/echo") == 0) {
            handle_post_echo(client_fd, req);
            return;
        }

        send_404(client_fd);
        return;
    }

    send_404(client_fd);
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

void handle_post_echo(int client_fd, http_request *req) {
    if (!req->body) {
        send_404(client_fd);
        return;
    }

    char header[256];

    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        req->content_length);

    send(client_fd, header, strlen(header), 0);
    send(client_fd, req->body, req->content_length, 0);
}