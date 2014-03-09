#ifndef __IRCSERVER_H
#define __IRCSERVER_H

#include <limits.h>

#define SERVER_LOG_IDENT "IRC_SERVER"
#define SERVER_MAX_CONNECTIONS INT_MAX
#define IRC_DEFAULT_PORT 6667
#define IRC_MSG_LENGTH 512
#define MAX_CMD_ARGS 15
#define IRC_BLANK 0x20
#define IRC_PREFIX ':'
#define IRC_MSG_END "\r\n"
#define IRC_NR_LEN 3
#ifndef ERROR
        #define ERROR -1
#endif
#define ERROR_WRONG_SYNTAX -2
#define ERROR_MAX_ARGS -3


/*
 * Function: thread_routine
 * Parameters:
 *      arg: Thread_handler.
 * Description:
 *      Take charge of a client, executing the commands received by them.
 * Return value:
 *      void *: not used.
 */
void *irc_thread_routine(void *arg);


/*
 * Function: irc_exit_message
 * Parameters:
 *      none
 * Description:
 *      Writes on the log that the server is being closed.
 * Return value:
 *      none
 */
void irc_exit_message();


/*
 * Function: irc_split_cmd
 * Implementation comments:
 * 
 *      Splits the string passed as first argument according to IRC RFC commands syntax and returns an array of strings, each one containing a parameter, in
 *      the target_array variable.
 *      The target_array must be a pointer to char*[MAX_CMD_ARGS] (IRC RFC sets the maximum parameters for a command to 15).
 * 
 *      Prefix argument is set to 1 if there is a prefix in the string (as indicated in the RFC, the string starts with a ':' if there is a prefix).
 * 
 *      n_strings is set to the number of strings in which the original command has been split.
 * 
 *      No memory is allocated in this function, the original cmd string is modified (after each parameter a \0 is set) and the corresponding pointer in target
 *      array is set to point to that position of the original string.
 *  
 *      On success, OK is returned.
 *      ERROR is returned if arguments are incorrect
 *      ERROR_BAD_SYNTAX is returned if the syntax of the command syntax doesn't fit RFC specs (i.e: exceeds number of arguments)
 */
int irc_split_cmd (char *cmd, char **target_array, int *prefix, int *n_strings);

/*
 * returns the position in the array of the first character of the commmand (ignoring prefix and blanks)
 * return ERROR if the string is invalid (contains a prefix and a last argument without a command)
 */
int irc_get_cmd_position(char* cmd);

void *irc_thread_routine(void *arg);
void irc_exit_message();
int irc_split_cmd (char *cmd, char *target_array[MAX_CMD_ARGS+2], int *prefix, int *n_strings);
int irc_get_cmd_position(char* cmd);
int exec_cmd (int number, char *msg);
int irc_send_numeric_response(int socket, int numeric_response);




#endif