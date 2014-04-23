#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include "G-2301-03-P1-types.h"
#include "G-2301-03-P1-connection.h"
#include "G-2301-03-P1-thread_handling.h"
#include "G-2301-03-P1-parser.h"
#include "G-2301-03-P1-semaphores.h"
#include "G-2301-03-P2-chat.h"
#include "G-2301-03-P2-chat_funcs.h"
#include "G-2301-03-P2-client_utility_functions.h"
#include "G-2301-03-P2-client_commands.h"

/*Chat global variables*/
extern int sfd;                      /*Socket decriptor*/
extern int connected;                /*Connected flag*/
extern int in_channel;               /*Joined to a channel flag*/
extern int server_called;            /*Server command has been written*/
extern int queried;                  /*In a queried window*/
extern int port;                     /*Port number*/
extern char nick[BUFFER];            /*Nickname*/
extern char client_channel[BUFFER];  /*Channel name*/
extern char client_server[BUFFER];   /*Server name*/

/*Sound global variables*/
extern char called_nick[BUFFER];     /*Nick of the user who has been called*/
extern char incoming_nick[BUFFER];   /*Nick of the user who has called*/
extern char my_calling_ip[BUFFER];   /*My ip for calling*/
extern char their_calling_ip[BUFFER];/*Their ip for calling*/
extern u_int16_t their_calling_port; /*Their port for calling*/

/*Semaphores variables*/
extern int readers_num;              /*Number of readers*/
extern int readers;                  /*Semaphore*/
extern int writer;                   /*Semaphore*/
extern int mutex_access;             /*Semaphore*/
extern int mutex_rvariables;         /*Semaphore*/


void *client_thread_listener(void *arg){

	int received, processed;
    char *command, *save_ptr;
    void *data;

    /*Receiving data*/
	while ((received = receive_msg(sfd, &data, IRC_MSG_LENGTH, IRC_MSG_END, strlen(IRC_MSG_END))) > 0) {

        /*Spliting messages up*/
        command = strtok_r(data, IRC_MSG_END, &save_ptr);
        if (command != NULL){
            processed = strlen(command);
        }

        /*Processing message*/
        while (processed <= received && command != NULL) {

            /*Management of received data at command variable*/

            client_receive_data_management(command);

            /*End of management*/

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

    /*Received has failed.*/
    semaphore_bw(writer, readers);
    connected = 0;
    close(sfd);
    semaphore_aw(writer, readers);

    pthread_exit(NULL);

}

/**
 * @brief Manage the received data.
 * @details Parses the commands receveid and takes the proper action informing
 * in each case to the client.
 * 
 * @param data received.
 * @return OK or ERROR, depending on the parsing or on the sending.
 */
int client_receive_data_management(char *data){

    int prefix, split_ret_value, n_strings;
    char *command, *target_array[MAX_CMD_ARGS + 2];
    char message[BUFFER];

    /*Splitting message up into parameters*/
    split_ret_value = irc_split_cmd(data, (char **) &target_array, &prefix, &n_strings);
    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }
    command = target_array[prefix];
    if(!command){
        return OK;
    }

    /************************************
     *                                  *
     *          COMMAND REPLIES         *
     *                                  *
     ************************************/


    /*Message from server*/
    /*Welcome*/
    if(!strcmp(command, RPL_WELCOME_STR)){
        semaphore_bw(writer, readers);
        strcpy(nick, target_array[n_strings-2]);
        interfaceText(NULL, target_array[n_strings-1], MSG_TEXT, !MAIN_THREAD);
        semaphore_aw(writer, readers);

    }
    /*More weolcome.*/
    if(!strcmp(command, RPL_YOURHOST_STR) || !strcmp(command, RPL_CREATED_STR) || 
       !strcmp(command, RPL_WHOISSERVER_STR) || !strcmp(command, RPL_WHOISIDLE_STR) || !strcmp(command, RPL_WHOISCHANNELS_STR) || 
       !strcmp(command, RPL_MOTD_STR) || !strcmp(command, RPL_NOWAWAY_STR) || !strcasecmp(command, NOTICE_CMD_STR)){
        interfaceText(NULL, target_array[n_strings-1], MSG_TEXT, !MAIN_THREAD);
    }

    /*JOIN command*/
    else if(!strcasecmp(command, JOIN_CMD_STR)){
        command_join_in((char **) target_array, prefix, n_strings);
    }

    /*NICK command*/
    else if(!strcasecmp(command, NICK_CMD_STR)){
        command_nick_in((char **) target_array, prefix, n_strings);
    }

    /*PART command*/
    else if(!strcasecmp(command, PART_CMD_STR)){
        command_part_in((char **) target_array, prefix, n_strings);   
    }

    /*KICK command*/
    else if(!strcasecmp(command, KICK_CMD_STR)){
        command_kick_in((char **) target_array, prefix, n_strings);   
    }


    /*QUIT command*/
    else if(!strcasecmp(command, QUIT_CMD_STR)){
        command_quit_in((char **) target_array, prefix, n_strings);   
    }

    /*PRIVMSG command*/
    else if(!strcasecmp(command, PRIVMSG_CMD_STR)){
        command_privmsg_in((char **) target_array, prefix, n_strings);   
    }

    /*INVITE command*/
    else if(!strcasecmp(command, INVITE_CMD_STR)){
        command_invite_in((char **) target_array, prefix, n_strings);   
    }

    /*PING command*/
    else if(!strcasecmp(command, PING_CMD_STR)){
        command_ping_in((char **) target_array, prefix, n_strings);
    }

    /*MODE command*/
    else if(!strcasecmp(command, MODE_CMD_STR)){
        command_mode_in((char **) target_array, prefix, n_strings);
    }

    /*ERROR command*/
    else if(!strcasecmp(command, ERROR_CMD_STR)){
        command_error_in((char **) target_array, prefix, n_strings);
    }

    /************************************
     *                                  *
     *          NUMERIC REPLIES         *
     *                                  *
     ************************************/

    /*WHOIS command*/

    else if(!strcmp(command, RPL_WHOISUSER_STR)){
        interfaceText(NULL, "Comienzo de información sobre usuario:", MSG_TEXT, !MAIN_THREAD);
        interfaceText(NULL, target_array[n_strings-1], MSG_TEXT, !MAIN_THREAD);
    }

    else if(!strcmp(command, RPL_ENDOFWHOIS_STR)){
        interfaceText(NULL, "Fin de información sobre usuario", MSG_TEXT, !MAIN_THREAD);
    }

    /*WHO command*/
    else if(!strcmp(command, RPL_WHOREPLY_STR)){
        rpl_who_in((char **) target_array, prefix, n_strings);
    }

    else if(!strcmp(command, RPL_ENDOFWHO_STR)){
        interfaceText(NULL, "Fin de la lista de participantes del canal.", MSG_TEXT, !MAIN_THREAD);
    }

    /*LIST command*/
    else if(!strcmp(command, RPL_LISTSTART_STR)){
        interfaceText(NULL, "Comienzo de la lista de canales del servidor:", MSG_TEXT, !MAIN_THREAD);
    }

    else if(!strcmp(command, RPL_LIST_STR)){
        rpl_list_in((char **) target_array, prefix, n_strings);
    }

    else if(!strcmp(command, RPL_LISTEND_STR)){
        interfaceText(NULL, "Fin de la lista de canales del servidor.", MSG_TEXT, !MAIN_THREAD);
    }

    /*INVITE command*/
    else if(!strcmp(command, RPL_INVITING_STR)){
        rpl_invite_in((char **) target_array, prefix, n_strings);
    }

    /*AWAY command*/
    else if(!strcmp(command, RPL_AWAY_STR)){
        rpl_away_in((char **) target_array, prefix, n_strings);
    }

    else if(strlen(command) == 3){
        if(atoi(command) >= atoi(ERR_NOSUCHNICK_STR) && n_strings - prefix >= 2){
            sprintf(message, "%s %s", target_array[prefix], target_array[n_strings-1]);
            interfaceText(NULL, message, ERROR_TEXT, !MAIN_THREAD);
        }
        if(!strcmp(command, ERR_NICKNAMEINUSE_STR)){
            interfaceText(NULL, "Utilice el comando NICK para obtener un nuevo nick", MSG_TEXT, !MAIN_THREAD);
        }
    }

    return OK;

}

void client_print_full_mesage(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

    int i;
    char message[BUFFER];

    strcpy(message, target_array[prefix]);
    /*Cat all the messages to a single one*/
    for(i = 1; i + prefix < n_strings; ++i){
        strcat(message, " ");
        strcat(message, target_array[prefix+i]);
    }
    
    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);

}

void client_cmd_parsing(char *string, int type){

    int i, prefix, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char command[BUFFER], message[BUFFER], changed[BUFFER];

    /*Switch to types: message or command*/
    switch (type){
        case IRC_MSG:
            semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
            /*Not in a channel*/
            if(!in_channel){
                interfaceText(NULL, "No está participando en ningún canal.", ERROR_TEXT, MAIN_THREAD);
            }
            /*In a channel*/
            else{
                sprintf(message, "%s :%s", client_channel, string);
                if(client_send_irc_command("PRIVMSG", message) == ERROR){
                    interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                }
                else{
                    interfaceText(nick, string, PRIVATE_TEXT, MAIN_THREAD);
                }
                
            }
            semaphore_ar(&readers_num, writer, mutex_rvariables);
            break;

        case IRC_CMD:
            /*Split messages up*/
            split_ret_value = irc_split_cmd(string, (char **) &target_array, &prefix, &n_strings);
            if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
                interfaceText(NULL, "Comando no reconcido.", ERROR_TEXT, MAIN_THREAD);
                return;
            }
            strcpy(command, target_array[prefix]);

            /*Case that the command is not equal to a exit command and the user is not connected*/
            semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
            if(strcasecmp(command, EXIT_CMD_STR) && strcasecmp(command, SERVER_CMD_STR) && !connected){
                interfaceText(NULL, "No está conectado a ningún servidor.", ERROR_TEXT, MAIN_THREAD);
                semaphore_ar(&readers_num, writer, mutex_rvariables);
                return;
            }
            semaphore_ar(&readers_num, writer, mutex_rvariables);
            
            /*JOIN command, particular case MUST be implemented here*/
            semaphore_bw(writer, readers);
            if(!strcasecmp(command, JOIN_CMD_STR) && in_channel && !queried){
                if(n_strings-prefix == 2){
                    if(*(target_array[prefix+1]) == '#' || *(target_array[prefix+1]) == '&'){
                        /*Part is sent*/
                        if(client_send_irc_command(PART_CMD_STR, client_channel) == ERROR){
                            interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                        }
                        else{
                            /*Not in a channel*/
                            in_channel = 0;
                        }
                    }
                }
                else{
                    interfaceText(NULL, "JOIN: uso incorrecto", ERROR_TEXT, MAIN_THREAD);
                    break;
                }
            }
            semaphore_aw(writer, readers);

            /*MSG command, particular case MUST be implemented here*/
            if(!strcasecmp(command, MSG_CMD_STR)){
                if (n_strings-prefix > 2){
                    strcpy(command, PRIVMSG_CMD_STR);
                    sprintf(changed, ":%s", target_array[prefix+2]);
                    target_array[prefix+2] = changed;
                    /*Printing the message*/
                    strcpy(message, target_array[prefix+2]);
                    for(i = prefix+3; i < n_strings; ++i){
                        strcat(message, " ");
                        strcat(message, target_array[i]);
                    }
                    interfaceText(nick, message, PRIVATE_TEXT, MAIN_THREAD);
                }
                else{
                    interfaceText(NULL, "MSG: uso incorrecto", ERROR_TEXT, MAIN_THREAD);
                }
            }

            /*QUERY command*/
            else if (!strcasecmp(command, QUERY_CMD_STR)){
                command_query_out((char **) target_array, prefix, n_strings);
                break;
            }

            /*ME command*/
            else if(!strcasecmp(command, ME_CMD_STR)){
                command_me_out((char **) target_array, prefix, n_strings);
                break;
            }

            /*PART command*/
            else if(!strcasecmp(command, PART_CMD_STR)){
                command_part_out((char **) target_array, prefix, n_strings);
            }

            /*MODE command*/
            else if(!strcasecmp(command, BAN_CMD_STR)){
                command_mode_out((char **) target_array, prefix, n_strings);
                break;
            }

            /*SERVER command*/
            else if(!strcasecmp(command, SERVER_CMD_STR)){
                command_server_out((char **) target_array, prefix, n_strings);
                break;
            }

            /*EXIT command*/
            else if(!strcasecmp(command, EXIT_CMD_STR)){
                command_exit_out((char **) target_array, prefix, n_strings);
                break;
            }

            else if(!strcasecmp(command, PCALL_CMD_STR)){
                if(command_pcall_out((char **) target_array, prefix, n_strings) == ERROR){
                    interfaceText(NULL, "PCALL: uso incorrecto", ERROR_TEXT, MAIN_THREAD);
                }
                break;
            }

            else if(!strcasecmp(command, PACCEPT_CMD_STR)){
                if(command_paccept_out((char **) target_array, prefix, n_strings) == ERROR){
                    interfaceText(NULL, "PACCEPT: uso incorrecto", ERROR_TEXT, MAIN_THREAD);
                }
                break;
            }

            else if(!strcasecmp(command, PCLOSE_CMD_STR)){
                if(command_pclose_out((char **) target_array, prefix, n_strings) == ERROR){
                    interfaceText(NULL, "PCLOSE: uso incorrecto", ERROR_TEXT, MAIN_THREAD);
                }
                break;
            }

            /*Any other command is sent just like that*/
            if(prefix + 1 < n_strings){

                strcpy(message, target_array[prefix+1]);

                for(i = 2; i + prefix < n_strings; ++i){
                    strcat(message, " ");
                    strcat(message, target_array[prefix+i]);
                }

                if(client_send_irc_command(command, message) == ERROR){
                    interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                }
            } 

            else{
                if(client_send_irc_command(command, NULL) == ERROR){
                    interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                }
            }

            break;

        default:
            break;
    }

}


int client_connect_to_server(char* server_url, int port, void* (*thread_routine) (void *arg)){

	if((sfd = connect_to_server(server_url, port, thread_routine)) == ERROR){
    	return ERROR;
    }

    interfaceText(NULL, "Conectado, esperando respuesta...", MSG_TEXT, MAIN_THREAD);
	return OK;
}


int client_new_session(char *nick, char *name, char *real_name, char *server_url){

	char message[BUFFER];

    /*NICK message*/
	if (client_send_irc_command("NICK", nick) == ERROR){
		return ERROR;
	}
	
    /*USER message*/
	sprintf(message, "%s %s %s :%s", name, name, server_url, real_name);
	if (client_send_irc_command("USER", message) == ERROR){
		return ERROR;
	}

	return OK;
}


int client_send_irc_command(char *command, char *parameters){

	char message[BUFFER];

    if(!parameters){
        sprintf(message, "%s\r\n", command);
        return send_msg(sfd, (void *) message, strlen(message)+1, IRC_MSG_LENGTH);
    }

	sprintf(message, "%s %s\r\n", command, parameters);
	return send_msg(sfd, (void *) message, strlen(command)+strlen(parameters)+3, IRC_MSG_LENGTH);

}

int client_check_full_connection(){
    semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if(!connected){
        interfaceText(NULL, "No está conectado a ningún servidor.", ERROR_TEXT, MAIN_THREAD);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
        return FALSE;
    }
    else if (!in_channel){
        interfaceText(NULL, "No está participando en ningún canal.", ERROR_TEXT, MAIN_THREAD);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
        return FALSE;
    }
    else if(queried){
        interfaceText(NULL, "No está participando en ningún canal.", ERROR_TEXT, MAIN_THREAD);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
        return FALSE;
    }
    semaphore_ar(&readers_num, writer, mutex_rvariables);
    return TRUE;
}