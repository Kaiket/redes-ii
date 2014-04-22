#ifndef __UDP_H
#define __UDP_H

#include <sys/types.h>

/**
 * @file G-1301-03-P1-udp.h
 * @details udp socket managing functions.
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/

/** 
 * @brief Opens a UDP socket
 * 
 * @details Opens an UDP socket for IPv4 and returns his id
 * 
 * @return socket id on success. -1 on failure.
 */
int open_UDP_socket ();

/** 
 * @brief Closes an UDP socket
 * 
 * @details Closes the UDP socket passed as argument
 * 
 * @param socket Id of the socket to close
 * 
 * @return none
 */
void close_UDP_socket(int socket);

/** 
 * @brief Binds an UDP socket
 * 
 * @details Binds the UDP socket passed as first argument to the port
 * in the second argument.
 * If the second argument is 0 it binds the socket to a random port over the Well Known Ports
 * 
 * @param socket Id of the socket to bind
 * @param port Port number to bind to, or 0 for any.
 *  
 * @return 0 on success, -1 on error and sets errno to specific error (as bind(2) would do)
 * 
 * @see bind(2)
 *
 */
int bind_UDP(int socket, u_int16_t port);

/** 
 * @brief Sets timeout on recv for a socket
 * 
 * @details Sets the timeout on reception to "milisecs" miliseconds
 * Thus, a blocking recv on such port lasting more than that amount of time
 * without receiving data, will return an error and the socket will be closed.
 * 
 * @param socket Id of the socket
 * @param milisecs Amount of miliseconds to wait until closure of the socket.
 * 
 * @return 0 on success, -1 on error and errno is set appropriately (as setsockopt(2) would do)
 *
 * @see recv(2)
 * @see setsockopt(2)
 */
int set_UDP_rcvtimeout(int socket,unsigned int milisecs);

/** 
 * @brief Gets the port number to which a socket is bound
 * 
 * @details Gets the port number to which a socket is bound.
 * 
 * @param socket Id of the socket
 * 
 * @return 0 on success, -1 on error and sets errno to specific error
 * 
 *      EBADF The argument sockfd is not a valid descriptor.
 *
 *      EFAULT The  addr  argument  points  to memory not in a valid part of the
 *             process address space.
 *
 *      EINVAL addrlen is invalid (e.g., is negative).
 *
 *      ENOBUFS Insufficient resources were available in the  system  to  perform
 *             the operation.
 *
 *      ENOTSOCK The argument sockfd is a file, not a socket.
 *
 * @see getsockname(2)
 */
u_int16_t get_UDP_port(int socket);

#endif