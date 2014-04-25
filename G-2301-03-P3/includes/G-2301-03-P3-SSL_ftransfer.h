#ifndef __SSL_FTRANSFER_H
#define __SSL_FTRANSFER_H

/**
 * @file G-2301-03-P2-call_funcs.h
 * @details Functions for management of file transfer
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/

#include <sys/types.h>

#ifndef OK
        #define OK 0
#endif
#ifndef ERROR
        #define ERROR -1
#endif
#define ERROR_ALREADY_TRANSFERING -2
#define ERROR_ALREADY_SETUP -3
#define ERROR_SOCKET -4
#define ERROR_BINDING -5
#define ERROR_TIMEOUTSET -6
#define ERROR_GETPORTBOUND -7
#define ERROR_PARAM -8
#define ERROR_THREAD -9

#define SENDER_SIDE 1
#define RECEIVER_SIDE 2

#define TIMEOUT_DEFAULT 10000
#define MAX_BUFFER 256
#define RECORD_SIZE 160

/** 
 * @brief Makes a transfer of a file passed to an IP and port
 * 
 * @details As a sender, this function opens a TCP socket and waits
 * for an SSL connection, once it is verified it starts transmitting the file passed.
 * 
 * As a receiver, this function connects to an IP and port via SSL connection
 * and, once verified, starts saving the file transfered to 
 * 
 * @param ip string containing ip to call (i.e "194.12.1.2")
 * @param port Port number in which such IP is expecting the connection.
 * 
 * @return 0 on success.
 *      ERROR_PARAM ip specified is not correct (or not IPv4)
 *      ERROR_SOCKET Error alocating memory for sockaddr_in variables
 *      ERROR_THREAD Error creating threads to manage the call.
 *      
 *
 * @see setup_call(), end_call()
 */
long transfer(char* ip, u_int16_t port, int side, char* filename, u_int32_t size);

/** 
 * @brief If a call is in progress
 *  
 * @details A global variable is set to 1 when a conversation starts ("call" function succeded).
 * Thus, with this function can be checked if a call is in progress.
 * 
 * @return 0 if no call is in progress, 1 if a call is in progress
 *
 */
char already_transfering();

/** 
 * @brief If a call has finished 
 * 
 * @details If a call has finished (because of the socket timing out) and no end_call()
 * has been performed, another call cannot be set up.
 * This function checks if a call has already ended so the needed end_call() is done by the developer.
 * 
 * @return 0 if no call has finished, 1 if a call has finished
 *
 * @see end_call, call(), setup_call()
 */
char is_finished_transfer();

/** 
 * @brief If the ip passed maches caller ip
 * 
 * @details This functions checks if the ip passed matches the ip address of the caller
 * of the call already in progress(if no call in progress it will compare to ip 0.0.0.0)
 * 
 * @param ip String representing ip to check (i.e "154.1.123.5")
 * 
 * @return 0 if IPs doesn't match, 1 if they do.
 *
 */
int is_sender_ip(char *ip);

/** 
 * @brief Ends a call
 * 
 * @details Closes socket and terminates threads that manages the call.
 * Sets every global variable to 0 so another call can be initiated (after another setup_call)
 * 
 */
void end_transfer();


#endif