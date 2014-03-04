/*
 * Description
 */


/*
 * Function: 
 * Parameters:
 * 
 * Description:
 * 
 * Return value:
 *      OK on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int init_server(int port, int max_connections);

/*
 * Function: 
 * Parameters:
 * 
 * Description:
 * 
 * Return value:
 *      OK on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int accept_connections(int socket);

/*
 * Function: 
 * Parameters:
 * 
 * Description:
 * 
 * Return value:
 *      OK on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int close_connection(int handler);


/*
 * Function: send_msg
 * Parameters:
 *      -socket: socket descriptor to where send the message.
 *      -data: pointer to data.
 *      -length: length of data.
 *      -segmentsize: max capacity of sending.
 * 
 * Description:
 *      Send messages from "data" as fragments of "segmentsize" bytes to socket
 *      with "socket" file descriptor.
 * 
 *      If "segmentsize" if set to zero or less, it is used the MSS of
 *      TCP.
 * 
 * Return value:
 *      Bytes sended on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int send_msg(int socket, void *data, size_t length, size_t segmentsize);


/*
 * Function: receive_msg
 * Parameters:
 *      -socket: socket descriptor from where received de message.
 *      -data: pointer to pointer to data.
 *      -segmentsize: max capacity of sending.
 *      -enddata: data that point out end of data.
 *	-enddata_len: size of enddata
 * 
 * Description:
 *      Receive messages to "data" as fragments of "segmentsize" bytes from socket
 *      with "socket" file descriptor. It allocate memory for *data so it is 
 *      necessary to free it.
 * 
 *	It stops receiving if the received fragment size is less than segmentsize or if
 *	the last received segment ends with "enddata"
 *
 *      If "segmentsize" if set to zero or less, it is used the MSS of
 *      TCP.
 *      
 * Return value:
 *      Bytes received on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int receive_msg(int socket, void **data, size_t segmentsize, void* enddata, size_t enddata_len);


/*
 * Function: 
 * Parameters:
 * 
 * Description:
 * 
 * Return value:
 *      OK on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int open_TCP_socket();

/*
 * Function: 
 * Parameters:
 * 
 * Description:
 * 
 * Return value:
 *      OK on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int bind_socket(int socket, int port);

/*
 * Function: 
 * Parameters:
 * 
 * Description:
 * 
 * Return value:
 *      OK on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int set_queue_length(int socket, int length);