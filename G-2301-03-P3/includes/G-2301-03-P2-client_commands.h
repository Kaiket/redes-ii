#ifndef __CLIENT_COMMANDS_H
#define __CLIENT_COMMANDS_H

/**
 * @file G-2301-03-P2-client_commands.h
 * @details Functions for management incoming and outcoming irc commands.
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/

#define TIMEOUT 10000

/*
 * The following functions are use to manage the behaviour of the program
 * when receiving one of the commands decribed below.
 */

/** 
 * @brief Take the proper action when receiving a JOIN command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_join_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a NICK command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_nick_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a PART command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_part_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a KICK command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_kick_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a QUIT command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_quit_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a PRIVMSG command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_privmsg_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a INVITE command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_invite_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a PING command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_ping_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a MODE command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_mode_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a ERROR command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_error_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);


/*
 * The following functions are use to manage the behaviour of the program
 * when receiving one of the replies decribed below.
 */

/** 
 * @brief Take the proper action when receiving a WHO reply.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void rpl_who_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a LIST reply.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void rpl_list_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a INVITE reply.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void rpl_invite_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action when receiving a AWAY reply.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void rpl_away_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);


/*
 * The following functions are use to manage the behaviour of the program
 * to send one of the commands decribed below.
 */

/** 
 * @brief Take the proper action to send a QUERY command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_query_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action to send a ME command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_me_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action to send a PART command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_part_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action to send a MODE command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_mode_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action to send a SERVER command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_server_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/** 
 * @brief Take the proper action to send a EXIT command.
 *  
 * @param target_array array with the parameters split up including the command.
 * @param prefix indicates if in the array there is a prefix
 * @param n_strings size of the array.
 */
void command_exit_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);



/*Calling commands*/
/*
 * This functions are use to manage the behaviour of the program when received a
 * PCALL / PACCEPT / PCLOSE command
 */
int command_pcall_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);
int command_paccept_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);
int command_pclose_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

/*
 * This functions are use to manage the behaviour of the program when sent a
 * PCALL / PACCEPT / PCLOSE command
 */
void command_pcall_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick);
void command_paccept_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick);
void command_pclose_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick);



#endif
