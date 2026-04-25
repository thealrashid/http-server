#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>

#define MAX_BODY_SIZE 1024

#define MAX_HEADERS 20

typedef struct {
    char key[64];
    char value[256];
} http_header;

typedef struct {
    char method[16];
    char path[256];

    size_t content_length;
    char *body;

    http_header headers[MAX_HEADERS];
    int header_count;
} http_request;

typedef struct {
    char key[64];
    char value[256];
} form_field;

int parse_request(int client_fd, http_request *req);

int read_headers(int client_fd, char *buffer, size_t buffer_size, int *total);

void parse_request_line(char *buffer, http_request *req);

void parse_headers(char *buffer, http_request *req);

int read_body(char *buffer, http_request *req, char *body_start, int client_fd, int total);

int parse_form_data(const char *body, form_field *fields, int max_fields);

void url_decode(char *src, char *dest);

const char *get_header(http_request *req, const char *key);

#endif