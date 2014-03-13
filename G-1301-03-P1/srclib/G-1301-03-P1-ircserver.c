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
    MODE,
    NICK,
    PASS,
    USER,
    PRIVMSG,
    NAMES,
    JOIN,
    LIST,
    TOPIC,
    WHO,
    PART,
    OPER,
    QUIT,
    SQUIT,
    IRC_TOTAL_COMMANDS
} command_enum;

char *command_names[IRC_TOTAL_COMMANDS] = {"PING", "MODE", "NICK", "PASS", "USER", "PRIVMSG", "NAMES", "JOIN", "LIST", "TOPIC", "WHO", "PART", "OPER", "QUIT" ,"SQUIT"};

/*
 * Initializes (to NULL) server hash tables;
 * This function MUST be used before launching clients, as uthash library needs hash tables to point to NULL the first time we want to add an item.
 */
void irc_server_data_init() {
    server_data.banned_users_llist=NULL;
    server_data.channels_hasht=NULL;
    server_data.users_hasht=NULL;
    /*inicializar semaforos*/
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
    user *my_user=NULL;
    channel_lst *elt;
    channel *chan;
    
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

    /*if socket was closed, maybe some data from user is still in server_data, we remove it*/
    /**********************************************************************************write sem down*/
    if (my_user->already_in_server) { /*known by the server*/
        if (user_hasht_find(my_user->nick)){
            user_hasht_remove(my_user);
            LL_FOREACH(my_user->channels_llist, elt) {
                chan=channel_hasht_find(elt->ch_name);
                if (chan) {
                    remove_nick_from_llist(my_user->nick, &(chan->users_llist));
                    remove_nick_from_llist(my_user->nick, &(chan->operators_llist));
                    chan->users_number--;
                    if (is_empty_channel(chan)) {
                        channel_hasht_remove(chan);
                        free_channel(chan);
                    }
                }
            }
        }
    }
    free_user(my_user);
    /**********************************************************************************write sem up*/
    
    
    pthread_exit(NULL);
}

void free_channel (channel* ch) {
    active_nicks *el, *tmp; 
    if (!ch) return;
    if (ch->name) free(ch->name);
    if (ch->topic) free(ch->topic);
    if (ch->pass) free(ch->pass);
    if (ch->users_llist) {
        LL_FOREACH_SAFE(ch->users_llist, el, tmp) {
            LL_DELETE(ch->users_llist,el);
            free(el);
        }
    }
    if (ch->operators_llist) {
        LL_FOREACH_SAFE(ch->operators_llist, el, tmp) {
            LL_DELETE(ch->operators_llist,el);
            free(el);
        }
    }
    if (ch->invited_llist) {
        LL_FOREACH_SAFE(ch->invited_llist, el, tmp) {
            LL_DELETE(ch->invited_llist,el);
            free(el);
        }
    }
    free(ch);
    return;
}

void free_user(user* us) {
    channel_lst *ch, *tmp;
    
    if (!us) return;
    if (us->user_name) free(us->user_name);
    if (us->host_name) free(us->host_name);
    if (us->server_name) free(us->server_name);
    if (us->real_name) free(us->real_name);
    if (us->channels_llist) {
        LL_FOREACH_SAFE(us->channels_llist, ch, tmp) {
            LL_DELETE(us->channels_llist,ch);
            free(ch->ch_name);
            free(ch);
        }
    }
    free(us);
    return;
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
        case MODE:
            ret=irc_mode_cmd(client, msg);
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
        case NAMES:
            ret=irc_names_cmd(client, msg);
            break;
        case JOIN:
            ret=irc_join_cmd(client, msg);
            break;
        case LIST:
            ret=irc_list_cmd(client, msg);
            break;
        case TOPIC:
            ret=irc_topic_cmd(client, msg);
            break;
        case WHO:
            ret=irc_who_cmd(client, msg);
            break;
        case PART:
            ret=irc_part_cmd(client, msg);
            break;
        case OPER:
            ret=irc_oper_cmd(client, msg);
            break;
        case QUIT:
            ret=irc_quit_cmd(client, msg);
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
    int length=1 + SERVER_NAME_LENGTH + 1 + IRC_NR_LEN + 1 + IRC_MAX_NICK_LENGTH + 1 + strlen(IRC_MSG_END) + 1; /*':servname number targetnick :details'*/

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

/*sends a string to every user in channel given*/
int send_msg_to_channel(channel* chan, char* msg) {
    active_nicks* elt;
    user* user_in_channel;
    if (!chan || !msg) return OK;
    /*notification to every user in channel*/
    LL_FOREACH(chan->users_llist, elt) {
        if ((user_in_channel=user_hasht_find(elt->nick))) {
            send_msg(user_in_channel->socket, msg, strlen(msg), IRC_MSG_LENGTH);
        }
    }
    LL_FOREACH(chan->operators_llist, elt) {
        if ((user_in_channel=user_hasht_find(elt->nick))) {
            send_msg(user_in_channel->socket, msg, strlen(msg), IRC_MSG_LENGTH);
        }
    }
    return OK;
}

/*sends a string to every user in channel given*/
int send_msg_to_channel_except(user* client, channel* chan, char* msg) {
    active_nicks* elt;
    user* user_in_channel;
    if (!chan || !msg) return OK;
    /*notification to every user in channel*/
    LL_FOREACH(chan->users_llist, elt) {
        if ((user_in_channel=user_hasht_find(elt->nick)) && strcmp(elt->nick, client->nick)) {
            send_msg(user_in_channel->socket, msg, strlen(msg), IRC_MSG_LENGTH);
        }
    }
    LL_FOREACH(chan->operators_llist, elt) {
        if ((user_in_channel=user_hasht_find(elt->nick)) && strcmp(elt->nick, client->nick)) {
            send_msg(user_in_channel->socket, msg, strlen(msg), IRC_MSG_LENGTH);
        }
    }
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
    if(send_msg(client->socket, response, strlen(response), IRC_MSG_LENGTH) <= 0){
        return ERROR;
    }
    return OK;
}


int apply_modes_to_chan(user *us, channel* ch, char* modestr, char** modeargs) {
    char oper, unknown_mode, *unk_msg, *msg=NULL;
    unsigned int wanted_modes;
    int i=0, arg_count=0;
    user* targ_us;
    
    if (!(unk_msg=malloc(sizeof(UNKNOWNMODE_MSG)+strlen(ch->name)+1))) return ERROR;
    sprintf(unk_msg, UNKNOWNMODE_MSG, ch->name);
    
    wanted_modes=chan_mode_from_str(modestr, &unknown_mode, &oper);
    if (modestr[0]=='-') { oper='-'; ++i;}
    else {
        oper='+';
        if (modestr[0]=='+') ++i;
    }
    for (;modestr[i]!='\0'; ++i) {
        switch(modestr[i]) {
            case 'o':
                if (arg_count>=3) continue;
                else if (modeargs[arg_count]==NULL) {irc_send_numeric_response(us, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG); arg_count++; continue;}
                else if (!is_valid_nick(modeargs[arg_count])) {arg_count++; continue;}
                if (!(msg=(char*)malloc(1+IRC_MAX_NICK_LENGTH*2 + 2 + strlen("MODE") + 1 + strlen(ch->name) + 5 + strlen(IRC_MSG_END) + 1))) {
                    arg_count++;
                    continue;
                }
                if (oper=='+'){
                    if (find_nick_in_llist(modeargs[arg_count], &(ch->operators_llist))) {
                        arg_count++; 
                        free(msg);
                        continue;
                    }
                    if (find_nick_in_llist(modeargs[arg_count], &(ch->users_llist))) {
                        if (remove_nick_from_llist(modeargs[arg_count], &(ch->users_llist))) {
                            add_nick_to_llist(modeargs[arg_count], &(ch->operators_llist));
                            sprintf(msg, ":%s MODE %s +o %s%s", us->nick, ch->name, modeargs[arg_count], IRC_MSG_END);
                            send_msg_to_channel(ch, msg);
                        }
                    }
                    else {
                        irc_send_numeric_response(us, ERR_USERNOTINCHANNEL, ":User not in channel");
                    }
                }
                else {
                    targ_us=user_hasht_find(modeargs[arg_count]);
                    if (!targ_us) {free(msg); arg_count++; continue;}
                    if (user_mode_o(targ_us->reg_modes) || user_mode_O(targ_us->reg_modes)) {
                        arg_count++; 
                        free(msg);
                        continue;
                    }
                    else if (remove_nick_from_llist(modeargs[arg_count], &(ch->operators_llist))) {
                        add_nick_to_llist(modeargs[arg_count], &(ch->users_llist));
                        sprintf(msg, ":%s MODE %s -o %s%s", us->nick, ch->name, modeargs[arg_count], IRC_MSG_END);
                        send_msg_to_channel(ch, msg);
                    }
                }
                arg_count++;
                free(msg);
                break;
                
            case 'i':
                if (!(msg=(char*)malloc(1+IRC_MAX_NICK_LENGTH + 1 + strlen("MODE") + 1 + strlen(ch->name) + 1 + strlen(IRC_MSG_END) + 1))) continue;
                if (oper=='+'){
                    sprintf(msg, ":%s MODE %s +i%s", us->nick, ch->name,  IRC_MSG_END);
                }
                else {
                    sprintf(msg, ":%s MODE %s -i%s", us->nick, ch->name,  IRC_MSG_END);
                }
                send_msg_to_channel(ch, msg);
                free(msg);
                break;
                
            case 'n':
                if (!(msg=(char*)malloc(1+IRC_MAX_NICK_LENGTH + 1 + strlen("MODE") + 1 + strlen(ch->name) + 1 + strlen(IRC_MSG_END) + 1))) continue;
                if (oper=='+'){
                    sprintf(msg, ":%s MODE %s +n%s", us->nick, ch->name,  IRC_MSG_END);
                }
                else {
                    sprintf(msg, ":%s MODE %s -n%s", us->nick, ch->name,  IRC_MSG_END);
                }
                send_msg_to_channel(ch, msg);
                free(msg);
                break;
                
            case 't':
                if (!(msg=(char*)malloc(1+IRC_MAX_NICK_LENGTH + 1 + strlen("MODE") + 1 + strlen(ch->name) + 1 + strlen(IRC_MSG_END) + 1))) continue;
                if (oper=='+'){
                    sprintf(msg, ":%s MODE %s +t%s", us->nick, ch->name,  IRC_MSG_END);
                }
                else {
                    sprintf(msg, ":%s MODE %s -t%s", us->nick, ch->name,  IRC_MSG_END);
                }
                send_msg_to_channel(ch, msg);
                free(msg);
                break;
                
            case 'k':
                if (!(msg=(char*)malloc(1+IRC_MAX_NICK_LENGTH + 1 + strlen("MODE") + 1 + strlen(ch->name) + 1 + strlen(" channel") + strlen(IRC_MSG_END) + 1))) 
                    continue;
                wanted_modes=(wanted_modes & ~CH_MODE_k);
                if (oper=='+'){
                    if (arg_count>=3) {free(msg); continue;}                    
                    else if (modeargs[arg_count]==NULL) {
                        irc_send_numeric_response(us, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG); 
                        free(msg); 
                        arg_count++; 
                        continue;
                    }
                    else if (!is_valid_pass(modeargs[arg_count])) {free(msg); arg_count++; continue;}
                    if (ch->pass) {
                        irc_send_numeric_response(us, ERR_KEYSET, ":Key already set");
                        free(msg);
                        arg_count++;
                        continue;
                    }
                    ch->pass=strdup(modeargs[arg_count]);
                    sprintf(msg, ":%s MODE %s +k channel%s", us->nick, ch->name,  IRC_MSG_END);
                    arg_count++;
                }
                else {
                    if (!ch->pass) {free(msg); continue;}
                    free(ch->pass);
                    ch->pass=NULL;
                    sprintf(msg, ":%s MODE %s -k%s", us->nick, ch->name,  IRC_MSG_END);
                    
                }
                wanted_modes=(wanted_modes | CH_MODE_k);
                send_msg_to_channel(ch, msg);
                free(msg);
                break;
                
            case 'l':
                if (!(msg=(char*)malloc(1+IRC_MAX_NICK_LENGTH + 1 + strlen("MODE") + 1 + strlen(ch->name) + 1 + strlen(IRC_MSG_END) + 1))) continue;
                wanted_modes=(wanted_modes & ~CH_MODE_l);
                if (oper=='+'){
                    if (arg_count>=3) {free(msg); continue;}                    
                    else if (modeargs[arg_count]==NULL) {
                        irc_send_numeric_response(us, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG); 
                        free(msg); 
                        arg_count++; 
                        continue;
                    }
                    if (atoi(modeargs[arg_count])<=0) {
                        wanted_modes=(wanted_modes & ~CH_MODE_l);
                        free(msg);
                        continue;
                    }
                    ch->users_max=atoi(modeargs[arg_count]);
                    sprintf(msg, ":%s MODE %s +l%s", us->nick, ch->name,  IRC_MSG_END);
                    arg_count++;
                }
                else {
                    wanted_modes=(wanted_modes & ~CH_MODE_l);
                    if (!chan_mode_l(ch->modes)) {free(msg); continue;}
                    sprintf(msg, ":%s MODE %s -l%s", us->nick, ch->name,  IRC_MSG_END);
                }
                wanted_modes=(wanted_modes | CH_MODE_l);
                send_msg_to_channel(ch, msg);
                free(msg);
                break;
                
            default:
                unk_msg[0]=modestr[i];
                irc_send_numeric_response(us, ERR_UNKNOWNMODE, unk_msg);
                break;
        }
        msg=NULL;
    }
    if (oper=='+') ch->modes=(ch->modes | wanted_modes);
    else ch->modes=(ch->modes & ~wanted_modes);
    free(unk_msg);
    return OK;
}

/*
 * MODE CMD
 */
int irc_mode_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value, passlen;
    char *target_array[MAX_CMD_ARGS + 2], *user_mode_str, *ch_mode_str;
    char unknown_mode, operation, *msg; 
    char user_new_mode;
    channel* target_chan;
    
    /*if not registered action not allowed*/
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
    if ((n_strings-prefix)<2) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG);
        return OK;
    }
    
    if (is_valid_nick(target_array[prefix+1])) { /*target is a user*/
        if (strcmp(client->nick, target_array[prefix+1])) { /*users dont match*/
            irc_send_numeric_response(client, ERR_USERSDONTMATCH, USERSDONTMATCH_MSG);
        }
        else if ((n_strings-prefix)==2) { /*no more parameters RPL_UMODEIS*/
            /***********************************************************************************************down server read sem*/
            if (!(user_mode_str=user_mode_string(client->reg_modes))) return ERROR;
            /***********************************************************************************************up server read sem*/
            irc_send_numeric_response(client, RPL_UMODEIS, user_mode_str);
            free(user_mode_str);
            return OK;
        }
        else {
            /***********************************************************************************************down server write sem*/
            user_new_mode=user_mode_from_str(target_array[prefix+2], &unknown_mode, &operation);
            if (operation=='+') {
                user_new_mode=(user_new_mode & ~(US_MODE_a | US_MODE_o | US_MODE_O)); /*cannot grant a o O modes this way*/
            }
            else user_new_mode=(user_new_mode | US_MODE_r); /*cannot set r to 0 this way*/
            if (unknown_mode) {
                irc_send_numeric_response(client, ERR_UMODEUNKNOWNFLAG, UMODEUNKNOWNFLAG_MSG);
            }
            client->reg_modes=(USER_REGISTERED|user_new_mode|client->reg_modes);
            /***********************************************************************************************up server write sem*/
            if (!(user_mode_str=user_mode_string(user_new_mode))) return ERROR;
            if (!(msg=(char*)malloc(sizeof(char)*(1 + IRC_MAX_NICK_LENGTH*2 + 1 + strlen("MODE") + 3 + strlen(user_mode_str) + strlen(IRC_MSG_END) + 1)))) {
                free(user_mode_str);
                return ERROR;
            }
            sprintf(msg, ":%s MODE %s :%s%s", client->nick, client->nick, user_mode_str, IRC_MSG_END);
            send_msg(client->socket, msg, strlen(msg), IRC_MSG_LENGTH);
            free(user_mode_str);
            free(msg);
        }
        
    }
    else if (is_valid_chname(target_array[prefix+1])) { /*target is a channel*/
        /***********************************************************************************************down write*******/
        target_chan=channel_hasht_find(target_array[prefix+1]);
        if (!target_chan) { /*no such channel*/
            if (!(msg=(char*)malloc(strlen(NOSUCHCHANNEL_MSG) + strlen(target_chan->name) + 1))) {
                return ERROR;
            }
            sprintf(msg, NOSUCHCHANNEL_MSG, target_chan->name);
            irc_send_numeric_response(client, ERR_NOSUCHCHANNEL,msg);
            free(msg);
            /***********************************************************************************************up write*******/
            return OK;
        }
        if ((n_strings-prefix)==2) { /*no more arguments*/
            if (!(ch_mode_str=chan_mode_string(target_chan->modes))) {
                /***********************************************************************************************up write*******/
                return ERROR;
            }
            passlen=((target_chan->pass==NULL)?0:strlen(target_chan->pass));
            if (!(msg=malloc(strlen(target_chan->name)+1+strlen(ch_mode_str)+1+passlen+22))) {
                /***********************************************************************************************up write*******/
                free(ch_mode_str);
                return ERROR;
            }
            sprintf(msg, "%s %s %s %u", target_chan->name, ch_mode_str, ((target_chan->pass==NULL)?"":target_chan->pass), target_chan->users_max );
            irc_send_numeric_response(client, RPL_CHANNELMODEIS, msg);
            free(ch_mode_str);
            /***********************************************************************************************up write*******/
            return OK;
        }
        else {    
            if (!find_nick_in_llist(client->nick, &(target_chan->operators_llist))) {
                irc_send_numeric_response(client, ERR_CHANOPRIVSNEEDED, ":You are not channel operator");
                return OK;
            }
            apply_modes_to_chan(client, target_chan, target_array[prefix+2], &(target_array[prefix+3]));
        }
        /***********************************************************************************************up write*******/
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
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG);
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
    
    /***************************************************************************************************************down server write semaphores*/
    if (user_hasht_find(new_nick)!=NULL) {
        irc_send_numeric_response(client, ERR_NICKNAMEINUSE, ": Nickname already in use");
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
        sprintf(success_msg, ":%s NICK %s%s", client->nick, new_nick, IRC_MSG_END);
        user_hasht_remove(client);
        strncpy(client->nick, new_nick, strlen(new_nick)+1);
        user_hasht_add(client);
        LL_FOREACH(client->channels_llist , elt) {
            chan=channel_hasht_find(elt->ch_name);
            if (remove_nick_from_llist(client->nick, &(chan->users_llist))==OK) {
                add_nick_to_llist(new_nick, &(chan->users_llist));
            }
            else if ((remove_nick_from_llist(client->nick, &(chan->operators_llist))==OK)) {
                add_nick_to_llist(new_nick, &(chan->users_llist));
            }
            send_msg_to_channel_except(client, chan, success_msg);
        }
        send_msg(client->socket, success_msg, strlen(success_msg), IRC_MSG_LENGTH);
    }    
    
    /***************************************************************************************************************up server write semaphores*/
    
    return OK;
}

int user_part_chan(user* client, channel* chan, char* leave_msg) {
    char *error_msg=NULL, *msg=NULL, *none=":", *user_msg=NULL;
    
    if (leave_msg) {
        user_msg=leave_msg;
    }
    else {
        user_msg=none;
    }
    
    if (!chan) { /*no such channel*/
        if (!(error_msg=(char*)malloc(strlen(NOSUCHCHANNEL_MSG)  + 1))) {
            return ERROR;
        }
        sprintf(error_msg, NOSUCHCHANNEL_MSG, "");
        irc_send_numeric_response(client, ERR_NOSUCHCHANNEL,error_msg);
        free(error_msg);
    }
    else if (!find_chname_in_llist(chan->name, &(client->channels_llist))) { /*not on channel*/
        if (!(error_msg=(char*)malloc(strlen(NOTONCHANNEL_MSG) + strlen(chan->name) + 1))) {
            return ERROR;
        }
        sprintf(error_msg, NOTONCHANNEL_MSG, chan->name);
        irc_send_numeric_response(client, ERR_NOTONCHANNEL,error_msg);
        free(error_msg);
    }
    else { /*on channel, leave, notify to members and if channel is empty delete it*/
        if (!(msg=(char*)malloc(1+strlen(client->nick)+1+strlen("PART")+1+strlen(chan->name)+1+strlen(user_msg)+strlen(IRC_MSG_END)+1))) {
            return ERROR;
        }
        sprintf(msg, ":%s PART %s %s%s", client->nick, chan->name, user_msg, IRC_MSG_END);
        send_msg_to_channel(chan, msg);
        remove_chname_from_llist(chan->name, &(client->channels_llist));
        remove_nick_from_llist(client->nick, &(chan->users_llist));
        remove_nick_from_llist(client->nick, &(chan->operators_llist));
        chan->users_number--;
        if (is_empty_channel(chan)) {
            channel_hasht_remove(chan);
            free_channel(chan);
        }
        free(msg);
    }
    return OK;
}

/*
 * IRC PART
 */
int irc_part_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char *chnametmp=NULL;
    char *chnametoken=NULL, *targetnm;
    channel *target_channel;
    char* msg=NULL;
    
    /*if not registered action not allowed*/
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
    if ((n_strings-prefix)<2) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG);
        return OK;
    }
    
    targetnm=target_array[prefix+1];
    if (n_strings-prefix>2) msg=target_array[prefix+2];
    
    /************************************************************************************down write semaphores*/
    while ( (chnametoken=strtok_r(targetnm, ",", &chnametmp)) ) {
        targetnm=NULL;
        if (is_valid_chname(chnametoken)) {
            target_channel=channel_hasht_find(chnametoken);
            if (target_channel) user_part_chan(client, target_channel, msg);
        }        
    }
    /************************************************************************************up write semaphores*/
    return OK;
}

int irc_quit_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char *msg=NULL;
    channel_lst *elt;
    channel* chan;
    
    
    /*split arguments*/
    split_ret_value = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    /*compose a message to notify departure*/
    if (n_strings-prefix>1) {
        if ((msg=(char*)malloc(1 + IRC_MAX_NICK_LENGTH + 1 + strlen("QUIT") + 1 + strlen(target_array[prefix+1]) + strlen(IRC_MSG_END) + 1))) { /*compose a message to notify departure*/
            sprintf(msg, ":%s QUIT %s%s", client->nick, target_array[prefix+1], IRC_MSG_END);
        }
    }
    else { /*no reason given*/
        if ((msg=(char*)malloc(1 + IRC_MAX_NICK_LENGTH + 1 + strlen("QUIT :") + strlen(IRC_MSG_END) + 1))) { /*compose a message to notify departure*/
            sprintf(msg, ":%s QUIT :%s", client->nick, IRC_MSG_END);
        }
    }
    syslog(LOG_NOTICE, "debug 2");
    /**********************************************************************************write sem down*/
    if (client->already_in_server) { /*known by the server*/
        user_hasht_remove(client);
        /*remove client from every channel he's in and notify to the rest of the users*/
        LL_FOREACH(client->channels_llist, elt) {
            chan=channel_hasht_find(elt->ch_name);
            if (chan) {
                remove_nick_from_llist(client->nick, &(chan->users_llist));
                remove_nick_from_llist(client->nick, &(chan->operators_llist));
                chan->users_number--;
                if (is_empty_channel(chan)) {
                    channel_hasht_remove(chan);
                    free_channel(chan);
                }
                else {
                    send_msg_to_channel(chan, msg);
                }
            }
        }
        send_msg(client->socket, "ERROR :Closing link", strlen("ERROR :Closing link"), IRC_MSG_LENGTH);
    }
    close_connection(client->socket);
    free_user(client);
    /**********************************************************************************write sem up*/
    pthread_exit(NULL);
}

int irc_squit_cmd (user* client, char* command) {
    /*developing version, automatically shuts down the server*/
    exit(0);
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
        if (!(full_reply=(char*)malloc(sizeof(char)*(1 + IRC_MAX_NICK_LENGTH*2 + 1 + strlen("PRIVMSG") + 1 + 1 + strlen(msg)+ 1 + strlen(IRC_MSG_END) + 1)))) {
            return ERROR;
        }
        sprintf(full_reply, ":%s PRIVMSG %s %s%s", origin->nick, target->nick, msg, IRC_MSG_END);
        if (send_msg(target->socket, full_reply, strlen(full_reply), IRC_MSG_LENGTH) == ERROR) {
            free(full_reply);
            return ERROR;
        }
        free(full_reply);
    }
    return OK;
}

int send_privmsg_to_chan (user *origin, channel *target, char* msg) {
    char* error_cantsend=NULL;
    char* full_msg=NULL;
    
    if (chan_mode_n(target->modes)) { //chan mode n can't receive message from not members
        if ((find_nick_in_llist(origin->nick, &(target->users_llist)) == NULL) && (find_nick_in_llist(origin->nick, &(target->operators_llist)) == NULL)) {
            if (!(error_cantsend=(char*)malloc((strlen(CANNOTSENDTOCHAN_MSG)+strlen(target->name)+1)*sizeof(char)))){
                syslog(LOG_NOTICE,"send_privmsg_to_chan malloc 1 error");
                return ERROR;
            }
            sprintf(error_cantsend, CANNOTSENDTOCHAN_MSG, target->name);
            irc_send_numeric_response(origin, ERR_CANNOTSENDTOCHAN, error_cantsend);
            free(error_cantsend);
            return OK;
        }
    }
    
    /*create the message*/
    if (!(full_msg=(char*)malloc(sizeof(char)*(1 + IRC_MAX_NICK_LENGTH + 1 + strlen("PRIVMSG") + 1 + strlen(target->name) + 1\
            + strlen(msg) + strlen(IRC_MSG_END) + 1)))) {
        syslog(LOG_NOTICE,"send_privmsg_to_chan malloc 2 error");
        return ERROR;
    }
    sprintf(full_msg, ":%s PRIVMSG %s %s%s", origin->nick, target->name, msg, IRC_MSG_END);
    send_msg_to_channel_except(origin, target, full_msg);
    free(full_msg);
    return OK;
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
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG);
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



int send_names (user* client, channel *target_chan) {
    char chantype='=', *chname=SERVER_NAME;
    char *details=NULL;
    active_nicks* elt;
    int length=4, count=0;
    if (target_chan) { /*send END of /NAMES*/
        if (chan_mode_p(target_chan->modes)) chantype='*';
        if (chan_mode_s(target_chan->modes)) chantype='@';
        chname=target_chan->name;
        length+=strlen(chname);
        LL_FOREACH(target_chan->operators_llist, elt) {
            length+=strlen(elt->nick)+2;
        }
        LL_FOREACH(target_chan->users_llist, elt) {
            length+=strlen(elt->nick)+1;
        }
        if (!(details=(char*)malloc(1+length+1))) {
            return ERROR;
        }
        sprintf(details, "%c %s :",chantype, chname);
        count+=strlen(details);
        LL_FOREACH(target_chan->operators_llist, elt) {
            sprintf(details+count, "@%s ", elt->nick);
            count+=strlen(elt->nick)+2;
        }
        LL_FOREACH(target_chan->users_llist, elt) {
            sprintf(details+count, "%s ", elt->nick);
            count+=strlen(elt->nick)+1;
        }
        irc_send_numeric_response(client, RPL_NAMREPLY, details);
        free(details);
        details=NULL;
    }
    
    details = (char*) malloc(strlen(chname) + 1 + strlen(":End of /NAMES list.") + 1);
    if(!details) return ERROR;
    sprintf(details, "%s :End of /NAMES list.", chname);
    irc_send_numeric_response(client, RPL_ENDOFNAMES, details);
    free(details);
    return OK;
}
/*
 * NAMES CMD
 */
int irc_names_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char *tok_tmp=NULL;
    char *token=NULL, *target;
    channel *target_channel;
    
    /*if not registered can't do this*/
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
    if ((n_strings-prefix)==1) {
        send_names(client, NULL);
        return OK;
    }
    target=target_array[prefix+1];
    
    /************************************************************************************down read semaphores*/
    while ( (token=strtok_r(target, ",", &tok_tmp)) ) {
        target=NULL;
        if (is_valid_chname(token)) { /*token is a channel_name*/
            if ((target_channel=channel_hasht_find(token))!=NULL) {
                send_names(client, target_channel);
            }
        }
    }
    /************************************************************************************up read semaphores*/
    return OK;
}


int user_join_chan(user *client,channel *chan,char *pass) {
    char *err_inviteonly, *full_reply=NULL, *chantopic;
    int removeinv=0;

    if (find_chname_in_llist(chan->name, &(client->channels_llist))) return OK;  /*if already in the channel return*/
    
    if (chan_mode_k(chan->modes)) {
        if (chan->pass) {
            if (!pass) {
                irc_send_numeric_response(client ,ERR_PASSWDMISMATCH ,":Password incorrect");
                return OK;
            }
            if (strcmp(pass, chan->pass)) {
                irc_send_numeric_response(client ,ERR_PASSWDMISMATCH ,":Password incorrect");
                return OK;
            }
        }
    }
    
    if (chan_mode_i(chan->modes)) { /*invite-only channel*/
        if (!find_nick_in_llist(client->nick, &(chan->invited_llist))) {
            if (!(err_inviteonly=(char*)malloc(sizeof(char)*(strlen(INVITEONLYCHAN_MSG) + strlen(chan->name) + 1)))) {
                syslog(LOG_NOTICE, "user_join_chan error in malloc 1");
                return ERROR;
            }
            sprintf(err_inviteonly, INVITEONLYCHAN_MSG, chan->name);
            irc_send_numeric_response(client ,ERR_INVITEONLYCHAN , err_inviteonly);
            return OK;
        }
        removeinv=1;
    }
    
    /*introducing user in channel*/
    if (add_chname_to_llist(chan->name, &(client->channels_llist))==ERROR) return ERROR;
    if (user_mode_o(client->reg_modes) || user_mode_O(client->reg_modes)) {
        if (add_nick_to_llist(client->nick, &(chan->operators_llist))==ERROR) {
            syslog(LOG_NOTICE, "debug :Entra como operador %s", client->nick);
            remove_chname_from_llist(chan->name, &(client->channels_llist));
            return ERROR;
        }
    }
    else {
        syslog(LOG_NOTICE, "debug: Entra como user %s", client->nick);
        if (add_nick_to_llist(client->nick, &(chan->users_llist))==ERROR) {
            remove_chname_from_llist(chan->name, &(client->channels_llist));
            return ERROR;
        }
    }
    (chan->users_number)++;
    
    /*Sending replies*/
    if (!(full_reply=(char*)malloc(sizeof(char)*(1 + IRC_MAX_NICK_LENGTH + 1 + strlen("JOIN") + 1 + strlen(chan->name) + strlen(IRC_MSG_END) + 1)))) {
        syslog(LOG_NOTICE, "user_join_chan error in malloc 2");
        return ERROR;
    }
    sprintf(full_reply, ":%s JOIN %s%s", client->nick, chan->name, IRC_MSG_END);
    send_msg_to_channel(chan, full_reply);
    free(full_reply);
    
    if (chan->topic) {
        if (!(chantopic=(char*)malloc(sizeof(char)*(strlen(chan->name) + 2 + strlen(chan->topic) + 2)))) {
            syslog(LOG_NOTICE, "user_join_chan error in malloc 3");
            return ERROR;
        }
        sprintf(chantopic, "%s :%s", chan->name, chan->topic);
        irc_send_numeric_response(client, RPL_TOPIC, chantopic);
        free(chantopic);
    }
    
    if (removeinv) remove_nick_from_llist(client->nick, &(chan->invited_llist));
    return OK;
}

int create_channel(user* client,char* chname) {
    channel *new=NULL;
    char *full_reply=NULL;
    
    if (!(new=(channel*)malloc(sizeof(channel)))) return ERROR;
    new->name=NULL;
    new->topic=NULL;
    new->pass=NULL;
    new->modes=CH_MODE_DEFAULT;
    new->users_number=1; 
    new->users_max=0; 
    new->users_llist=NULL;
    new->operators_llist=NULL;
    new->invited_llist=NULL;
    
    if (!(new->name=strdup(chname))) {
        free(new);
        return ERROR;
    }
    
    if (!(full_reply=(char*)malloc(sizeof(char)*(1 + IRC_MAX_NICK_LENGTH + 1 + strlen("JOIN") + 1 + strlen(new->name) + strlen(IRC_MSG_END) + 1)))) {
        free_channel(new);
        return ERROR;
    }
    sprintf(full_reply, ":%s JOIN %s%s", client->nick, new->name, IRC_MSG_END);
    send_msg(client->socket, full_reply, strlen(full_reply), IRC_MSG_LENGTH);
    free(full_reply);
    /*introducing user in channel*/
    if (add_chname_to_llist(new->name, &(client->channels_llist))!=OK) return ERROR;
    if (add_nick_to_llist(client->nick, &(new->operators_llist))!=OK) { /*creator is operator by default*/
        free_channel(new);
        return ERROR;
    }
    /*introducing channel in hash table*/
    channel_hasht_add(new);
    return OK;
}

/*
 * JOIN CMD
 */
int irc_join_cmd (user* client, char* command) {
    int prefix=0, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char *error_nochan=NULL;
    char *chnametmp=NULL, *chpasstmp=NULL;
    char *chnametoken=NULL, *chpasstoken=NULL, *targetnm, *targetpass=NULL;
    channel *target_channel;
    
    /*if not registered action not allowed*/
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
    if ((n_strings-prefix)<2) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG);
        return OK;
    }
    
    if ((target_array[prefix+1][0]=='0') && (target_array[prefix+1][1]=='\0')) { /*join 0 means leave all channels currently in*/
        
        /*call quit cmd for every channel*/
        
        return OK;
    }
    
    targetnm=target_array[prefix+1];
    if (n_strings-prefix>2) {
        targetpass=target_array[prefix+2];
    }
    /************************************************************************************down write semaphores*/
    while ( (chnametoken=strtok_r(targetnm, ",", &chnametmp)) ) {
        targetnm=NULL;
        if (is_valid_chname(chnametoken)) {
            target_channel=channel_hasht_find(chnametoken);
            if (target_channel) { /*channel exists*/
                if (n_strings-prefix>2) {
                    chpasstoken=strtok_r(targetpass, ",", &chpasstmp);
                    targetpass=NULL;
                }
                user_join_chan(client, target_channel, chpasstoken);
                send_names (client, target_channel);
            }
            else { /*channel doesn't exist, we create it*/
                create_channel(client, chnametoken);
                send_names (client, channel_hasht_find(chnametoken));
            }
            target_channel=NULL;
        }
        else { /*not a valid channel name*/
            if (!(error_nochan=(char*)malloc(sizeof(char)*(strlen(chnametoken) + 1 + strlen(ILLEGAL_CHNAME) + 1)))) {
                break;
            }
            sprintf(error_nochan, "%s %s", chnametoken, ILLEGAL_CHNAME);
            irc_send_numeric_response(client, ERR_ILLEGALCHNAME, error_nochan);
            free(error_nochan);
            error_nochan=NULL;
        }
    }
    /************************************************************************************up write semaphores*/
    return OK;
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
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG);
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
    if ((n_strings-prefix)<5) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, NEEDMOREPARAMS_MSG);
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
        requested_modes=(requested_modes & (US_MODE_DEFAULT)); 
        if (!(client->user_name=strdup(user))) {
            /**************************************************************************up write sem*/
            return ERROR;
        }
        if (realname[0]==':') realname++;
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
                        details = malloc(strlen(param) + 2 + strlen(usr_found->user_name) + 5 + strlen(usr_found->nick) + 7 + strlen(usr_found->real_name)+1);
                        if(!details) return ERROR;
                        sprintf(details, "%s ~%s - - %s H@ :0 %s", param, 
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
                        details = malloc(strlen(param) + 2 + strlen(usr_found->user_name) + 5 + strlen(usr_found->nick) + 6 + strlen(usr_found->real_name)+1);
                        if(!details) return ERROR;
                        sprintf(details, "%s ~%s - - %s H :0 %s", param, 
                                                                   usr_found->user_name, 
                                                                   usr_found->nick,
                                                                   usr_found->real_name);
                        irc_send_numeric_response(client, RPL_WHOREPLY, details);
                        free(details);
                    }
                }
            }
        }

        /**SEMAPHORE_HERE**/
        details = malloc(strlen(param) + 1 + strlen(":End of /WHO list.") + 1);
        if(!details) return ERROR;
        sprintf(details, "%s :End of /WHO list.", param);
        irc_send_numeric_response(client, RPL_ENDOFWHO, details);
        free(details);
        param = strtok_r(NULL, ",", &saveptr);
    }

    return OK;
}
    

/*
 * LIST CMD
 */
int irc_list_cmd(user *client, char *command){

    int prefix=0, n_strings, split_ret;
    char *target_array[MAX_CMD_ARGS + 2];
    char *param, *saveptr, *details;
    channel *ch_found, *tmp;

    split_ret = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret == ERROR || split_ret == ERROR_WRONG_SYNTAX){
        return ERROR;
    }

    
    if ((n_strings-prefix) == 1) {
        irc_send_numeric_response(client, RPL_LISTSTART, "Channel :Users Name");
        /*SEMAPHORE HERE*/
        
        HASH_ITER(hh, server_data.channels_hasht, ch_found, tmp){

            if(ch_found->topic){
                details = malloc(strlen(ch_found->name) + 23 + strlen(ch_found->topic) + 1);
                if(!details) return ERROR;
                sprintf(details, "%s %u :%s", ch_found->name, ch_found->users_number, ch_found->topic);
            }
            else{
                details = malloc(strlen(ch_found->name) + 24);
                if(!details) return ERROR;
                sprintf(details, "%s %u :", ch_found->name, ch_found->users_number);
            }
            
            irc_send_numeric_response(client, RPL_LIST, details);
            free(details);
            
        }
        /*SEMAPHORE HERE*/
        irc_send_numeric_response(client, RPL_LISTEND, ":End of /LIST");
    }

    else{

        param = strtok_r(target_array[prefix+1], ",", &saveptr);
        irc_send_numeric_response(client, RPL_LISTSTART, "Channel :Users Name");

        while(param){

            /**SEMAPHORE_HERE**/
            ch_found = channel_hasht_find(param);
            if(ch_found){
                if(ch_found->topic){
                    details = malloc(strlen(ch_found->name) + 23 + strlen(ch_found->topic) + 1);
                    if(!details) return ERROR;
                    sprintf(details, "%s %u :%s", ch_found->name, ch_found->users_number, ch_found->topic);
                }
                else{
                    details = malloc(strlen(ch_found->name) + 24);
                    if(!details) return ERROR;
                    sprintf(details, "%s %u :", ch_found->name, ch_found->users_number);
                }

                irc_send_numeric_response(client, RPL_LIST, details);
                free(details);
            }
            /*SEMAPHORE HERE*/

            param = strtok_r(NULL, ",", &saveptr);
            
        }
   
        irc_send_numeric_response(client, RPL_LISTEND, ":End of /LIST");
    }

    return OK;
    

}



/*
 * OPER CMD
 */
int irc_oper_cmd(user *client, char *command) {

    int prefix=0, n_strings, split_ret;
    char *target_array[MAX_CMD_ARGS + 2];
    char *details;
    channel* ch_found;
    channel_lst *chlst;

    split_ret = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret == ERROR || split_ret == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    
    if ((n_strings-prefix) < 3) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, ":Not enough parameters");
        return OK;
    }

    /*SEMAPHORE HERE*/
    if(user_mode_o(client->reg_modes) && user_mode_O(client->reg_modes)){
        return OK;
    }

    if(strcmp(target_array[1], OPER_USER) || strcmp(target_array[2], OPER_PASS)){
        irc_send_numeric_response(client, ERR_PASSWDMISMATCH, ":Password incorrect");
        return OK;
    }
    /*SEMAPHORE HERE*/

    /*WRITE SEMAPHORE HERE*/
    client->reg_modes = client->reg_modes | US_MODE_o | US_MODE_O;

    LL_FOREACH(client->channels_llist, chlst){
        ch_found = channel_hasht_find(chlst->ch_name);
        if(ch_found){
            if(remove_nick_from_llist(client->nick, &(ch_found->users_llist)) != ERROR){
                if(add_nick_to_llist(client->nick, &(ch_found->operators_llist)) == ERROR) return ERROR;
            }
        }
    }

    /*WRITE SEMAPHORE HERE*/

    details = malloc(strlen("MODE +oO") + 1 + strlen(client->nick) + 1 + strlen(":You are now an IRC operator") + 1);
    if(!details) return ERROR;
    sprintf(details, "MODE +oO %s :You are now an IRC operator", client->nick);
    irc_send_numeric_response(client, RPL_YOUREOPER, details);
    free(details);

    return OK;

}


/*
 * TOPIC CMD
 */
int irc_topic_cmd(user *client, char *command) {

    int prefix=0, n_strings, split_ret;
    char *target_array[MAX_CMD_ARGS + 2];
    char *details;
    channel* ch_found;

    split_ret = irc_split_cmd(command, (char **) &target_array, &prefix, &n_strings);

    if(split_ret == ERROR || split_ret == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    
    if ((n_strings-prefix) == 1) {
        irc_send_numeric_response(client, ERR_NEEDMOREPARAMS, ":Not enough parameters");
        return OK;
    }

    if ((n_strings-prefix) == 2){

        ch_found = channel_hasht_find(target_array[1]);
        if(ch_found){
            if(!ch_found->topic){
                details = malloc(strlen(ch_found->name) + 1 + strlen(":No topic is set") + 1);
                sprintf(details, "%s :No topic is set", ch_found->name);
                irc_send_numeric_response(client, RPL_NOTOPIC, details);
                free(details);

            }
            else{
                details = malloc(strlen(ch_found->name) + 2 + strlen(ch_found->topic) + 1);
                sprintf(details, "%s :%s", ch_found->name, ch_found->topic);
                irc_send_numeric_response(client, RPL_TOPIC, details);
                free(details);
            }
        }
        else{
            details = malloc(strlen(target_array[1]) + 1 + strlen(":No such channel") + 1);
            sprintf(details, "%s :No such channel", target_array[1]);
            irc_send_numeric_response(client, ERR_NOSUCHCHANNEL, details);
            free(details);
        }
    }

    else if ((n_strings-prefix) == 3){

        ch_found = channel_hasht_find(target_array[1]);
        if(ch_found){
            if(user_mode_o(client->reg_modes) && user_mode_O(client->reg_modes)){

                ch_found->topic = malloc(strlen(target_array[2])+1);
                if(!ch_found->topic) return ERROR;
                strcpy(ch_found->topic, target_array[2]);
            }
            else{
                if(!find_chname_in_llist(target_array[1], &(client->channels_llist))){
                    details = malloc(strlen(target_array[1]) + 1 + strlen(":You're not on that channel") + 1);
                    sprintf(details, "%s :You're not on that channel", target_array[1]);
                    irc_send_numeric_response(client, ERR_NOTONCHANNEL, details);
                    free(details);

                }
                else{
                    if(!find_nick_in_llist(client->nick, &(ch_found->operators_llist))){
                        details = malloc(strlen(target_array[1]) + 1 + strlen(":You're not channel operator") + 1);
                        sprintf(details, "%s :You're not channel operator", target_array[1]);
                        irc_send_numeric_response(client, ERR_CHANOPRIVSNEEDED, details);
                        free(details);
                    }
                    else{
                        ch_found->topic = malloc(strlen(target_array[2])+1);
                        if(!ch_found->topic) return ERROR;
                        strcpy(ch_found->topic, target_array[2]);
                    }
                }
            }
        }
        else{
            details = malloc(strlen(target_array[1]) + 1 + strlen(":No such channel") + 1);
            sprintf(details, "%s :No such channel", target_array[1]);
            irc_send_numeric_response(client, ERR_NOSUCHCHANNEL, details);
            free(details);
        }
    }

    return OK;
}