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
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-connection.h"
#include "G-1301-03-P1-thread_handling.h"

#define SEGMENT_SIZE 20
#define END_CHAR "\r\n"
#define CLOSE_CONNECTION "close_connection\r\n"

/*
 * Thread routine
 */
void *thread_routine(void *arg) {
    int received;
    void *data;
    Thread_handler *settings = (Thread_handler *) arg;
    
    /*Testing routine*/
    char *client_message[2000];
    int read_size;
    syslog(LOG_NOTICE, "New thread created for socket %d\n", settings->socket);
    
    while ((received = receive_msg(settings->socket, &data, SEGMENT_SIZE, END_CHAR, strlen(END_CHAR))) > 0) {
        syslog(LOG_NOTICE, "Message received in socket %d\n", settings->socket);

        if (send_msg(settings->socket, data, received, SEGMENT_SIZE) == ERROR) {
            printf("Error al enviar mensaje.\n");
            free(data);
        }
        
        if(!strncmp(data, CLOSE_CONNECTION, strlen(CLOSE_CONNECTION))){
            free(data);
            close(settings->socket);
            pthread_exit(NULL);
        }
        free(data);      
    }
    pthread_exit(NULL);
}


int main(int argc, char* argv[]) {

    int socket, client_socket, position;

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
    
    while (1) {
	client_socket = accept_connections(socket);
	printf("Conexion aceptada\n");
    	printf("Socket: %d\nClient Socket: %d\n", socket, client_socket);
        fflush(stdout);
    	position=nbjoin_threads();
	printf("Creando hilo %d\n", position);
	fflush(stdout);
	if (launch_thread(client_socket, thread_routine) != OK) {
            printf("Error al lanzar hilo %d\n", position);
            fflush(stdout);
	}
    }
    
    close(socket);
    return OK;

}
