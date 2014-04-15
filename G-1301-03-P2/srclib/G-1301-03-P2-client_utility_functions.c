#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-connection.h"
#include "G-1301-03-P1-thread_handling.h"
#include "G-1301-03-P1-parser.h"
#include "G-1301-03-P1-semaphores.h"
#include "G-1301-03-P2-chat.h"
#include "G-1301-03-P2-chat_funcs.h"
#include "G-1301-03-P2-client_utility_functions.h"

extern int sfd;
extern int connected;
extern int in_channel;
extern char nick[BUFFER];
extern char client_channel[BUFFER];
extern int readers_num;
extern int readers;
extern int writer;
extern int mutex_access;
extern int mutex_rvariables;


void *client_thread_listener(void *arg){

	int received, processed;
    char *command, *save_ptr;
    void *data;

	while ((received = receive_msg(sfd, &data, IRC_MSG_LENGTH, IRC_MSG_END, strlen(IRC_MSG_END))) > 0) {

        command = strtok_r(data, IRC_MSG_END, &save_ptr);
        if (command != NULL){
            processed = strlen(command);
        }

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

    semaphore_bw(writer, readers);
    connected = 0;
    semaphore_aw(writer, readers);

    pthread_exit(NULL);

}


int client_receive_data_management(char *data){

    int prefix, split_ret_value, n_strings;
    char *command, *target_array[MAX_CMD_ARGS + 2];
    char *exclam;
    char message[BUFFER], recv_nick[BUFFER];

    split_ret_value = irc_split_cmd(data, (char **) &target_array, &prefix, &n_strings);
    if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
        return ERROR;
    }

    command = target_array[prefix];

    if(!command){
        return OK;
    }

    if(!strcmp(command, "001") || !strcmp(command, "002") || !strcmp(command, "003") || 
       !strcmp(command, "372") || !strcasecmp(command, "NOTICE")){
        interfaceText(NULL, target_array[n_strings-1], MSG_TEXT, !MAIN_THREAD);
    }

    else if(!strcasecmp(command, "JOIN")){
        semaphore_bw(writer, readers);

        if (n_strings-prefix == 2){
            if(prefix && *(target_array[prefix+1]+sizeof(char)) == '#'){
                strcpy(recv_nick, target_array[0]+sizeof(char));
                exclam = strchr(recv_nick, (int) '!');
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '%');
                }
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '@');
                }
                if(exclam){
                    *exclam = '\0';
                    strcpy(client_channel, target_array[prefix+1]+sizeof(char));
                    if(!strcasecmp(recv_nick, nick)){
                        sprintf(message, "Has entrado en el canal %s.", client_channel);
                        interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                        in_channel = 1;
                    }
                    else{
                        sprintf(message, "%s ha entrado en el canal %s.", recv_nick, client_channel);
                        interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                    }
                }
            }
        }

        semaphore_aw(writer, readers);
    }

    else if(!strcasecmp(command, "PART")){

        semaphore_bw(writer, readers);

        if (n_strings-prefix >= 2){
            if(prefix && *(target_array[prefix+1]) == '#'){
                strcpy(recv_nick, target_array[0]+sizeof(char));
                exclam = strchr(recv_nick, (int) '!');
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '%');
                }
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '@');
                }
                if(exclam){
                    *exclam = '\0';
                    strcpy(client_channel, target_array[prefix+1]);
                    if(!strcasecmp(recv_nick, nick)){
                        if(n_strings-prefix > 2){
                            if(target_array[prefix+2]+sizeof(char)){
                                sprintf(message, "Has salido del canal %s. (%s).", client_channel, target_array[prefix+2]+sizeof(char));
                            }
                        }
                        else{
                            sprintf(message, "Has salido del canal %s.", client_channel);
                        }
                        interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                        in_channel = 0;
                    }
                    else{
                        if(n_strings-prefix > 2){
                            if(target_array[prefix+2]+sizeof(char)){
                                sprintf(message, "%s ha abandonado el canal (%s)", recv_nick, target_array[prefix+1]+sizeof(char));
                            }
                        }
                        else{
                            sprintf(message, "%s ha abandonado el canal.", recv_nick);
                        }
                        interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                    }
                }
            }
        }

        semaphore_aw(writer, readers);
    }

    else if(!strcasecmp(command, "QUIT")){

        semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);

        if (n_strings-prefix >= 2){
            if(prefix){
                strcpy(recv_nick, target_array[0]+sizeof(char));
                exclam = strchr(recv_nick, (int) '!');
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '%');
                }
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '@');
                }
                if(exclam){
                    *exclam = '\0';
                    if(target_array[prefix+1]+sizeof(char)){
                        sprintf(message, "%s ha abandonado el canal (%s)", recv_nick, target_array[prefix+1]+sizeof(char));
                    }
                    else{
                        sprintf(message, "%s ha abandonado el canal.", recv_nick);
                    }
                    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                }
            }
        }

        semaphore_ar(&readers_num, writer, mutex_rvariables);
    }

    else if(!strcasecmp(command, "PRIVMSG")){

        semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);

        if (n_strings-prefix == 3){
            if(prefix){
                strcpy(recv_nick, target_array[0]+sizeof(char));
                exclam = strchr(recv_nick, (int) '!');
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '%');
                }
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '@');
                }
                if(exclam){
                    *exclam = '\0';
                    strcpy(message, target_array[prefix+2]+sizeof(char));
                    if(*(target_array[prefix+1]) == '#'){
                        interfaceText(recv_nick, message, PUBLIC_TEXT, !MAIN_THREAD);
                    }
                    else{
                        interfaceText(recv_nick, message, PRIVATE_TEXT, !MAIN_THREAD);
                    }
                }
            }
        }

        semaphore_ar(&readers_num, writer, mutex_rvariables);

    }

    else if(!strcasecmp(command, "NICK")){
        semaphore_bw(writer, readers);

        if (n_strings-prefix == 2){
            if(prefix){
                strcpy(recv_nick, target_array[0]+sizeof(char));
                exclam = strchr(recv_nick, (int) '!');
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '%');
                }
                if(!exclam){
                    exclam = strchr(recv_nick, (int) '@');
                }
                if(exclam){
                    *exclam = '\0';
                    if(!strcasecmp(recv_nick, nick)){
                        strcpy(nick, target_array[prefix+1]+sizeof(char));
                        sprintf(message, "Tu nick ha cambiado a %s.", nick);
                        interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                        in_channel = 1;
                    }
                }
            }
        }

        semaphore_aw(writer, readers);
    }

    else if(!strcasecmp(command, "PING")){

        if(n_strings-prefix == 2){
            strcpy(message, target_array[prefix+1]);
            if(client_send_irc_command("PONG", message) == ERROR){
                interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
            }
        }

    }

    else if(!strcasecmp(command, "MODE")){
        client_print_full_mesage((char **) &target_array, prefix, n_strings);
    }

    else if(!strcasecmp(command, "ERROR")){
        semaphore_bw(writer, readers);
        connected = 0;
        interfaceText(NULL, "Ha sido desconectado.", MSG_TEXT, !MAIN_THREAD);
        semaphore_aw(writer, readers);
    }

    return OK;

}

void client_print_full_mesage(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

    int i;
    char message[BUFFER];

    strcpy(message, target_array[prefix]);

    for(i = 1; i + prefix < n_strings; ++i){
        strcat(message, " ");
        strcat(message, target_array[prefix+i]);
    }
    
    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);

}

void client_cmd_parsing(char *string, int type){

    int i, prefix, n_strings, split_ret_value;
    char *target_array[MAX_CMD_ARGS + 2];
    char message[BUFFER];

    switch (type){
        case IRC_MSG:
            semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
            if(!in_channel){
                interfaceText(NULL, "No está participando en ningún canal.", ERROR_TEXT, MAIN_THREAD);
            } 
            else{
                sprintf(message, "%s :%s", client_channel, string);
                if(client_send_irc_command("PRIVMSG", message) == ERROR){
                    interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                }
                else{
                    interfaceText(nick, string, PUBLIC_TEXT, MAIN_THREAD);
                }
                
            }
            semaphore_ar(&readers_num, writer, mutex_rvariables);
            break;

        case IRC_CMD:
            split_ret_value = irc_split_cmd(string, (char **) &target_array, &prefix, &n_strings);
            if(split_ret_value == ERROR || split_ret_value == ERROR_WRONG_SYNTAX){
                interfaceText(NULL, "Comando no reconcido.", ERROR_TEXT, MAIN_THREAD);
                return;
            }
            
            semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
            if(!strcasecmp(target_array[prefix], "JOIN") && in_channel){
                if(n_strings-prefix == 2){
                    if(*(target_array[prefix+1]) == '#'){
                        if(client_send_irc_command("PART", client_channel) == ERROR){
                            interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                        }
                    }
                }
            }
            semaphore_ar(&readers_num, writer, mutex_rvariables);

            if(prefix + 1 < n_strings){

                strcpy(message, target_array[prefix+1]);

                for(i = 2; i + prefix < n_strings; ++i){
                    strcat(message, " ");
                    strcat(message, target_array[prefix+i]);
                }

                if(client_send_irc_command(target_array[prefix], message) == ERROR){
                    interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                }
            } 

            else{
                if(client_send_irc_command(target_array[prefix], NULL) == ERROR){
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

    if(!parameters){
        sprintf(message, "%s\r\n", command);
        return send_msg(sfd, (void *) message, strlen(message)+1, IRC_MSG_LENGTH);
    }

	sprintf(message, "%s %s\r\n", command, parameters);
	return send_msg(sfd, (void *) message, strlen(command)+strlen(parameters)+3, IRC_MSG_LENGTH);

}