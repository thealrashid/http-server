#ifndef SERVER_H
#define SERVER_H

int create_server_socket();

void start_server();

void *handle_client_thread(void *arg);

#endif