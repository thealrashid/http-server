/* HTTP response */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "response.h"

void send_file_response(int client_fd, FILE *file, long file_size, const char *mime) {
    char header[512];

    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n",
        mime, file_size);

    send(client_fd, header, strlen(header), 0);

    char file_buffer[4096];
    size_t bytes_read;

    while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
        send(client_fd, file_buffer, bytes_read, 0);
    }
}

void send_response(int client_fd, 
                   int status_code, 
                   const char *status_text, 
                   const char *content_type, 
                   const void *body, 
                   size_t content_length) {
    char header[512];

    snprintf(header, sizeof(header),
            "HTTP/1.1 %d %s\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n",
            status_code, status_text,
            content_type,
            content_length);
    
    send(client_fd, header, strlen(header), 0);

    if (body && content_length > 0) {
        send(client_fd, body, content_length, 0);
    }
}

void send_ok(int client_fd, const char *body) {
    send_response(client_fd, 200, "OK", "text/plain", body, strlen(body));
}

void send_404(int client_fd) {
    const char *msg = "Not Found\n";

    send_response(client_fd, 404, "Not Found", "text/plain", msg, strlen(msg));
}

void send_500(int client_fd) {
    const char *msg = "Internal Server Error\n";
    send_response(client_fd, 500, "Internal Server Error", "text/plain", msg, strlen(msg));
}

void send_400(int client_fd) {
    const char *msg = "Unsupported Content-Type\n";
    send_response(client_fd, 400, "Unsupported Content-Type\n", "text/plain", msg, strlen(msg));
}

void send_403(int client_fd) {
    const char *msg = "Forbidden\n";
    send_response(client_fd, 403, "Forbidden", "text/plain", msg, strlen(msg));
}