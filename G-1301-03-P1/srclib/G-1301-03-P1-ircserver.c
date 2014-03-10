#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-thread_handling.h"
#include "G-1301-03-P1-ircserver.h"
#include "G-1301-03-P1-connection.h"
#include "G-1301-03-P1-irc_errors.h"
#include "G-1301-03-P1-irc_utility_functions.h"
#include "G-1301-03-P1-parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

struct {
    channel* channels_hasht;
    /*¿semaforos?*/
    user* users_hasht;
    ban* banned_users_llist;
} server_data;

enum {
    PING,
    IRC_TOTAL_COMMANDS
} command_enum;

char *command_names[IRC_TOTAL_COMMANDS] = {"PING"};

/*
 * Initializes (to NULL) server hash tables;
 * This function MUST be used before launching clients, as uthash library needs hash tables to point to NULL the first time we want to add an item.
 */
void irc_server_data_init() {
    server_data.banned_users_llist=NULL;
    server_data.channels_hasht=NULL;
    server_data.users_hasht=NULL;
    /*semaforos*/
}

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
    user* my_user=NULL;
    
    syslog(LOG_NOTICE, "Server: New thread created for socket %d\n", settings->socket);

    if (!(my_user=(user*)malloc(sizeof(user)))) {
        send_msg(settings->socket, "Server is full", strlen("Server is full") + 1, IRC_MSG_LENGTH);
        syslog(LOG_NOTICE, "Server: Terminating thread for socket %d due to malloc error while creating user struct\n", settings->socket);
        pthread_exit(NULL);
    }
    my_user->socket=settings->socket;
    strcpy(my_user->nick, "");
    my_user->user_name=NULL;
    my_user->host_name=NULL;
    my_user->server_name=NULL;
    my_user->real_name=NULL;
    my_user->reg_modes=0;
    my_user->channels_llist=NULL;
    my_user->already_in_server=0;

    while ((received = receive_msg(settings->socket, &data, IRC_MSG_LENGTH, IRC_MSG_END, strlen(IRC_MSG_END))) > 0) {

        command = strtok_r(data, IRC_MSG_END, &save_ptr);
        if (command != NULL){
            processed = strlen(command);
        }

        while (processed <= received && command != NULL) {
            if ((command_pos = irc_get_cmd_position(command)) != ERROR_WRONG_SYNTAX) {
                if ((command_num = parser(IRC_TOTAL_COMMANDS, command_names, command + command_pos)) == IRC_TOTAL_COMMANDS) {
                    if (irc_send_numeric_response(my_user, ERR_UNKNOWNCOMMAND) == ERROR) {
                        syslog(LOG_ERR, "Server: Failed while sending numeric response to socket %d: %s", settings->socket, strerror(errno));
                    }
                } else {
                    if (exec_cmd(command_num, my_user, command) == ERROR) {
                        syslog(LOG_ERR, "Server: Failed while executing command: %s. Received from socket %d", command, settings->socket);
                    }
                }
            }
            processed+=strlen(IRC_MSG_END);
            if (processed < received) {
                command = strtok_r(NULL, IRC_MSG_END, &save_ptr);
            }
            if(command != NULL){
                processed += strlen(command);
            }
            
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
 *      ERROR_WRONG_SYNTAX is returned if the syntax of the command syntax doesn't fit RFC specs (i.e: exceeds number of arguments)
 */
int irc_split_cmd(char *cmd, char *target_array[MAX_CMD_ARGS + 2], int *prefix, int *n_strings) {
    int arg_start = 0, prefix_flag = 1, cmd_length, i=0;
    if (!cmd || !target_array || !prefix || !n_strings) return ERROR;
    *n_strings = 0;
    *prefix = 0;
    cmd_length = strlen(cmd);
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
 * Function: irc_get_cmd_position
 * 
 * Implementation comments:
 * returns the position in the array of the first character of the commmand (ignoring prefix and blanks)
 * return ERROR_BAD_SYNTAX if the string is invalid (contains a prefix and a last argument without a command)
 */
int irc_get_cmd_position(char* cmd) {
    int i=0, prefix = 0, cmd_length;
    cmd_length = strlen(cmd);
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

/*
 * Function: exec_cmd
 * Implementation comments:
 * 
 */
int exec_cmd(int number, user* client, char *msg) {
    switch (number) {
        case PING:
            irc_ping_cmd(client, msg);
            break;
        default:
            break;
    }
    return OK;
}

/*
 * Function: irc_send_numeric_response
 * Implementation comments:
 * 
 *  Sends a numeric response to the user passed as argument (to his socket) 
 */
int irc_send_numeric_response(user* client, int numeric_response) {

    char ascii_response[IRC_NR_LEN + 1];

    if (sprintf(ascii_response, "%d", numeric_response) <= 0) {
        return ERROR;
    }

    if (send_msg(client->socket, ascii_response, IRC_NR_LEN + 1, IRC_MSG_LENGTH) <= 0) {
        return ERROR;
    }

    return OK;
}


/*
 * PING CMD
 */
int irc_ping_cmd(user* client, char *command){

    int prefix, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char response[strlen("PONG ")+strlen(SERVER_NAME)+1];

    split_ret_value = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }

    strcpy(response, "PONG ");
    strcat(response, SERVER_NAME);
    if(send_msg(client->socket, response, strlen(response) ,IRC_MSG_LENGTH) == ERROR){
        return ERROR;
    }
    return OK;
}

/*
 * NICK CMD
 */
int irc_nick_cmd (user* client, char* command) {
    int prefix, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    
    /*split arguments*/
    split_ret_value = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    
    /*check argument number*/
    if ((split_ret_value-prefix)<2)
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS);
    
    /*check arguments format*/
    if (!is_valid_nick(target_array[prefix+1]))
        irc_send_numeric_response(client, ERR_ERRONEUSNICKNAME);
    
    if (!strcmp(client->nick, target_array[prefix+1])) /*if same nick, nothing to do*/
        return OK;
    
    /*check user modes*/
    if ((client->reg_modes & US_MODE_r))
        irc_send_numeric_response(client, ERR_RESTRICTED);
    
    /*down server semaphores*/
    /*if nick exist, collision*/
   
    
    /*up server semaphores*/
    
    return OK;
}

    /*split arguments*/
    
    /*check argument number (enough?)*/
    
    /*check arguments format (correct?)*/
    
    /*check user modes (can i do this?)*/
    
    /*down semaphores if needed*/
    
    /*do something*/
    
    /*up semaphores if needed*/


/*
 * hash tables manipulation functions
 */
void user_hasht_add (user *item) {
    HASH_ADD_STR(server_data.users_hasht, nick, item);
}

void user_hasht_remove(user *item) {
    HASH_DEL(server_data.users_hasht, item);
}

user* user_hasht_find(char key[IRC_MAX_NICK_LENGTH + 1]) {
    user* found=NULL;
    HASH_FIND_STR(server_data.users_hasht, key, found);
    return found;
}

void channel_hasht_add (channel *item) {
    HASH_ADD_KEYPTR(hh, server_data.channels_hasht, item->name, strlen(item->name), item);
}

void channel_hasht_remove(channel *item) {
    HASH_DEL(server_data.channels_hasht, item);
}

channel* channel_hasht_find(char *key) {
    channel* found=NULL;
    HASH_FIND_STR(server_data.channels_hasht, key, found);
    return found;
}