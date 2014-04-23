#ifndef __CONNECTION_H
#define __CONNECTION_H

#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT_LEN 6
/**
 * @file G-2301-03-P1-connection.h
 * @details Functions for connecting processes.
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/


/**
 * @brief Starts a new server.
 * @details Initializes a server opening a TCP socket, binding his address to the specified port (argument), 
 * and setting the queue length to max_connections (argument).
 * Returns the socket descriptor or an error code and writes a message in the system log describing such error.
 * 
 * @param port port to bind the adress of the created TCP socket.
 * @param max_connections number of the queue length
 * 
 * @return On failure, an error code defined in G-2301-03-P1-types.h is returned. On success 
 * the file descriptor of the new socket is returned.
 */
int init_server(int port, int max_connections);


/**
 * @brief Starts accepting incoming connexions.
 * @details It starts listening and accepting conming connections through the socket
 * which file descriptor is the argumment "socket".
 * This function is a blocking function, what means that when it is called, the calling process
 * will be blocked until receive a connection request.
 * This function writes in the system logger.
 * 
 * @param socket file descriptor of the socket for which the connections are going to be
 * accepted.
 * @return On success, the file descriptor of the client socket. ERROR code on failure, which is defined in
 * G-2301-03-P1-types.h header file.
 */
int accept_connections(int socket);

/**
 * @brief Close a connection.
 * @details Close the established connection with the socket which file descriptor
 * is the "socket" argument. For further information, see close(2).
 * 
 * @param handler file descriptor of the socket which connection is going to be closed.
 * @return the same as close(2) function.
 */
int close_connection(int socket);


 /**
  * @brief Sends messages through a socket.
  * @details Send messages from "data" as fragments of "segmentsize" bytes to socket
  * with "socket" file descriptor.
  * 
  * If "segmentsize" if set to zero or less, it is used the MSS of
  * TCP.
  * 
  * @param socket socket descriptor to where send the message.
  * @param data pointer to data which is going to be sent.
  * @param length length of data.
  * @param segmentsize max capacity of sending.
  * @return Bytes sended on success.
  * ERROR on failure (which is defined in G-2301-03-P1-types.h), and it writes the description of the error in the log
  * file.
  */
int send_msg(int socket, void *data, size_t length, size_t segmentsize);


 /**
  * @brief Receive messages through a socket.
  * @details
  * Receive messages to "data" as fragments of "segmentsize" bytes from socket
  * with "socket" file descriptor. It allocate memory for *data so it is 
  * necessary to free it.
  * 
  * It stops receiving if the received fragment size is less than segmentsize or if
  * the last received segment ends with "enddata"
  *
  * If "segmentsize" if set to zero or less, it is used the MSS of
  * TCP.
  * 
  * @param socket socket descriptor from where received de message.
  * @param data pointer to pointer to data.
  * @param segmentsize max capacity of sending.
  * @param enddata data that point out end of data.
  * @param enddata_len size of enddata.
  * @return Bytes received on success.
  * ERROR on failure (which is defined in G-2301-03-P1-types.h), and it writes the description of the error in the log
  * file.
  */
int receive_msg(int socket, void **data, size_t segmentsize, void* enddata, size_t enddata_len);


/*Client functions*/
/**
 * @brief Connects to a server.
 * @details This function uses a host name (or host ip) and a service (port)
 * to connect to a server, returning the file descriptor of the created socket.
 * 
 * @param host_name name/ip of the host.
 * @param port port to which connect.
 * 
 * @return On success connect_to_server returns the SSL pointer.
 * On failure, it returns NULL.
 */
SSL *connect_to_server(char *host_name, int port, void* (*thread_routine) (void *arg));


/**
 * @brief Gets the ip assoiated to a socket descriptor.
 * @details The local ip is stored at ip pointer with the format
 * x.x.x.x where x is a number between 0 and 255.
 * 
 * @param socket from where get the ip
 * @param ip objetive to copy the ip
 * 
 * @return OK or ERROR depending on the returned value of getsockname
 */
int get_own_ip(int socket, char *ip);

#endif