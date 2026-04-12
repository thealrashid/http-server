/* Main entry point of the HTTP server */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"

int main() {
    start_server();

    return 0;
}