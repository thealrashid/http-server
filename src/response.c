/* HTTP response */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "response.h"

void send_404(int client_fd) {
    const char *response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "File not found\n";

    send(client_fd, response, strlen(response), 0);
}

void send_file_response(int client_fd, FILE *file, long file_size, const char *mime) {
    char header[256];

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