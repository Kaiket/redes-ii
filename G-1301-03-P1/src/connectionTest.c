/*connection.c test*/

#include <stdio.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "../srclib/G-1301-03-P1-connection.h"

void main(int argc, char* argv[]) {
    int socket, client_socket;
    
    openlog("connectionTest", LOG_PID, LOG_LOCAL0);
    
    printf("Iniciando server\n");
    fflush(stdout);
    socket=init_server(atoi(argv[1]), 3);
    if (socket<=0) {
        perror("Error iniciando server\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    printf("Aceptando conexiones\n");
    fflush(stdout);
    while (1) {
        client_socket=accept_connections(socket);
        printf("Conexion aceptada\n");
        printf("Socket: %d\nClient Socket: %d\n", socket, client_socket);
        fflush(stdout);
    }
    
}