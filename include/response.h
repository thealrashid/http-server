#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdio.h>

void send_file_response(int client_fd, FILE *file, long file_size, const char *mime);

void send_simple_response(int client_fd, const char *body, size_t content_length);

void send_response(int client_fd, int status_code, const char *status_text, const char *content_type, const void *body, 
                   size_t content_length);

void send_404(int client_fd);

void send_ok(int client_fd, const char *body);

void send_500(int client_fd);

void send_400(int client_fd);

#endif