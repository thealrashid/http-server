#ifndef FILE_H
#define FILE_H

const char *get_mime_type(const char *path);
void serve_static_file(int client_fd, const char *path);

#endif