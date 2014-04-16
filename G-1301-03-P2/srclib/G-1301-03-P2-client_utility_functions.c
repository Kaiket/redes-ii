#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
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

    /************************************
     *                                  *
     *          COMMAND REPLIES         *
     *                                  *
     ************************************/

    /*Message from server*/
    if(!strcmp(command, RPL_WELCOME_STR) || !strcmp(command, RPL_YOURHOST_STR) || !strcmp(command, RPL_CREATED_STR) || 
       !strcmp(command, RPL_WHOISSERVER_STR) || !strcmp(command, RPL_WHOISIDLE_STR) || !strcmp(command, RPL_WHOISCHANNELS_STR) || 
       !strcmp(command, RPL_MOTD_STR) || !strcasecmp(command, "NOTICE")){
        interfaceText(NULL, target_array[n_strings-1], MSG_TEXT, !MAIN_THREAD);
    }

    /*JOIN command*/
    else if(!strcasecmp(command, JOIN_CMD_STR)){
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
                        if(client_send_irc_command("WHO", client_channel) == ERROR){
                            interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                        }
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

    /*NICK command*/
    else if(!strcasecmp(command, NICK_CMD_STR)){
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
                    }
                }
            }
        }

        semaphore_aw(writer, readers);
    }

    /*PART command*/
    else if(!strcasecmp(command, PART_CMD_STR)){

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

    /*QUIT command*/
    else if(!strcasecmp(command, QUIT_CMD_STR)){

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
    }

    /*PRIVMSG command*/
    else if(!strcasecmp(command, PRIVMSG_CMD_STR)){

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
    }

    /*INVITE command*/
    else if(!strcasecmp(command, INVITE_CMD_STR)){
        if (n_strings-prefix == 3){
            if(prefix && *(target_array[n_strings-1]+sizeof(char)) == '#'){
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
                    if(!strcasecmp(target_array[n_strings-2], nick)){
                        sprintf(message, "Has sido invitado por %s al canal %s.", recv_nick, target_array[n_strings-1]+sizeof(char));
                    }
                    else{
                        sprintf(message, "%s ha sido invitado al canal %s por %s.", target_array[n_strings-2], target_array[n_strings-1]+sizeof(char), recv_nick);
                    }
                    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                }
            }
        }
    }

    /*PING command*/
    else if(!strcasecmp(command, PING_CMD_STR)){

        if(n_strings-prefix == 2){
            strcpy(message, target_array[prefix+1]);
            if(client_send_irc_command("PONG", message) == ERROR){
                interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
            }
        }

    }

    /*MODE command*/
    else if(!strcasecmp(command, MODE_CMD_STR)){
        client_print_full_mesage((char **) &target_array, prefix, n_strings);
    }

    /*ERROR command*/
    else if(!strcasecmp(command, ERROR_CMD_STR)){
        semaphore_bw(writer, readers);
        in_channel = 0;
        connected = 0;
        interfaceText(NULL, "Ha sido desconectado.", MSG_TEXT, !MAIN_THREAD);
        semaphore_aw(writer, readers);
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
        if(n_strings > 2){
            if(!strcasecmp(target_array[n_strings-3], nick)){
                sprintf(message, "Comienzo de la lista de participantes del canal:");
            }
            else{
                sprintf(message, "\t%s", target_array[n_strings-3]);
            }
            interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
        }
    }

    else if(!strcmp(command, RPL_ENDOFWHO_STR)){
        interfaceText(NULL, "Fin de la lista de participantes del canal.", MSG_TEXT, !MAIN_THREAD);
    }

    /*LIST command*/
    else if(!strcmp(command, RPL_LISTSTART_STR)){
        interfaceText(NULL, "Comienzo de la lista de canales del servidor:", MSG_TEXT, !MAIN_THREAD);
    }

    else if(!strcmp(command, RPL_LIST_STR)){
        if(n_strings > 2){
            if(target_array[n_strings-1]+sizeof(char)){
                sprintf(message, "\tCanal: %s\n\tTopic: %s\n", target_array[n_strings-3], target_array[n_strings-1]+sizeof(char));
            }
            else{
                sprintf(message, "\tCanal: %s\n\tTopic: Sin topic.\n", target_array[n_strings-3]);
            }
            interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
        }
    }

    else if(!strcmp(command, RPL_LISTEND_STR)){
        interfaceText(NULL, "Fin de la lista de canales del servidor.", MSG_TEXT, !MAIN_THREAD);
    }

    /*INVITE command*/
    else if(!strcmp(command, RPL_INVITING_STR)){
        if(n_strings > 1){
            sprintf(message, "Has invitado a %s al canal %s.", target_array[n_strings-2], target_array[n_strings-1]);
        }
        interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
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
    char command[BUFFER], message[BUFFER], changed[BUFFER];

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
            strcpy(command, target_array[prefix]);

            semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
            if(strcasecmp(command, EXIT_CMD_STR) && !connected){
                interfaceText(NULL, "No está conectado a ningún servidor.", ERROR_TEXT, MAIN_THREAD);
                semaphore_ar(&readers_num, writer, mutex_rvariables);
                return;
            }
            semaphore_ar(&readers_num, writer, mutex_rvariables);
            
            /*JOIN command, particular case*/
            semaphore_bw(writer, readers);
            if(!strcasecmp(command, JOIN_CMD_STR) && in_channel && n_strings-prefix > 1){
                if(n_strings-prefix == 2){
                    if(*(target_array[prefix+1]) == '#'){
                        if(client_send_irc_command(PART_CMD_STR, client_channel) == ERROR){
                            interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                        }
                        else{
                            in_channel = 0;
                        }
                    }
                }
            }
            semaphore_aw(writer, readers);

            /*MSG command*/
            if(!strcasecmp(command, MSG_CMD_STR) && n_strings-prefix > 2){
                strcpy(command, PRIVMSG_CMD_STR);
                sprintf(changed, ":%s", target_array[prefix+2]);
                target_array[prefix+2] = changed;
            }

            /*QUERY command*/
            else if (!strcasecmp(command, QUERY_CMD_STR) && n_strings-prefix == 2){
                semaphore_bw(writer, readers);
                if(in_channel){
                    if(client_send_irc_command(PART_CMD_STR, client_channel) == ERROR){
                        interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
                    }
                    else{
                        sprintf(message, "Comienzo de mensajes con %s", target_array[n_strings-1]);
                        strcpy(client_channel, target_array[n_strings-1]);
                        interfaceText(nick, message, PUBLIC_TEXT, MAIN_THREAD);
                    }
                }
                else{
                    in_channel = 1;
                    sprintf(message, "Comienzo de mensajes con %s", target_array[n_strings-1]);
                    strcpy(client_channel, target_array[n_strings-1]);
                    interfaceText(nick, message, PUBLIC_TEXT, MAIN_THREAD);
                    
                }
                semaphore_aw(writer, readers);
                break;
            }

            /*ME command*/
            else if(!strcasecmp(command, ME_CMD_STR) && n_strings-prefix > 1){

                strcpy(message, target_array[prefix+1]);

                for(i = 2; i + prefix < n_strings; ++i){
                    strcat(message, " ");
                    strcat(message, target_array[prefix+i]);
                }

                interfaceText(nick, message, PUBLIC_TEXT, MAIN_THREAD);
                break;
            }

            /*EXIT command*/
            else if(!strcasecmp(command, EXIT_CMD_STR) && n_strings-prefix == 1){
                client_send_irc_command(QUIT_CMD_STR, NULL);
                semaphore_rm(readers);
                semaphore_rm(writer);
                semaphore_rm(mutex_access);
                semaphore_rm(mutex_rvariables);
                close(sfd);
                exit(EXIT_SUCCESS);
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