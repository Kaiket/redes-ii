#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-connection.h"
#include "G-1301-03-P1-thread_handling.h"
#include "G-1301-03-P1-parser.h"
#include "G-1301-03-P1-ircserver.h"
#include "G-1301-03-P2-chat.h"
#include "G-1301-03-P2-chat_funcs.h"
#include "G-1301-03-P2-client_utility_functions.h"


void *client_thread_listener(void *arg){

	int received, processed, command_pos, command_num;
    char *command, *save_ptr;
    void *data;
    char *AAAAAAA;
    Thread_handler *settings = (Thread_handler *) arg;

	while ((received = receive_msg(sfd, &data, IRC_MSG_LENGTH, IRC_MSG_END, strlen(IRC_MSG_END))) > 0) {

        command = strtok_r(data, IRC_MSG_END, &save_ptr);
        if (command != NULL){
            processed = strlen(command);
        }

        while (processed <= received && command != NULL) {

AAAAAAA = (char*) data+received;
*AAAAAAA = '\0';
interfaceText(NULL, data, MSG_TEXT, !MAIN_THREAD);

            /*
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
            */
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

}


int client_connect_to_server(char* server_url, int port, void* (*thread_routine) (void *arg)){

	char message[BUFFER];

	if((sfd = connect_to_server(server_url, port, thread_routine)) == ERROR){
    	return ERROR;
    }

    interfaceText(NULL, "Conectado", MSG_TEXT, MAIN_THREAD);
	return OK;
}

int client_new_session(char *nick, char *name, char *real_name, char *server_url){

	char message[BUFFER];

	if (client_send_irc_command("NICK", nick) == ERROR){
		return ERROR;
	}
	
	sprintf(message, "%s %s %s :%s", name, name, server_url, real_name);
	if (client_send_irc_command("USER", message) == ERROR){
		return ERROR;
	}

	return OK;
}


int client_send_irc_command(char *command, char *parameters){


	char message[BUFFER];

	sprintf(message, "%s %s\r\n", command, parameters);

	return send_msg(sfd, (void *) message, strlen(command)+strlen(parameters)+3, IRC_MSG_LENGTH);

}