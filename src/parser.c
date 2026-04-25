/* HTTP parsing */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#include "parser.h"
#include "response.h"

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
    req->header_count = 0;

    while (line_start && req->header_count < MAX_HEADERS) {
        char *line_end = strstr(line_start, "\r\n");
        if (!line_end) break;

        int len = line_end - line_start;

        if (len == 0) break; // end of headers

        char line[512];
        strncpy(line, line_start, len);
        line[len] = '\0';

        if (len == 0) break;

        printf("Header: %s\n", line);

        char *colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';

            char *key = line;
            char *value = colon + 1;

            while (*value == ' ') value++; // trim space

            strncpy(req->headers[req->header_count].key, key, sizeof(req->headers[0].key) - 1);

            strncpy(req->headers[req->header_count].value, value, sizeof(req->headers[0].value) - 1);

            req->headers[req->header_count].key[sizeof(req->headers[0].key) - 1] = '\0';

            req->headers[req->header_count].value[sizeof(req->headers[0].value)  -1] = '\0';

            req->header_count++;
        }

        line_start = line_end + 2;
    }

    const char *val = get_header(req, "Content-Length");

    if (val) {
        req->content_length = (size_t)strtol(val, NULL, 10);
    } else {
        req->content_length = 0;
    }
}

const char *get_header(http_request *req, const char *key) {
    for (int i = 0; i < req->header_count; i++) {
        if (strcasecmp(req->headers[i].key, key) == 0) {
            return req->headers[i].value;
        }
    }

    return NULL;
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
        send_response(client_fd, 
                413, "Payload Too Large", 
                "text/plan", 
                "Payload Too Large\n", 
                strlen("Payload Too Large\n"));
        return -1;
    }

    if (read_body(buffer, req, body_start, client_fd, total) < 0) {
        return -1;
    }

    return 0;
}

int parse_form_data(const char *body, form_field *fields, int max_fields) {
    int count = 0;

    char *data = strdup(body);
    char *pair = strtok(data, "&");

    while (pair && count < max_fields) {
        char *eq = strchr(pair, '=');

        if (eq) {
            *eq = '\0';

            char decoded_key[64];
            char decoded_value[256];

            url_decode(pair, decoded_key);
            url_decode(eq + 1, decoded_value);

            strncpy(fields[count].key, decoded_key, sizeof(fields[count].key) - 1);
            strncpy(fields[count].value, decoded_value, sizeof(fields[count].value) - 1);

            fields[count].key[sizeof(fields[count].key) - 1] = '\0';
            fields[count].value[sizeof(fields[count].value) - 1] = '\0';

            count++;
        }

        pair = strtok(NULL, "&");
    }

    free(data);
    return count;
}

void url_decode(char *src, char *dest) {
    char *p = src;
    char code[3] = {0};

    while (*p) {
        if (*p == '%') {
            if (isxdigit(*(p + 1)) && isxdigit(*(p + 2))) {
                code[0] = *(p + 1);
                code[1] = *(p + 2);
                *dest++ = (char)strtol(code, NULL, 16);
                p += 3;
            }
        } else if (*p == '+') {
            *dest++ = ' ';
            p++;
        } else {
            *dest++ = *p++;
        }
    }

    *dest = '\0';
}