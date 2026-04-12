/* HTTP parsing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "parser.h"

/* Read from socket */
int read_headers(int client_fd, char *buffer, size_t buffer_size, int *total) {
    int bytes;
    
    while (1) {
        if (*total >= (int)buffer_size - 1) {
            return -1; // headers too large
        }

        bytes = recv(client_fd, buffer + *total, buffer_size - *total - 1, 0);

        if (bytes <= 0) return -1;

        *total += bytes;
        buffer[*total] = '\0';

        if (strstr(buffer, "\r\n\r\n")) {
            break; // full headers received
        }
    }

    return 0;
}

/* Parse request line */
void parse_request_line(char *buffer, http_request *req) {
    sscanf(buffer, "%15s %255s", req->method, req->path);
    printf("Method: %s\n", req->method);
    printf("Path: %s\n", req->path);
}

/* Parse headers */
void parse_headers(char *buffer, http_request *req) {
    char *line_start = strstr(buffer, "\r\n");
    if (!line_start) return;

    line_start += 2;

    while (line_start) {
        char *line_end = strstr(line_start, "\r\n");
        if (!line_end) break;

        int len = line_end - line_start;

        char line[512];
        strncpy(line, line_start, len);
        line[len] = '\0';

        if (len == 0) break;

        printf("Header: %s\n", line);

        if (strncasecmp(line, "Content-Length:", 15) == 0) {
            req->content_length = atoi(line + 15);
        }

        line_start = line_end + 2;
    }
}

/* Allocate memory for body and copy from buffer */
int read_body(char *buffer, http_request *req, char *body_start, int client_fd, int total) {
    if (req->content_length > 0) {
        req->body = malloc(req->content_length);
        if (!req->body) {
            perror("malloc");
            return -1;
        }
    } else {
        req->body = NULL;
        return 0;
    }

    int header_len = body_start - buffer;
    int body_bytes_in_buffer = total - header_len;

    if (body_bytes_in_buffer > (int)req->content_length) {
        body_bytes_in_buffer = (int)req->content_length;
    }

    if (req->body && body_bytes_in_buffer > 0) {
        memcpy(req->body, body_start, body_bytes_in_buffer);
    }

    /* Read body if present */
    int remaining = req->content_length - body_bytes_in_buffer;
    int offset = body_bytes_in_buffer;

    while (remaining > 0) {
        int bytes = recv(client_fd, req->body + offset, remaining, 0);
        if (bytes <= 0) return -1;

        offset += bytes;
        remaining -= bytes;
    }

    return 0;
}

int parse_request(int client_fd, http_request *req) {
    char buffer[4096];
    int total = 0;

    req->content_length = 0;

    if (read_headers(client_fd, buffer, sizeof(buffer), &total) < 0) {
        printf("Error receiving bytes\n");
        return -1;
    }
    parse_request_line(buffer, req);

    /* Locate body start */
    char *body_start = strstr(buffer, "\r\n\r\n");
    if (!body_start) {
        printf("Invalid HTTP request\n");
        return -1;
    }
    body_start += 4; // skip "\r\n\r\n"

    parse_headers(buffer, req);

    if (req->content_length > MAX_BODY_SIZE) {
        printf("Content length is too large\n");

        char *response = "HTTP/1.1 413 Payload Too Large\r\n"
                            "Content-Type: text/plain\r\n"
                            "\r\n"
                            "Payload too large\n";
        
        send(client_fd, response, strlen(response), 0);
        return -1;
    }

    if (read_body(buffer, req, body_start, client_fd, total) < 0) {
        return -1;
    }

    return 0;
}