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
#include <linux/tcp.h>
#include <unistd.h>
#include "../includes/G-1301-03-P1-connection.h"
#include "../includes/G-1301-03-P1-types.h"


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
    int client_sock;
    unsigned int addrsize;
    char IP_char[16];
    u_int8_t *client_IP;

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


/*Función que cierra la comunicación. Tendrá como parámetro el handler de la conexión a cerrar y devolverá
un código de error.
 */

int close_connection(int socket) {
    return close(socket);
}


/*
 * Function: send_msg
 * Implementation comments:
 * 	Each time that some data is sended, the variable "sended" increments
 *	its value by the value of segmentsize; length is decremented by
 *	the same value. 
 *	This way, length indicates the remaining data that are not sended yet.
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
 * Function: receive_msg
 * Implementation comments:
 * 	It allocates "segmentsize" bytes for buffer that are used to receive data from socket.
 *	After that, appends the data received to the content of "data" allocating the necessary
 *	memory to do it.
 *	Buffer is freed but "data" is not, so the user must free the variable after using it.
 */
int receive_msg(int socket, void **data, size_t segmentsize, void* enddata, size_t enddata_len) {

    short finished_flag = 0;
    int total_received = 0;
    int just_received;
    void *buffer=NULL, *memalloc;

    if (segmentsize <= 0) {
        segmentsize = TCP_MSS_DEFAULT;
    }

    buffer = malloc(segmentsize);
    if (!buffer) {
        syslog(LOG_ERR, "Failed while allocating memory. %s", strerror(errno));
        return ERROR;
    }

    *data=NULL;
    
    while (!finished_flag) {
        memalloc=NULL;
        if ((just_received = recv(socket, buffer, segmentsize, 0)) <= 0) {
            syslog(LOG_ERR, "Failed while receiving. %s", strerror(errno));
            return ERROR;
        }

        memalloc = realloc(*data, total_received + just_received);
        if (!(memalloc)) {
            syslog(LOG_ERR, "Failed while allocating memory. %s", strerror(errno));
            return ERROR;
        }
        *data=memalloc;
        memcpy(*data+total_received, buffer, just_received);
        total_received += just_received;

        if (just_received < segmentsize || !memcmp(*data+(total_received-enddata_len), enddata, enddata_len)) {
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


