#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

const char *get_extension(const char *path) {
    const char *dot = strrchr(path, '.');
    return dot ? dot + 1 : "";
}

const char *get_mime_type(const char *path) {
    const char *ext = get_extension(path);

    if (strcmp(ext, "html") == 0) return "text/html";
    if (strcmp(ext, "txt") == 0) return "text/plain";
    if (strcmp(ext, "png") == 0) return "image/png";
    if (strcmp(ext, "jpg") == 0) return "image/jpg";
    if (strcmp(ext, "jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, "css") == 0) return "text/css";
    if (strcmp(ext, "js") == 0) return "application/javascript";

    return "application/octet-stream"; // default
}

void start_server() {
    int server_fd;
    int client_fd;
    struct sockaddr_in address;
    int bytes;
    int opt = 1;

    printf("Starting server...\n");

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Client connected\n");

        char buffer[4096];
        char method[16] = {0};
        char path[256] = {0};
        char file_path[512];
        int total = 0;
        int content_length = 0;

        while (1) {
            bytes = recv(client_fd, buffer + total, sizeof(buffer) - total - 1, 0);

            if (bytes <= 0) break;

            total += bytes;
            buffer[total] = '\0';

            if (strstr(buffer, "\r\n\r\n")) {
                break; // full headers received
            }
        }
        
        if (bytes < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        buffer[bytes] = '\0';

        //printf("Received:\n%s\n", buffer);

        sscanf(buffer, "%s %s", method, path);

        char *line = strtok(buffer, "\r\n");

        while (line != NULL) {
            printf("Header: %s\n", line);

            if (strncmp(line, "Content-Length:", 15) == 0) {
                content_length = atoi(line + 15);
            }

            line = strtok(NULL, "\r\n");
        }

        printf("Method: %s\n", method);
        printf("Path: %s\n", path);

        if (strcmp(path, "/") == 0) {
            strcpy(file_path, "static/index.html");
        } else {
            snprintf(file_path, sizeof(file_path), "static%s", path);
        }

        FILE *file = fopen(file_path, "r");

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);

        if (!file) {
            char *not_found = "HTTP/1.1 404 Not Found\r\n"
                              "Content-Type text/plain\r\n"
                              "\r\n"
                              "File not found\n";
            
            send(client_fd, not_found, strlen(not_found), 0);
            close(client_fd);
            continue;                              
        }

        const char *mime = get_mime_type(file_path);

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

        fclose(file);     

        close(client_fd);
    }

    close(server_fd);
}