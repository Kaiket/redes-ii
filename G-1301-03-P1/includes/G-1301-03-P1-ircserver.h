#ifndef __IRCSERVER_H
#define __IRCSERVER_H

#include <limits.h>
#include "uthash.h"
#include "utlist.h"

/*server info*/
#define SERVER_NAME "irc_server_V1.0"
#define WELCOME_MSG ":Welcome to this IRC server %s"
#define HOST_MSG ":Your host is irc_server_V1.0, running version 1.0"
#define DATE_MSG ":This server was created in February 2014"
#define MYINFO_MSG "irc_server_V1.0 1.0 sOorwia lktio"

/*server replies*/
#define CANNOTSENDTOCHAN_MSG ":Cannot send to channel %s"
#define NICK_NFOUND_MSG "%s :No such nick/channel"
#define USER_AWAY_MSG ":User %s is away"
#define ILLEGAL_CHNAME ":Illegal channel name"
#define INVITEONLYCHAN_MSG "%s :Cannot join channel (+i)"
#define NOSUCHCHANNEL_MSG "%s :No such channel"
#define NOTONCHANNEL_MSG "%s :You're not on that channel"
#define UMODEUNKNOWNFLAG_MSG ":Unknown MODE flag"
#define USERSDONTMATCH_MSG ":Cannot change mode for other users"
#define UNKNOWNMODE_MSG "x :is unknown mode char to me for %s"
#define NEEDMOREPARAMS_MSG ":Need more parameters"
#define CHANOPRIVSNEEDED_MSG "%s :You're not channel operator"
#define NOSUCHNICK_MSG NICK_NFOUND_MSG
#define USERONCHANNEL_MSG "%s %s :is already on channel"
#define NOPRIVILEGES_MSG ":Permission Denied- You're not an IRC operator"
#define CHANNELISFULL_MSG "%s :Cannot join channel (+l)"

/*server constants*/
#define SERVER_NAME_LENGTH 15
#define SERVER_LOG_IDENT "IRC_SERVER"
#define SERVER_MAX_CONNECTIONS INT_MAX
#define IRC_DEFAULT_PORT 6667
#define IRC_MSG_LENGTH 512
#define MAX_CMD_ARGS 15
#define IRC_BLANK 0x20
#define IRC_PREFIX ':'
#define IRC_MSG_END "\r\n"
#define IRC_NR_LEN 3
#define IRC_MAX_NICK_LENGTH 9
#define IRC_MAX_PASS_LENGTH 23
#ifndef ERROR
        #define ERROR -1
#endif
#define ERROR_WRONG_SYNTAX -2
#define ERROR_MAX_ARGS -3

/*CHANNEL MODES FLAGS*/
#define CH_MODE_I 1
#define CH_MODE_e 2
#define CH_MODE_b 4
#define CH_MODE_l 8
#define CH_MODE_k 16
#define CH_MODE_t 32
#define CH_MODE_r 64
#define CH_MODE_s 128
#define CH_MODE_p 256
#define CH_MODE_q 512
#define CH_MODE_n 1024
#define CH_MODE_m 2048
#define CH_MODE_i 4096
#define CH_MODE_a 8192
#define CH_MODE_v 16384
#define CH_MODE_o 32768
#define CH_MODE_O 65536
#define CH_MODE_NUMBER 17
#define CH_MODE_DEFAULT (CH_MODE_O | CH_MODE_o | CH_MODE_n | CH_MODE_t )

/*USER MODES FLAGS*/
#define US_MODE_a 1
#define US_MODE_w 2
#define US_MODE_i 4
#define US_MODE_r 8
#define US_MODE_o 16
#define US_MODE_O 32
#define US_MODE_s 64
#define USER_REGISTERED 128
#define US_MODE_NUMBER 7
#define US_MODE_DEFAULT (US_MODE_i | US_MODE_s | US_MODE_w)

/*user and key for oper command*/
#define OPER_USER "someuser"
#define OPER_PASS "somepass"

/*
 * linked list of nicks struct
 */
typedef struct t {
    char nick[IRC_MAX_NICK_LENGTH + 1];
    struct t *next;
} invite, active_nicks;

/*
 * linked list of channel names struct
 */
typedef struct ch {
    char *ch_name;
    struct ch *next;
} channel_lst;

/*
 * main struct for a user
 */
typedef struct {
    int socket;
    char nick[IRC_MAX_NICK_LENGTH + 1]; /*maximum nick length + \0*/
    char* user_name;
    char* host_name;
    char* server_name;
    char* real_name;
    char reg_modes; /*More significative bit indicates registered, the rest are for flag modes: sOoriwa */
    char already_in_server; /*if a particular instance is already in the hash table of clients*/
    channel_lst* channels_llist;
    UT_hash_handle hh;
} user;

/*
 * main struct for a channel
 * 
 */
typedef struct {
    char* name;
    char* topic;
    char* pass;
    unsigned int modes; /*0...0, OovaimnqpsrtklbeI*/
    unsigned int users_number;
    unsigned int users_max; /*if flag l is 1*/
    active_nicks* users_llist;
    active_nicks* operators_llist;
    invite* invited_llist; /*linked list of nicks invited to channel*/
    UT_hash_handle hh;
} channel;

/*
 * server data global variable
 * 
 * contains a hash table for users, a hash table for channels and semaphores to ensure safe reading/writing
 */
struct {
    channel* channels_hasht;
    int readers_num, readers, writer, mutex_access, mutex_rvariables;
    user* users_hasht;
} server_data;

/*
 * Function: irc_server_data_init
 * Parameters:
 * 
 * Description:
 *      Initializes server_data global variable, creating semaphores and setting hash tables to NULL
 * Return value:
 *      
 */
void irc_server_data_init();


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

void free_channel(channel* ch);
void free_user(user* us);
void irc_server_data_free();



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
int irc_split_cmd(char *cmd, char **target_array, int *prefix, int *n_strings);

/*
 * returns the position in the array of the first character of the commmand (ignoring prefix and blanks)
 * return ERROR if the string is invalid (contains a prefix and a last argument without a command)
 */
int irc_get_cmd_position(char* cmd);

void *irc_thread_routine(void *arg);
void irc_exit_message();
int irc_split_cmd(char *cmd, char *target_array[MAX_CMD_ARGS + 2], int *prefix, int *n_strings);
int irc_get_cmd_position(char* cmd);
int exec_cmd(int number, user* client, char *msg);
int irc_send_numeric_response(user* client, int numeric_response, char *details);


/*
 * Each of the following functions execute an irc command
 * Parameters:
 *      client: user struct which wants to execute the command
 *      command: command line following rfc specs for such command
 * Details:
 *      Executes the command extracting required parameters from the string passed.
 *      Command execution follows IRC RFC specification, sending confirmation/error/replies IRC messages to targets.
 *      These functions only return ERROR when the command string is not well formed (not enough parameters) or when errors regarding memory allocation
 *      ocurred.
 *      Each function ensures mutex.
 */
int irc_ping_cmd(user *client, char *command);
int irc_mode_cmd(user *client, char *command);
int irc_nick_cmd(user *client, char *command);
int irc_pass_cmd(user *client, char *command);
int irc_user_cmd(user* client, char* command);
int irc_privmsg_cmd(user* client, char* command);
int irc_oper_cmd(user *client, char *command);
int irc_names_cmd(user* client, char* command);
int irc_join_cmd(user* client, char* command);
int irc_list_cmd(user *client, char *command);
int irc_topic_cmd(user *client, char *command);
int irc_who_cmd(user *client, char *command);
int irc_part_cmd(user* client, char* command);
int irc_quit_cmd(user *client, char *command);
int irc_squit_cmd(user *client, char *command);
int irc_invite_cmd (user* client, char* command);

#endif