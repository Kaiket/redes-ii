/*
 * Description
 */
typedef struct {
int socket;
char active;
} Thread_handler;

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
int init_server (int port, int max_connections);

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
int accept_connections (int socket);

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
int close_connection (int handler);

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
int open_TCP_socket ();

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
int bind_socket (int socket, int port);

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
int set_queue_length (int socket, int length);

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
void *thread_routine (void *arg);