/*
 * TCP socket creation, port binding, and send/receive functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <pthread.h>
#include <linux/tcp.h>
#include "../includes/G-1301-03-P1-connection.h"
#include "../includes/G-1301-03-P1-types.h"

Thread_handler* threads = NULL;
int max_threads = 0;

/*HIGH LEVEL FUNCTIONS (public)*/

/*
 * Function: init_server
 * Implementation comments:
 *      Initializes a server opening a TCP socket, binding his address to the specified port (argument), 
 *      and setting the queue length to max_connections (argument).
 *      Returns the socket descriptor or an error code and writes a message in the system log describing such error.
 */
int init_server(int port, int max_connections) {
    int fd, ret;

    syslog(LOG_NOTICE, "Creating a new TCP socket");
    fd = open_TCP_socket();
    if (fd <= 0) {
        syslog(LOG_ERR, "Error creating a new TCP socket: %s", strerror(errno));
        return ERROR;
    }

    syslog(LOG_NOTICE, "Binding port %d to TCP socket", port);
    if (bind_socket(fd, port) != OK) {
        syslog(LOG_ERR, "Error binding port %d to TCP socket: %s", port, strerror(errno));
        return ERROR;
    }

    syslog(LOG_NOTICE, "Setting connections queue length to %d", max_connections);
    ret = set_queue_length(fd, max_connections);
    if (ret != OK) {
        if (ret == ERROR_Q_LENGTH) {
            syslog(LOG_ERR, "Error setting queue length: Invalid queue length (<1)");
        } else {
            syslog(LOG_ERR, "Error setting queue length: %s", strerror(errno));
        }
        return ret;
    }

    return fd;

}

/*Función que pondrá al servidor a “escuchar” peticiones de conexión. Devolverá un código de error si la
conexión no se ha realizado.
 */
int accept_connections(int socket) {
    struct sockaddr_in client;
    int client_sock, addrsize, *sock_arg = NULL;
    char IP_char[16];
    u_int8_t *client_IP;
    pthread_t thread;

    addrsize = sizeof (client);

    syslog(LOG_NOTICE, "Server waiting for a new connection.");
    client_sock = accept(socket, (struct sockaddr*) &client, &addrsize);
    if (client_sock < 0) {
        syslog(LOG_ERR, "Error accepting a connection: %s", strerror(errno));
        return ERROR;
    }

    client_IP = (uint8_t*)&(client.sin_addr.s_addr);
    sprintf(IP_char, "%"SCNu8".%"SCNu8".%"SCNu8".%"SCNu8, client_IP[0], client_IP[1], client_IP[2], client_IP[3]);
    syslog(LOG_NOTICE, "Connection from %s in socket %d", IP_char, client_sock);
    return client_sock;
}

pthread_t launch_thread(int client_sock) {
    int *sock_arg;
    pthread_t thread;

    syslog(LOG_NOTICE, "Creating a thread to handle connection in socket %d", client_sock);
    sock_arg = (int*) malloc(sizeof (int));
    *sock_arg = client_sock;
    if (pthread_create(&thread, NULL, thread_routine, (void *) sock_arg) < 0) {
        syslog(LOG_ERR, "Could not create a thread to handle connection in socket %d", client_sock);
        return ERROR;
    }

    if (thread > max_threads) {

    }

    return thread;
}

/*Función que cierra la comunicación. Tendrá como parámetro el handler de la conexión a cerrar y devolverá
un código de error.
 */

int close_connection(int socket) {
    return close(socket);
}

/*
La función que envíe los datos tendrá como parámetros un puntero a los datos (que será de tipo void *)
y la longitud en bytes a enviar. Devolverá la longitud en bytes enviada o un código de error (negativo). Enviará
los datos en paquetes del tamaño máximo de un segmento salvo el último que sólo enviará los datos restantes.
 */
int send_msg(int socket, void *data, size_t length, size_t segmentsize) {

    int sended = 0;

    if (segmentsize <= 0) {
        segmentsize = TCP_MSS_DEFAULT;
    }

    while (length) {

        if (length <= segmentsize) {
            if (send(socket, data + sended, length, 0) != length) {
                syslog(LOG_ERR, "Failed while sending msg. %s", strerror(errno));
                return ERROR;
            }
            sended += length;
            return sended;
        }

        if (send(socket, data + sended, segmentsize, 0) != segmentsize) {
            syslog(LOG_ERR, "Failed while sending msg. %s", strerror(errno));
            return ERROR;
        }
        sended += segmentsize;
        length -= segmentsize;
    }

    return sended;
}

/*
La función que recibe los datos tendrá como parámetro un puntero a un puntero a los datos (que será
de tipo void **). Devolverá la longitud en bytes recibida o un código de error (negativo). Esta función hará sitio
en la memoria para un buffer del tamaño adecuado a esa longitud (cuidado con la terminación de cadena
de caracteres) y rellenará el buffer con los datos. 
Será necesario liberar el buffer tras su llamada.
 */
int receive_msg(int socket, void **data, size_t segmentsize, void* endchar) {

    short finished_flag = 0;
    int total_received = 0;
    int just_received;
    void *buffer;

    if (segmentsize <= 0) {
        segmentsize = TCP_MSS_DEFAULT;
    }

    buffer = malloc(segmentsize);
    if (!buffer) {
        syslog(LOG_ERR, "Failed while allocating memory. %s", strerror(errno));
        return ERROR;
    }

    while (!finished_flag) {

        if ((just_received = recv(socket, buffer, segmentsize, 0)) == -1) {
            syslog(LOG_ERR, "Failed while receiving. %s", strerror(errno));
            return ERROR;
        }

        *data = realloc(*data, total_received + just_received);
        if (!(*data)) {
            syslog(LOG_ERR, "Failed while allocating memory. %s", strerror(errno));
            return ERROR;
        }
        
        memcpy(*data+total_received, buffer, just_received);
        total_received += just_received;

        if (just_received < segmentsize || !memcmp(*data+(total_received-strlen(endchar)), endchar, strlen(endchar))) {
            finished_flag = 1;
        }

    }

    free(buffer);
    return total_received;
}


/*LOW LEVEL FUNCTIONS (private)*/

/*Función que abre un socket de servidor TCP. No tiene parámetros. Devolverá un código de error o el handler
del socket según corresponda.*/
int open_TCP_socket() {
    int fd;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd >= 0) return fd;
    return ERROR;
}

/* Función que asigna un puerto al socket. Tendrá dos parámetros que serán el handler del socket y el número
de puerto. Devolverá un código de error.
 */
int bind_socket(int socket, int port) {
    struct sockaddr_in sa;

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = INADDR_ANY;
    bzero((void*) &(sa.sin_zero), 8);

    return bind(socket, (struct sockaddr *) &sa, sizeof (struct sockaddr));
}

/*Función que determina la longitud de la cola. Tendrá dos parámetros que serán el handler del socket y la
longitud de la cola. Devolverá un código de error.
 */
int set_queue_length(int socket, int length) {
    if (length < 1) return ERROR_Q_LENGTH;
    return listen(socket, length);
}

/*
 * Thread routine
 */
void *thread_routine(void *arg) {
    int socket = *((int *) arg);
    /*Testing routine*/
    char *client_message[2000];
    int read_size;
    syslog(LOG_NOTICE, "New thread created for socket %d\n", socket);
    while ((read_size = recv(socket, client_message, 2000, 0)) > 0) {
        send(socket, client_message, read_size, 0);
        syslog(LOG_NOTICE, "Message received in socket %d\n", socket);
    }
    free(arg);
    pthread_exit(NULL);
}

