#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include "../includes/G-1301-03-P1-types.h"
#include "../includes/G-1301-03-P2-udp.h"

/* Opens and UDP port*/
int open_UDP_socket (){
    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { 
        return ERROR; 
    }
    return fd;
}

void close_UDP_socket(int socket) {
    close(socket);
}

/* Binds a socket to a port*/
int bind_UDP(int socket, u_int16_t port) {
    struct sockaddr_in sa;

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    bzero((void*) &(sa.sin_zero), 8);

    return bind(socket, (struct sockaddr *) &sa, sizeof (struct sockaddr));
}

/*
 * sets a receive timeout for a socket
 */
int set_UDP_rcvtimeout(int socket,unsigned int milisecs) {
    struct timeval tv;
    tv.tv_usec=(milisecs % 1000)*1000;
    tv.tv_sec=(milisecs/1000);
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) return ERROR;
    return OK;
}

/*
 * Gets the port number for a socket
 */
u_int16_t get_UDP_port(int socket) {
    struct sockaddr_in sa_in;
    socklen_t addrlen = sizeof(sa_in);
    if(getsockname(socket, (struct sockaddr *)&sa_in, &addrlen) == 0 &&
       sa_in.sin_family == AF_INET && 
       addrlen == sizeof(sa_in)) {
        return ntohs(sa_in.sin_port);
    }
    return ERROR;
}