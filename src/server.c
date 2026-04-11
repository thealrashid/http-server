#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

void start_server() {
    int server_fd;
    int client_fd;
    struct sockaddr_in address;
    char buffer[1024];
    int bytes;
    char *response;
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

        bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        buffer[bytes] = '\0';

        printf("Received:\n%s\n", buffer);

        response = "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/plain\r\n"
                    "\r\n"
                    "Hello from server\n";

        send(client_fd, response, strlen(response), 0);

        close(client_fd);
    }

    close(server_fd);
}