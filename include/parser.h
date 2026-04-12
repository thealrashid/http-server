#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

typedef struct {
    char method[16];
    char path[256];
    size_t content_length;
    char *body;
} http_request;

#define MAX_BODY_SIZE 1024

int parse_request(int client_fd, http_request *req);
int read_headers(int client_fd, char *buffer, size_t buffer_size, int *total);
void parse_request_line(char *buffer, http_request *req);
void parse_headers(char *buffer, http_request *req);
int read_body(char *buffer, http_request *req, char *body_start, int client_fd, int total);

#endif