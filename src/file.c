/* File serving */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file.h"

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

void serve_static_file(int client_fd, const char *path) {
    char file_path[512];

    if (strcmp(path, "/") == 0) {
        strcpy(file_path, "static/index.html");
    } else {
        snprintf(file_path, sizeof(file_path), "static%s", path);
    }

    FILE *file = fopen(file_path, "rb");

    if (!file) {
        printf("File not found\n");
        send_404(client_fd);
        return;                              
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    const char *mime = get_mime_type(file_path);

    send_file_response(client_fd, file, file_size, mime);

    fclose(file);
}