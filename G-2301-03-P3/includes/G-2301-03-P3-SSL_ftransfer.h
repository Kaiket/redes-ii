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
 * @param ip string containing ip to get the transfer from (for a sender, can be NULL)
 * @param port Port number in which such IP is expecting the connection.
 * @param side Either SENDER_SIDE (file sender) or RECEIVER_SIDE (file receiver)
 * @param filename path to the file that wants to be sent/name to give the file to be received
 * @param size size of the file to be sent/received
 * 
 * @return 0 on success.
 *      ERROR_PARAM ip specified is not correct (or not IPv4)
 *      ERROR_SOCKET Error initializing sockets for the connection.
 *      ERROR_THREAD Error creating threads to manage the transfer.
 *      
 *
 * @see end_transfer()
 */
long transfer(char* ip, u_int16_t port, int side, char* filename, u_int32_t size);

/** 
 * @brief If a transfer is in progress
 *  
 * @details A global variable is set to 1 when a transfer starts
 * Thus, with this function can be checked if a transfer is in progress.
 * 
 * @return 0 if no transfer is in progress, 1 if a transfer is in progress
 *
 */
char already_transfering();

/** 
 * @brief If a transfer has finished 
 * 
 * @details If a transfer has finished and no end_transfer()
 * has been performed, another transfer cannot be made.
 * This function checks if a transfer has already ended so the needed end_transfer() is done by the developer.
 * 
 * @return 0 if no transfer has finished, 1 if a transfer has finished
 *
 * @see end_transfer, transfer()
 */
char is_finished_transfer();

/** 
 * @brief If the ip passed matches peer ip
 * 
 * @details This functions checks if the ip passed matches the ip address of the peer
 * of the transfer already in progress (if no transfer in progress it will compare to ip 0.0.0.0)
 * 
 * @param ip String representing ip to check (i.e "154.1.123.5")
 * 
 * @return 0 if IPs doesn't match, 1 if they do.
 *
 */
int is_sender_ip(char *ip);

/** 
 * @brief Ends a transfer
 * 
 * @details Closes socket and all SSL variables and terminates threads that manages the transfer.
 * Sets every global variable to 0 so another transfer can be initiated
 * 
 */
void end_transfer();


#endif