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
#include "../includes/G-1301-03-P1-types.h"
#include "../includes/G-1301-03-P1-connection.h"

#define SEGMENT_SIZE 20
#define END_CHAR "findecadena\r\n"
#define CLOSE_CONNECTION "close_connection\r\n"

int main(int argc, char* argv[]) {

    int socket, client_socket;
    int received;
    void *data;

    if (argc != 2) {
        printf("Error de argumentos.\nUso:\n\t %s num_puerto\n", argv[0]);
        return ERROR;
    }

    openlog("connection_test", LOG_PID, LOG_LOCAL0);

    printf("Iniciando server\n");
    fflush(stdout);
    socket = init_server(atoi(argv[1]), 3);
    if (socket <= 0) {
        perror("Error iniciando server\n");
        fflush(stdout);
        exit(EXIT_FAILURE);
    }
    printf("Aceptando conexiones\n");
    fflush(stdout);

    client_socket = accept_connections(socket);
    printf("Conexion aceptada\n");
    printf("Socket: %d\nClient Socket: %d\n", socket, client_socket);
    fflush(stdout);
    
    while (1) {

        if ((received = receive_msg(client_socket, &data, SEGMENT_SIZE, END_CHAR, strlen(END_CHAR))) == ERROR) {
            printf("Error al recibir mensaje.\n");
            return ERROR;
        }

        if (send_msg(client_socket, data, received, SEGMENT_SIZE) == ERROR) {
            printf("Error al enviar mensaje.\n");
            return ERROR;
        }
        
        if(!strncmp(data, CLOSE_CONNECTION, strlen(CLOSE_CONNECTION))){
            free(data);
            close(client_socket);
            close(socket);
            return OK;
        }

    }

    return OK;

}
