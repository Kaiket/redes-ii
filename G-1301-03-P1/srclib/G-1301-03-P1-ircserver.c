#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-thread_handling.h"
#include "G-1301-03-P1-ircserver.h"
#include "G-1301-03-P1-connection.h"
#include "G-1301-03-P1-irc_errors.h"
#include "G-1301-03-P1-parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

enum {
    PING,
    IRC_TOTAL_COMMANDS
} command_enum;

char *command_names[IRC_TOTAL_COMMANDS] = {"PING"};

/*
 * Function: thread_routine
 * Implementation comments:
 * 
 */
void *irc_thread_routine(void *arg) {

    int received, processed, command_pos, command_num;
    char *command, *save_ptr;
    void *data;
    Thread_handler *settings = (Thread_handler *) arg;


    syslog(LOG_NOTICE, "Server: New thread created for socket %d\n", settings->socket);

    while ((received = receive_msg(settings->socket, &data, IRC_MSG_LENGTH, IRC_MSG_END, strlen(IRC_MSG_END))) > 0) {

        command = strtok_r(data, IRC_MSG_END, &save_ptr);
        processed = strlen(command);

        while (processed <= received) {
            if ((command_pos = irc_get_cmd_position(command)) != ERROR_WRONG_SYNTAX) {
                if ((command_num = parser(IRC_TOTAL_COMMANDS, command_names, command + command_pos)) == IRC_TOTAL_COMMANDS) {
                    if (irc_send_numeric_response(settings->socket, ERR_UNKNOWNCOMMAND) == ERROR) {
                        syslog(LOG_ERR, "Server: Failed while sending numeric response to socket %d: %s", settings->socket, strerror(errno));
                    }
                } else {
                    if (exec_cmd(command_num, settings->socket, command) == ERROR) {
                        syslog(LOG_ERR, "Server: Failed while executing command: %s. Received from socket %d", command, settings->socket);
                    }
                }
            }

            if (processed < received) {
                command = strtok_r(NULL, IRC_MSG_END, &save_ptr);
            }

            processed += strlen(command);
        }
        
        free(data);
    }

    pthread_exit(NULL);
}

/*
 * Function: irc_exit_message
 * Implementation comments:
 *      none
 */
void irc_exit_message() {
    syslog(LOG_ALERT, "Server: finished.");
}

/*
 * Function: irc_split_cmd
 * Implementation comments:
 * 
 *      Splits the string passed as first argument according to IRC RFC commands syntax and returns an array of strings, each one containing a parameter, in
 *      the target_array variable.
 *      The target_array must be a pointer to char*[MAX_CMD_ARGS+2] (IRC RFC sets the maximum parameters for a command to 15, 
 *              (plus prefix and cmd itself it's 17).
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
int irc_split_cmd(char *cmd, char *target_array[MAX_CMD_ARGS + 2], int *prefix, int *n_strings) {
    int arg_start = 0, arg_end = 0, prefix_flag = 1, cmd_length, i;
    if (!cmd || !target_array || !prefix || !n_strings) return ERROR;
    *n_strings = 0;
    *prefix = 0;
    cmd_length = strlen(cmd) - strlen(IRC_MSG_END);
    while (i < cmd_length) {
        if (cmd[i] != IRC_BLANK) {
            if (*n_strings == (*prefix + 1 + MAX_CMD_ARGS)) {
                return ERROR_WRONG_SYNTAX;
            }
            arg_start = i;
            /*prefix found*/
            if (cmd[i] == IRC_PREFIX && prefix_flag == 1) {
                *prefix = 1;
                while ((cmd[i] != IRC_BLANK) && (i < cmd_length)) ++i; /*advance till the next blank char*/
                cmd[i] = '\0';
                ++i;
            }/*last argument found*/
            else if (cmd[i] == IRC_PREFIX) {
                arg_end = cmd_length - strlen(IRC_MSG_END);
                cmd[arg_end + 1] = '\0';
                i = cmd_length;
            }/*common parameter found*/
            else {
                while ((cmd[i] != IRC_BLANK) && (i < cmd_length)) ++i; /*advance till the next blank char*/
                cmd[i] = '\0';
                ++i;
            }
            target_array[*n_strings] = &(cmd[arg_start]);
            ++(*n_strings);
            prefix_flag = 0; /*after a non-blank char, if what we read was not a prefix, we won't have one*/
        } else ++i;
    }
    for (i = (*n_strings); i < MAX_CMD_ARGS; ++i) {
        target_array[i] = NULL;
    }
    return OK;
}

/*
 * returns the position in the array of the first character of the commmand (ignoring prefix and blanks)
 * return ERROR_BAD_SYNTAX if the string is invalid (contains a prefix and a last argument without a command)
 */
int irc_get_cmd_position(char* cmd) {
    int i, prefix = 0, cmd_length;
    cmd_length = strlen(cmd) - strlen(IRC_MSG_END);
    while (i < cmd_length) {
        if (cmd[i] != IRC_BLANK) {
            if (cmd[i] == IRC_PREFIX) {
                if (prefix) return ERROR; /*already found a prefix, invalid command*/
                while ((cmd[i] != IRC_BLANK) && (i < cmd_length)) ++i; /*advance till the next blank char*/
                prefix = 1;
            } else return i;
        } else ++i;
    }
    /*while ended without finding a command*/
    return ERROR_WRONG_SYNTAX;
}

int exec_cmd(int number, int socket, char *msg) {
    switch (number) {
        case PING:
            return irc_ping_cmd(socket, msg);
        default:
            break;
    }

    return OK;
}

int irc_send_numeric_response(int socket, int numeric_response) {

    char ascii_response[IRC_NR_LEN + 1];

    if (sprintf(ascii_response, "%d", numeric_response) <= 0) {
        return ERROR;
    }

    if (send_msg(socket, ascii_response, IRC_NR_LEN + 1, IRC_MSG_LENGTH) <= 0) {
        return ERROR;
    }

    return OK;
}

int irc_ping_cmd(int socket, char *command){

    return OK;
}