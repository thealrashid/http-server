#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdio.h>

void send_file_response(int client_fd, FILE *file, long file_size, const char *mime);
void send_404(int client_fd);

#endif