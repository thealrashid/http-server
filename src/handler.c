/* Orchestrates request lifecycle */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "handler.h"
#include "parser.h"
#include "file.h"
#include "response.h"

typedef struct {
    char method[8];
    char path[64];
    void (*handler)(int, http_request *);
} route;

route routes[] = {
    {"POST", "/echo", handle_post_echo},
    {"POST", "/submit", handle_submit},
};

void route_request(int client_fd, http_request *req) {
    if (strcmp(req->method, "GET") == 0) {
        serve_static_file(client_fd, req->path);
        return;
    }

    int num_routes = sizeof(routes) / sizeof(routes[0]);

    for (int i = 0; i < num_routes; i++) {
        if (strcmp(req->method, routes[i].method) == 0 && 
            strcmp(req->path, routes[i].path) == 0) {

                routes[i].handler(client_fd, req);
                return;
            }
    }

    send_404(client_fd);
}

void handle_client(int client_fd) {
    http_request req = {0};

    if (parse_request(client_fd, &req) < 0) {
        close(client_fd);
        return;
    }

    printf("------Headers------\n");

    for (int i = 0; i < req.header_count; i++) {
        printf("%s: %s\n", req.headers[i].key, req.headers[i].value);
    }

    route_request(client_fd, &req);

    if (req.body) {
        free(req.body);
    }
}

void handle_post_echo(int client_fd, http_request *req) {
    if (!req->body) {
        send_404(client_fd);
        return;
    }

    const char *type = get_header(req, "Content-Type");

    if (type && strstr(type, "application/x-www-form-urlencoded")) {

    }

    form_field fields[10];

    int n = parse_form_data(req->body, fields, 10);

    printf("Parsed form data\n");

    for (int i = 0; i < n; i++) {
        printf("Key: %s, value: %s\n", fields[i].key, fields[i].value);
    }

    send_response(client_fd, 200, "OK", "text/plain", req->body, req->content_length); // echo raw body
}

void handle_submit(int client_fd, http_request *req) {
    const char *type = get_header(req, "Content-Type");

    if (!req->body) {
        send_response(client_fd,
                      400, "Bad Request",
                      "text/plain",
                      "Missing body\n",
                      strlen("Missing body\n"));
        return;
    }

    if (type && strstr(type, "application/x-www-form-urlencoded")) {
        form_field fields[10];

        int n = parse_form_data(req->body, fields, 10);

        printf("Parsed %d fields\n", n);

        for (int i = 0; i < n; i++) {
            printf("%s = %s\n", fields[i].key, fields[i].value);
        }

        send_ok(client_fd, "Form parsed\n");
        return;
    }
    
    send_response(client_fd,
                  415, "Unsupported Media Type",
                  "text/plain",
                  "Unsupported Content-Type\n",
                  strlen("Unsupported Content-Type\n"));
}