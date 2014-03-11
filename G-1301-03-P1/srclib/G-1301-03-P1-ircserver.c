#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-thread_handling.h"
#include "G-1301-03-P1-ircserver.h"
#include "G-1301-03-P1-connection.h"
#include "G-1301-03-P1-irc_errors.h"
#include "G-1301-03-P1-irc_utility_functions.h"
#include "G-1301-03-P1-parser.h"
#include "../includes/uthash.h"
#include "../includes/utlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

enum {
    PING,
    NICK,
    PASS,
    USER,
    PRIVMSG,
    WHO,
    SQUIT,
    IRC_TOTAL_COMMANDS
} command_enum;

char *command_names[IRC_TOTAL_COMMANDS] = {"PING", "NICK", "PASS", "USER", "PRIVMSG", "WHO" ,"SQUIT"};

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
        send_msg(settings->socket, "Server is full", strlen("Server is full"), IRC_MSG_LENGTH);
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
                    if (irc_send_numeric_response(my_user, ERR_UNKNOWNCOMMAND, ":Unknown command or too much parameters") == ERROR) {
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
    int ret;
    switch (number) {
        case PING:
            ret=irc_ping_cmd(client, msg);
            break;
        case NICK:
            ret=irc_nick_cmd(client, msg);
            break;
        case PASS:
            ret=irc_pass_cmd(client, msg);
            break;
        case USER:
            ret=irc_user_cmd(client, msg);
            break;
        case PRIVMSG:
            ret=irc_privmsg_cmd(client, msg);
            break;
        case SQUIT:
            ret=irc_squit_cmd(client, msg);
            break;
        default:
            break;
    }
    return ret;
}

/*
 * Function: irc_send_numeric_response
 * Implementation comments:
 * 
 *  Sends a numeric response to the user passed as argument (to his socket) 
 */
int irc_send_numeric_response(user* client, int numeric_response, char* details) {

    char *ascii_response=NULL;
    int length=1 + SERVER_NAME_LENGTH + 1 + IRC_NR_LEN + 1 + IRC_MAX_NICK_LENGTH + strlen(IRC_MSG_END) + 1; /*':servname number targetnick :details'*/

    length+=strlen(details);
    
    if (!(ascii_response=(char*)malloc(length*sizeof(char)))) {
        return ERROR;
    }
    
    if (sprintf(ascii_response, ":%s %d %s %s%s", SERVER_NAME, numeric_response, client->nick, details, IRC_MSG_END) <= 0) {
        free(ascii_response);
        return ERROR;
    }

    if (send_msg(client->socket, ascii_response, strlen(ascii_response), IRC_MSG_LENGTH) <= 0) {
        free(ascii_response);
        return ERROR;
    }

    free(ascii_response);
    return OK;
}


/*
 * PING CMD
 */
int irc_ping_cmd(user* client, char *command){

    int prefix, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char response[strlen("PONG ")+strlen(SERVER_NAME)+strlen(IRC_MSG_END)+1];

    split_ret_value = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }

    strcpy(response, "PONG ");
    strcat(response, SERVER_NAME);
    strcat(response, IRC_MSG_END);
    if(send_msg(client->socket, response, strlen(response) ,IRC_MSG_LENGTH) == ERROR){
        return ERROR;
    }
    return OK;
}

/*
 * NICK CMD
 */
int irc_nick_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2], *new_nick;
    char success_msg[1+IRC_MAX_NICK_LENGTH*2+strlen("NICK")+2+strlen(IRC_MSG_END)+1];
    char welcome_msg[strlen(WELCOME_MSG) + IRC_MAX_NICK_LENGTH + 1];
    channel_lst* elt;
    channel* chan;
    
    /*split arguments*/
    split_ret_value = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    
    /*check argument number*/
    if ((n_strings-prefix)<2) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, ":Need more parameters");
        return OK;
    }
    new_nick=target_array[prefix+1];
    /*check arguments format*/
    if (!is_valid_nick(new_nick)) {
        irc_send_numeric_response(client, ERR_ERRONEUSNICKNAME, ":Incorrect Nickname");
        return OK;
    }
    
    if (!strcmp(client->nick, new_nick)) /*if same nick, nothing to do*/
        return OK;
    
    /*check user modes*/
    if ((client->reg_modes & US_MODE_r)) {
        irc_send_numeric_response(client, ERR_RESTRICTED, ":You don't have permission to do that");
    }
    
    /***************************************************************************************************************down server semaphores*/
    if (user_hasht_find(new_nick)!=NULL) {
        irc_send_numeric_response(client, ERR_NICKNAMEINUSE, ":Nickname already in use");
    }
    else if (client->already_in_server==0 || !user_registered(client->reg_modes)) {/*if not already recongnized by the server, we can just change the nick cause he is not in any channel*/
        if (client->already_in_server==1) user_hasht_remove(client);
        strncpy(client->nick, new_nick, strlen(new_nick)+1);
        user_hasht_add(client);
        client->already_in_server=1;
        if (client->user_name!=NULL) { /*if already has a user name (command USER succesful) we register him*/
            client->reg_modes=(client->reg_modes | USER_REGISTERED);
            sprintf(welcome_msg,WELCOME_MSG,client->user_name);
            irc_send_numeric_response(client, RPL_WELCOME, welcome_msg);
            irc_send_numeric_response(client, RPL_YOURHOST, HOST_MSG);
            irc_send_numeric_response(client, RPL_CREATED, DATE_MSG);
            irc_send_numeric_response(client, RPL_MYINFO, MYINFO_MSG);
        }
    }
    else { /*in server and registered, seek every channel he is in and change the nick in the list of users or operators*/
        LL_FOREACH(client->channels_llist , elt) {
            chan=channel_hasht_find(elt->ch_name);
            if (remove_nick_from_llist(client->nick, &(chan->users_llist))==OK) {
                add_nick_to_llist(new_nick, &(chan->users_llist));
            }
            else if ((remove_nick_from_llist(client->nick, &(chan->operators_llist))==OK)) {
                add_nick_to_llist(new_nick, &(chan->users_llist));
            }
        }
        sprintf(success_msg, ":%s NICK %s%s", client->nick, new_nick, IRC_MSG_END);
        user_hasht_remove(client);
        strncpy(client->nick, new_nick, strlen(new_nick)+1);
        user_hasht_add(client);
        send_msg(client->socket, success_msg, strlen(success_msg), IRC_MSG_LENGTH);
    }    
    
    /***************************************************************************************************************up server semaphores*/
    
    return OK;
}

int irc_squit_cmd (user* client, char* command) {
    /*developing version, automatically shuts down the server*/
    exit(0);
}

int irc_privmsg_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char error_nonick_msg[1+strlen(NICK_NFOUND_MSG) + IRC_MAX_NICK_LENGTH + 1];
    char *error_nochan=NULL;
    char *tok_tmp=NULL;
    char *token=NULL, *target, *msg;
    user *target_user;
    channel *target_channel;
    
    /*if not registered cannot send messages*/
    if (!user_registered(client->reg_modes)) {
        irc_send_numeric_response(client, ERR_NOTREGISTERED, ":You are not registered.");
        return OK;
    }
    
    /*split arguments*/
    split_ret_value = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    
    /*check argument number*/
    if ((n_strings-prefix)<3) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, ":Need more parameters");
        return OK;
    }
    target=target_array[prefix+1];
    msg=target_array[prefix+2];
    
    /************************************************************************************down read semaphores*/
    while ( (token=strtok_r(target, ",", &tok_tmp)) ) {
        target=NULL;
        if (is_valid_nick(token)) { /*token is a user_name*/
            target_user=user_hasht_find(token);
            if (!target_user) { /*user not found*/
                sprintf(error_nonick_msg, NICK_NFOUND_MSG, token);
                irc_send_numeric_response(client, ERR_NOSUCHNICK, error_nonick_msg);
            }
            else {
                send_privmsg_to_user(client, target_user, msg);
            }
        }
        else if (is_valid_chname(token)) { /*token is a channel_name*/
            target_channel=channel_hasht_find(token);
            if (!target_channel) { /*channel not found*/
                if (!(error_nochan=(char*)malloc((strlen(CANNOTSENDTOCHAN_MSG)+strlen(token)+1)*sizeof(char)))) continue;
                sprintf(error_nochan, CANNOTSENDTOCHAN_MSG, token);
                irc_send_numeric_response(client, ERR_CANNOTSENDTOCHAN, error_nochan);
                free(error_nochan);
                error_nochan=NULL;
            }
            else {
                send_privmsg_to_chan(client, target_channel, msg);
            }
        }
    }
    return OK;
    /************************************************************************************up read semaphores*/
}

int send_privmsg_to_user (user *origin, user *target, char* msg) {
    char away_err[strlen(USER_AWAY_MSG) + IRC_MAX_NICK_LENGTH + 1];
    char error_nonick_msg[strlen(NICK_NFOUND_MSG) + IRC_MAX_NICK_LENGTH + 1];
    char *full_reply=NULL; 
    
    if (!user_registered(target->reg_modes)){
        sprintf(error_nonick_msg, NICK_NFOUND_MSG, target->nick);
        irc_send_numeric_response(origin, ERR_NOSUCHNICK, error_nonick_msg);
    }
    else if (user_mode_a(target->reg_modes)) {
        sprintf(away_err, USER_AWAY_MSG, target->nick);
        irc_send_numeric_response(origin, RPL_AWAY, away_err);
    }
    else {
        syslog(LOG_NOTICE, "d");
        if (!(full_reply=(char*)malloc(sizeof(char)*(1 + IRC_MAX_NICK_LENGTH*2 + 1 + strlen("PRIVMSG") + 1 + 1 + strlen(msg)+ 1 + strlen(IRC_MSG_END) + 1)))) {
            return ERROR;
        }
        sprintf(full_reply, ":%s PRIVMSG %s %s%s", origin->nick, target->nick, msg, IRC_MSG_END);
        if (send_msg(target->socket, full_reply, strlen(full_reply), IRC_MSG_LENGTH)!=OK) {
            free(full_reply);
            return ERROR;
        }
        free(full_reply);
    }
    return OK;
}

int send_privmsg_to_chan (user *origin, channel *target, char* msg) {
    if (chan_mode_n(target->modes)) { //cant receive message from not members
        
    }
}

/*
 * PASS CMD
 */
int irc_pass_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    
    /*split arguments*/
    split_ret_value = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    
    /*check argument number*/
    if ((n_strings-prefix)<2) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, ":Need more parameters");
        return OK;
    }
    
    /***********************************************************************************************down server read sem*/
    
    if (user_registered(client->reg_modes)) {
        irc_send_numeric_response(client, ERR_ALREADYREGISTRED, ":You can't register twice!");
    }
    
    /***********************************************************************************************up server read sem*/
    
    return OK;
}

int irc_user_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value, requested_modes=0;
    char *target_array[MAX_CMD_ARGS + 2];
    char *user, *mode, *realname, welcome_msg[strlen(WELCOME_MSG) + IRC_MAX_NICK_LENGTH + 1];
    
    /*split arguments*/
    split_ret_value = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    
    /*check argument number*/
    if ((n_strings-prefix)<4) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, ":Need more parameters");
        return OK;
    }
    user=target_array[prefix+1];
    mode=target_array[prefix+2];
    realname=target_array[prefix+4];
   
    /***********************************************************************************************down server write sem*/
    if (user_registered(client->reg_modes)) {
        irc_send_numeric_response(client, ERR_ALREADYREGISTRED, ":You can't register twice!");
    }
    else {
        requested_modes=atoi(mode);
        requested_modes=(requested_modes & (US_MODE_default)); 
        if (!(client->user_name=strdup(user))) {
            /**************************************************************************up write sem*/
            return ERROR;
        }
        if (!(client->real_name=strdup(realname))) {
            free(client->user_name);
            client->user_name=NULL;
            /***************************************************************************up write sem*/
            return ERROR;
        }
        client->reg_modes=requested_modes;
        if (client->already_in_server) { /*user already has a nickname, we register and welcome him*/
            client->reg_modes=(client->reg_modes | (USER_REGISTERED));
            sprintf(welcome_msg,WELCOME_MSG,user);
            irc_send_numeric_response(client, RPL_WELCOME, welcome_msg);
            irc_send_numeric_response(client, RPL_YOURHOST, HOST_MSG);
            irc_send_numeric_response(client, RPL_CREATED, DATE_MSG);
            irc_send_numeric_response(client, RPL_MYINFO, MYINFO_MSG);
        }
    }
    /***********************************************************************************************up server write sem*/
    return OK;
}


/*
 * WHO CMD
 */
int irc_who_cmd(user *client, char *command){

    int prefix=0, n_strings, split_ret;
    char *target_array[MAX_CMD_ARGS + 2];
    char *param, *saveptr, *details;
    user *usr_found;
    channel *ch_found;
    active_nicks* nick;

    split_ret = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret == ERROR || split_ret == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    
    if ((n_strings-prefix)<2) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, ":Not enough parameters");
        return OK;
    }

    param = strtok_r(target_array[prefix+1], ",", &saveptr);

    while(param){
        /**SEMAPHORE_HERE**/
        ch_found = channel_hasht_find(param);

        if(ch_found){
            LL_FOREACH(ch_found->operators_llist, nick){
                usr_found = user_hasht_find(nick->nick);
                if (usr_found) {
                    if(!user_mode_i(usr_found->reg_modes)){
                        details = malloc(strlen(param) + 2 + strlen(usr_found->user_name) + 5 + strlen(usr_found->nick) + 4 + strlen(usr_found->real_name)+1);
                        if(!details) return ERROR;
                        sprintf(details, "%s ~%s - - %s H@ %s", param, 
                                                                   usr_found->user_name,  
                                                                   usr_found->nick,
                                                                   usr_found->real_name);
                        irc_send_numeric_response(client, RPL_WHOREPLY, details);
                        free(details);
                    }
                }
            }

            LL_FOREACH(ch_found->users_llist, nick){
                usr_found = user_hasht_find(nick->nick);
                if (usr_found) {
                    if(!user_mode_i(usr_found->reg_modes)){
                        details = malloc(strlen(param) + 2 + strlen(usr_found->user_name) + 5 + strlen(usr_found->nick) + 3 + strlen(usr_found->real_name)+1);
                        if(!details) return ERROR;
                        sprintf(details, "%s ~%s - - %s H %s", param, 
                                                                   usr_found->user_name, 
                                                                   usr_found->nick,
                                                                   usr_found->real_name);
                        irc_send_numeric_response(client, RPL_WHOREPLY, details);
                        free(details);
                    }
                }
            }
        }

        irc_send_numeric_response(client, RPL_ENDOFWHO, ":End of /WHO list.");
        param = strtok_r(NULL, ",", &saveptr);
    }

    return OK;
    

}

    /*split arguments*/
    
    /*check argument number (enough?)*/
    
    /*check arguments format (correct?)*/
    
    /*check user modes (can i do this?)*/
    
    /*down semaphores if needed*/
    
    /*do something*/
    
    /*up semaphores if needed*/
