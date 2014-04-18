#ifndef __CLIENT_UTILITY_FUNCTIONS_H
#define __CLIENT_UTILITY_FUNCTIONS_H

#include "G-1301-03-P1-ircserver.h"

/**
 * @brief Thread function.
 * @details It receives data through a socket and takes the proper action
 * depending on the message that receives.
 * 
 * @param arg Thread management structure.
 * @return always NULL
 */
void *client_thread_listener(void *arg);

/**
 * @brief Manages data received by client_thread_listener.
 * @details Identifies the command/message received and takes the proper action
 * depeding on it.
 * 
 * @param data received by client_thread_listener function.
 * @return OK / ERROR.
 */
int client_receive_data_management(char *data);

/**
 * @brief Prints a full message on the interface.
 * @details The message is printed as it is received, without any change.
 * 
 * @param target_array array of strings that compound the message. 
 * @param prefix indicates wether the message has a prefix or not.
 * @param n_strings number of elements in target_array
 */
void client_print_full_mesage(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/**
 * @brief Manage data received by the user.
 * @details The data may be a command or a message. 
 * If it is a command, it may be an user command or a standard command.
 * In case that it is an user command, the proper action is taken. If 
 * it is a server command, it is sent to the server.
 * If it is a plain message it is sent after check some conditions: the
 * user must be connected to a server and must be in a channel.
 * 
 * @param string received data
 * @param type of data, it may be a command (IRC_CMD) or a message (IRC_MSG).
 */
void client_cmd_parsing(char *string, int type);

/**
 * @brief Sends a request to connect to a server using information provided by the user.
 * @details It needs the server's url and port where to connect and a pointer to the thread
 * thas is going to be executed as a result of the connection, usually to receive data.
 * 
 * @param server_url server's ip or url
 * @param port name of the service or port number
 * @param thread_routine pointer to a thread to be executed.
 * @return OK or ERROR depending on the connection result.
 */
int client_connect_to_server(char* server_url, int port, void* (*thread_routine) (void *arg));

/**
 * @brief Initialize a new session with the server.
 * @details The calling process/thread must be connected to an irc server.
 * This function sends the information needed to stablish a new session with the irc server.
 * 
 * @param nick Nick of the IRC user
 * @param name Name of the IRC user
 * @param real_name Real name of the IRC user.
 * @param server_url Server's ip or url.
 * @return OK or ERROR depending on the session result.
 */
int client_new_session(char *nick, char *name, char *real_name, char *server_url);

/**
 * @brief Send's a command to a server.
 * @details The command may have some parameters that are passed to the function in a string
 * separated by spaces. In case that the user wants to send a singles command without parameters
 * to the server, parameter must be a null pointer.
 * 
 * @param command to send
 * @param parameters of the command.
 * 
 * @return OK or ERROR depending on wether the function to send messages through the socket
 * has failed or not..
 */
int client_send_irc_command(char *command, char *parameters);

/**
 * @brief Check if the client is full connected.
 * @details Here, full connected means that it is connected to a server and
 * joined to a channel.
 * 
 * @return FALSE if the checking results not successful. TRUE if it does.
 */
int client_check_full_connection();

#endif