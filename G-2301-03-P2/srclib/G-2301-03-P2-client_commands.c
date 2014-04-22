#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include "G-2301-03-P1-types.h"
#include "G-2301-03-P1-semaphores.h"
#include "G-2301-03-P2-chat.h"
#include "G-2301-03-P2-chat_funcs.h"
#include "G-2301-03-P2-client_utility_functions.h"
#include "G-2301-03-P2-client_commands.h"
#include "G-2301-03-P2-call_funcs.h"
#include "G-2301-03-P1-connection.h"

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


void command_join_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

    char *exclam;
    char message[BUFFER], recv_nick[BUFFER];

	semaphore_bw(writer, readers);

    if (n_strings-prefix == 2){
        /*Parameter must be a channel*/
        if(prefix && (*(target_array[prefix+1]+sizeof(char)) == '#' || *(target_array[prefix+1]+sizeof(char)) == '&')){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            /*Getting user*/
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
                /*Checking user*/
                if(!strcasecmp(recv_nick, nick)){
                    sprintf(message, "Has entrado en el canal %s.", client_channel);
                    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                    in_channel = 1;
                    queried = 0;
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


void command_nick_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char *exclam;
    char message[BUFFER], recv_nick[BUFFER];

	semaphore_bw(writer, readers);

	if (n_strings-prefix == 2){
        if(prefix){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            /*Getting user*/
            exclam = strchr(recv_nick, (int) '!');
            if(!exclam){
                exclam = strchr(recv_nick, (int) '%');
            }
            if(!exclam){
                exclam = strchr(recv_nick, (int) '@');
            }
            if(exclam){
                *exclam = '\0';
                /*Checking user*/
                if(!strcasecmp(recv_nick, nick)){
                    strcpy(nick, target_array[prefix+1]+sizeof(char));
                    sprintf(message, "Tu nick ha cambiado a %s.", nick);
                    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                }
                else{
                    strcpy(nick, target_array[prefix+1]+sizeof(char));
                    sprintf(message, "%s ha cambiado su nick a %s.", recv_nick, target_array[n_strings-1]+sizeof(char));
                    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                }
            }
        }
    }

    semaphore_aw(writer, readers);
}

void command_part_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char *exclam;
    char message[BUFFER], recv_nick[BUFFER], more_info[BUFFER];

	semaphore_bw(writer, readers);

    if (n_strings-prefix >= 2){
        if(prefix && (*(target_array[prefix+1]) == '#' || *(target_array[prefix+1]) == '&')){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            /*Getting user*/
            exclam = strchr(recv_nick, (int) '!');
            if(!exclam){
                exclam = strchr(recv_nick, (int) '%');
            }
            if(!exclam){
                exclam = strchr(recv_nick, (int) '@');
            }
            if(exclam){
                *exclam = '\0';
                strcpy(more_info, target_array[prefix+1]);
                /*Checking user*/
                if(!strcasecmp(recv_nick, nick)){
                    /*Checking parameters*/
                    if(n_strings-prefix > 2){
                        if(target_array[prefix+2]+sizeof(char)){
                            sprintf(message, "Has salido del canal %s. (%s).", more_info, target_array[prefix+2]+sizeof(char));
                        }
                    }
                    else{
                        sprintf(message, "Has salido del canal %s.", more_info);
                    }
                    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                }
                else{
                    /*Checking parameters*/
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

void command_kick_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char *exclam;
    char message[BUFFER], recv_nick[BUFFER], more_info[BUFFER];

	semaphore_bw(writer, readers);

    if (n_strings-prefix >= 2){
        if(prefix && (*(target_array[prefix+1]) == '#' || *(target_array[prefix+1]) == '&')){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            /*Getting user*/
            exclam = strchr(recv_nick, (int) '!');
            if(!exclam){
                exclam = strchr(recv_nick, (int) '%');
            }
            if(!exclam){
                exclam = strchr(recv_nick, (int) '@');
            }
            if(exclam){
                *exclam = '\0';
                strcpy(more_info, target_array[prefix+1]);
                /*You have kicked someone*/
                if(!strcasecmp(recv_nick, nick)){
                    if(!strcasecmp(target_array[prefix+2], nick)){
                        sprintf(message, "Te has expulsado a ti mismo del canal %s", more_info);
                        queried = 0;
                        in_channel = 0;
                    }
                    else{
                        sprintf(message, "Has expulsado a %s del canal %s", target_array[prefix+2], more_info);
                        queried = 0;
                        in_channel = 0;
                    }
                    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                }
                /*Someone has kick some other*/
                else{
                    if(!strcasecmp(target_array[prefix+2], nick)){
                        sprintf(message, "%s te ha expulsado del canal %s", recv_nick, more_info);
                    }
                    else{
                        sprintf(message, "%s ha expulsado a %s del canal %s", recv_nick, target_array[prefix+2], more_info);
                    }
                    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
                }
            }
        }
    }
    
    semaphore_aw(writer, readers);
}


void command_quit_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){
	
	char *exclam;
    char message[BUFFER], recv_nick[BUFFER];

	if (n_strings-prefix >= 2){
        if(prefix){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            exclam = strchr(recv_nick, (int) '!');
            /*Getting user*/
            if(!exclam){
                exclam = strchr(recv_nick, (int) '%');
            }
            if(!exclam){
                exclam = strchr(recv_nick, (int) '@');
            }
            if(exclam){
                *exclam = '\0';
                /*Checking parameters*/
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


void command_privmsg_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char *exclam;
    char message[BUFFER], recv_nick[BUFFER];

    if (n_strings-prefix == 3){
        if(prefix){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            /*Getting user*/
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
                if(*(target_array[prefix+1]) == '#' || *(target_array[prefix+1]) == '&'){
                    interfaceText(recv_nick, message, PUBLIC_TEXT, !MAIN_THREAD);
                }
                else{
                    if(!strncasecmp(message, PCALL_CMD_STR, strlen(PCALL_CMD_STR))){
                        command_pcall_in((char **) target_array, prefix, n_strings, recv_nick);
                    }
                    else if(!strncasecmp(message, PACCEPT_CMD_STR, strlen(PACCEPT_CMD_STR))){
                        command_paccept_in((char **) target_array, prefix, n_strings, recv_nick);
                    }
                    else if(!strncasecmp(message, PCLOSE_CMD_STR, strlen(PCLOSE_CMD_STR))){
                        command_pclose_in((char **) target_array, prefix, n_strings, recv_nick);
                    }
                    else{
                        interfaceText(recv_nick, message, PRIVATE_TEXT, !MAIN_THREAD);
                    }
                }
            }
        }
    }
}


void command_invite_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char *exclam;
    char message[BUFFER], recv_nick[BUFFER];

	if (n_strings-prefix == 3){
        if(prefix && (*(target_array[n_strings-1]+sizeof(char)) == '#' || *(target_array[n_strings-1]+sizeof(char)) == '&')){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            /*Getting user*/
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

void command_ping_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

    char message[BUFFER];

	if(n_strings-prefix == 2){
        strcpy(message, target_array[prefix+1]);
        if(client_send_irc_command("PONG", message) == ERROR){
            interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
        }
    }   
}

void command_mode_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char *exclam;
    char message[BUFFER], recv_nick[BUFFER];

	/*Channel mode changed*/
    if(n_strings-prefix == 3){
        if(prefix && (*(target_array[prefix+1]) == '#' || *(target_array[prefix+1]) == '&')){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            /*Getting user*/
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
                    sprintf(message, "Has cambiado el modo del canal %s a %s.", target_array[prefix+1], target_array[prefix+2]);
                }
                else{
                    sprintf(message, "%s ha cambiado el modo del canal %s a %s.", recv_nick, target_array[prefix+1], target_array[prefix+2]);
                }
                interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
            }
        }
    }
    /*User mode in channel changed*/
    else if(n_strings-prefix == 4){
        if(prefix && (*(target_array[prefix+1]) == '#' || *(target_array[prefix+1]) == '&')){
            strcpy(recv_nick, target_array[0]+sizeof(char));
            /*Getting user*/
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
                    sprintf(message, "Has cambiado el modo en el canal %s de %s a %s.", target_array[prefix+1], target_array[prefix+3], target_array[prefix+2]);
                }
                else{
                    sprintf(message, "%s ha cambiado el modo en el canal %s de %s a %s.", recv_nick, target_array[prefix+1], target_array[prefix+3], target_array[prefix+2]);
                }
                interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
            }
        }
    }
    else{
        client_print_full_mesage((char **) &target_array, prefix, n_strings);
    }
        
}

void command_error_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	semaphore_bw(writer, readers);
    /*Disconnected*/
    in_channel = 0;
    queried = 0;
    connected = 0;
    interfaceText(NULL, "Ha sido desconectado.", MSG_TEXT, !MAIN_THREAD);
    semaphore_aw(writer, readers);

}

void rpl_who_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){
	
    char message[BUFFER];

	/*Checking parameters*/
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


void rpl_list_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char message[BUFFER];

	/*Checking parameters*/
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

void rpl_invite_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char message[BUFFER];

	/*Checking parameters*/
    if(n_strings > 1){
    	sprintf(message, "Has invitado a %s al canal %s.", target_array[n_strings-2], target_array[n_strings-1]);
    }
    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
}


void rpl_away_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char message[BUFFER];

	/*Checking parameters*/
    if(n_strings > 2){
        if(target_array[n_strings-1]+sizeof(char)){
            sprintf(message, "%s is away. (%s)", target_array[n_strings-2], target_array[n_strings-1]+sizeof(char));
        }
        else{
            sprintf(message, "%s is away. ()", target_array[n_strings-2]);
        }
        interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
    }

}

void command_query_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char message[BUFFER];

	semaphore_bw(writer, readers);
    if(n_strings-prefix == 2){
        /*In a channel*/
        if(in_channel && !queried){
            if(client_send_irc_command(PART_CMD_STR, client_channel) == ERROR){
                interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
            }
            else{
                sprintf(message, "Comienzo de mensajes con %s", target_array[n_strings-1]);
                queried = 1;
                strcpy(client_channel, target_array[n_strings-1]);
                interfaceText(nick, message, PUBLIC_TEXT, MAIN_THREAD);
            }
        }
        /*Not in a channel*/
        else{
            in_channel = 1;
            queried = 1;
            sprintf(message, "Comienzo de mensajes con %s", target_array[n_strings-1]);
            strcpy(client_channel, target_array[n_strings-1]);
            interfaceText(nick, message, PUBLIC_TEXT, MAIN_THREAD);
        }
        
    }
    else{
        interfaceText(NULL, "QUERY: uso incorrecto", ERROR_TEXT, MAIN_THREAD);
    }
    semaphore_aw(writer, readers);
}

void command_me_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	int i;
	char message[BUFFER];

    if(n_strings-prefix > 1){
    	strcpy(message, target_array[prefix+1]);

        for(i = 2; i + prefix < n_strings; ++i){
        	strcat(message, " ");
            strcat(message, target_array[prefix+i]);
        }

        interfaceText(nick, message, PUBLIC_TEXT, MAIN_THREAD);
    }
    else{
        interfaceText(NULL, "ME: uso incorrecto", ERROR_TEXT, MAIN_THREAD);
    }
                  
}

void command_part_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	if(n_strings-prefix == 2){
        if(!strcasecmp(target_array[n_strings-1], client_channel)){
            semaphore_bw(writer, readers);
            in_channel = 0;
            semaphore_aw(writer, readers);
        }
            
    }

}

void command_mode_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char message[BUFFER];

	semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if(n_strings-prefix == 2){
        sprintf(message, "%s +b %s", client_channel, target_array[n_strings-1]);
        if(client_send_irc_command(MODE_CMD_STR, message) == ERROR){
            interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
        }
    }
    else if(n_strings-prefix == 3){
        sprintf(message, "%s +b %s", target_array[n_strings-2], target_array[n_strings-1]);
        if(client_send_irc_command(MODE_CMD_STR, message) == ERROR){
            interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
        }
    }
    else{
        interfaceText(NULL, "BAN: uso incorrecto", ERROR_TEXT, MAIN_THREAD);
    }
    semaphore_ar(&readers_num, writer, mutex_rvariables);
}

void command_server_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){
	
	semaphore_bw(writer, readers);

    /*Check parameters*/
    if(n_strings-prefix == 1 && !connected){
        semaphore_aw(writer, readers);
        connectClient();
        semaphore_bw(writer, readers);
    }
    /*Two parameters*/
    else if(!connected && n_strings-prefix == 2){
        strcpy(client_server, target_array[n_strings-1]);
        port = DFLT_PORT;
        server_called = 1;
        semaphore_aw(writer, readers);
        connectClient();
        semaphore_bw(writer, readers);
    }
    /*Three parameters*/
    else if(!connected && n_strings-prefix == 3){
        strcpy(client_server, target_array[n_strings-2]);
        port = atoi(target_array[n_strings-1]);
        server_called = 1;
        semaphore_aw(writer, readers);
        connectClient();
        semaphore_bw(writer, readers);
    }
    else if(connected){
        if(client_send_irc_command(QUIT_CMD_STR, NULL) == ERROR){
            interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
        }
    }
    /*Invalid*/
    else{
        interfaceText(NULL, "SERVER: uso incorrecto.", ERROR_TEXT, MAIN_THREAD);
    }

    semaphore_aw(writer, readers);
}

void command_exit_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	if(n_strings-prefix == 1){
        client_send_irc_command(QUIT_CMD_STR, NULL);
        semaphore_rm(readers);
        semaphore_rm(writer);
        semaphore_rm(mutex_access);
        semaphore_rm(mutex_rvariables);
        close(sfd);
        exit(EXIT_SUCCESS);
    }
    else{
        interfaceText(NULL, "EXIT: uso incorrecto.", ERROR_TEXT, MAIN_THREAD);
    }

}


int command_pcall_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

	char message[BUFFER];
    long my_calling_port;

    /*Checking parameters*/
	if(n_strings-prefix != 2){
		return ERROR;
	}

    /*Avoid rejection due to timeout*/
    if(already_calling() && is_finished_call()){
        end_call();
    }

	semaphore_bw(writer, readers);

    /*Getting port*/
	my_calling_port = setup_call(0, TIMEOUT);
	if(my_calling_port == ERROR_ALREADY_CALLING || my_calling_port == ERROR_ALREADY_SETUP){
		interfaceText(NULL, "Debe terminarse la llamada actual.", ERROR_TEXT, MAIN_THREAD);
		semaphore_aw(writer, readers);
		return OK;
	}
	else if(my_calling_port <= 0){
		interfaceText(NULL, "Error durante el establecimiento de la llamada.", ERROR_TEXT, MAIN_THREAD);
		semaphore_aw(writer, readers);
		return OK;
	}

    /*Obtener IP propia*/
    if(get_own_ip(sfd, my_calling_ip) == ERROR){
        end_call();
        semaphore_aw(writer, readers);
        interfaceErrorWindow("Error al obtener la IP local.", MAIN_THREAD);
        return OK;
    }

    /*Storing nick*/
	strcpy(called_nick, target_array[prefix+1]);

    /*Message*/
	sprintf(message, "%s :%s %s %ld", called_nick, PCALL_CMD_STR, my_calling_ip, my_calling_port);
	if(client_send_irc_command(PRIVMSG_CMD_STR, message) == ERROR){
		interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
        semaphore_aw(writer, readers);
        return OK;
	}

	semaphore_aw(writer, readers);
    interfaceText(NULL, "Llamada realizada.", MSG_TEXT, MAIN_THREAD);
	return OK;

}

int command_paccept_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

    int rcall;
    long my_calling_port;
    char message[BUFFER];

    /*Checking parameters*/
    if(n_strings-prefix != 2){
        return ERROR;
    }

    /*Avoid rejection due to timeout*/
    if(already_calling() && is_finished_call()){
        end_call();
    }

    semaphore_bw(writer, readers);

    /*Checking user*/
    if(strcasecmp(incoming_nick, target_array[prefix+1])){
        sprintf(message, "La ultima llamada recibida no es del usuario %s.", target_array[prefix+1]);
        interfaceText(NULL, message, ERROR_TEXT, MAIN_THREAD);
        semaphore_aw(writer, readers);
        return OK;
    }

    /*Getting port*/
    my_calling_port = setup_call(0, TIMEOUT);
    if(my_calling_port == ERROR_ALREADY_CALLING || my_calling_port == ERROR_ALREADY_SETUP){
        interfaceText(NULL, "Debe terminarse la llamada actual.", ERROR_TEXT, MAIN_THREAD);
        semaphore_aw(writer, readers);
        return OK;
    }
    else if(my_calling_port <= 0){
        interfaceText(NULL, "Error durante el establecimiento de la llamada.", ERROR_TEXT, MAIN_THREAD);
        semaphore_aw(writer, readers);
        return OK;
    }

    /*Obtener IP propia*/
    if(get_own_ip(sfd, my_calling_ip) == ERROR){
        end_call();
        semaphore_aw(writer, readers);
        interfaceText(NULL, "Error al obtener la IP local.", ERROR_TEXT, MAIN_THREAD);
        return OK;
    }

    /*Storing nick*/
    strcpy(called_nick, target_array[prefix+1]);

    /*En el sprintf realmente va la IP*/
    sprintf(message, "%s :%s %s %ld", called_nick, PACCEPT_CMD_STR, my_calling_ip, my_calling_port);
    if(client_send_irc_command(PRIVMSG_CMD_STR, message) == ERROR){
        interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
        end_call();
        semaphore_aw(writer, readers);
        return OK;
    }

    rcall = call(their_calling_ip, their_calling_port);

    semaphore_aw(writer, readers);

    if(rcall == ERROR_PARAM){
        interfaceText(NULL, "Destino inválido.", ERROR_TEXT, MAIN_THREAD);
        end_call();
        return OK;
    }

    else if (rcall < 0){
        interfaceText(NULL, "Error al establecer la conexión.", ERROR_TEXT, MAIN_THREAD);
        end_call();
        return OK;
    }

    else{
        interfaceText(NULL, "Conexión establecida.", MSG_TEXT, MAIN_THREAD);
    }

    return OK;

}

int command_pclose_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings){

    char message[BUFFER];

    /*Checking parameters*/
    if(n_strings-prefix != 2){
        return ERROR;
    }

    if(!already_calling() || strcasecmp(called_nick, target_array[prefix+1])){
        end_call();
        return OK;
    }

    end_call();

    interfaceText(NULL, "Llamada cancelada.", MSG_TEXT, MAIN_THREAD);
    sprintf(message, "%s :%s", target_array[prefix+1], PCLOSE_CMD_STR);

    if(client_send_irc_command(PRIVMSG_CMD_STR, message) == ERROR){
        interfaceErrorWindow("Error al enviar el mensaje. Inténtelo de nuevo.", MAIN_THREAD);
    }

	return OK;

}

void command_pcall_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick){

    char message[BUFFER];
    char *ip, *port;

    /*Receive message*/
    strcpy(message, target_array[prefix+2]+sizeof(char));

    ip = strstr(message, " ")+sizeof(char);
    if(!ip){
        interfaceText(recv_nick, message, PRIVATE_TEXT, !MAIN_THREAD);
        return;
    }

    port = strstr(ip, " ")+sizeof(char);
    if(!port){
        interfaceText(recv_nick, message, PRIVATE_TEXT, !MAIN_THREAD);
        return;
    }

    semaphore_bw(writer, readers);
    strcpy(incoming_nick, recv_nick);

    strcpy(their_calling_ip, ip);
    ip = strstr(their_calling_ip, " ");
    *ip = '\0';

    their_calling_port = atoi(port);
    semaphore_aw(writer, readers);

    sprintf(message, "Recibida llamada de %s.", recv_nick);
    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
    sprintf(message, "Para contestar: /PACCEPT %s", recv_nick);
    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);


}

void command_paccept_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick){

    int rcall;
    char message[BUFFER];
    char *ip, *port;

    /*Receive message*/
    strcpy(message, target_array[prefix+2]+sizeof(char));

    ip = strstr(message, " ")+sizeof(char);
    if(!ip){
        interfaceText(recv_nick, message, PRIVATE_TEXT, !MAIN_THREAD);
        return;
    }

    port = strstr(ip, " ")+sizeof(char);
    if(!port){
        interfaceText(recv_nick, message, PRIVATE_TEXT, !MAIN_THREAD);
        return;
    }


    semaphore_bw(writer, readers);

    if(strcmp(called_nick, recv_nick)){
        semaphore_aw(writer, readers);
        return;
    }

    strcpy(their_calling_ip, ip);
    ip = strstr(their_calling_ip, " ");
    *ip = '\0';

    their_calling_port = atoi(port);
    
    rcall = call(their_calling_ip, their_calling_port);

    semaphore_aw(writer, readers);

    if(rcall == ERROR_PARAM){
        interfaceText(NULL, "Destino inválido.", ERROR_TEXT, !MAIN_THREAD);
        end_call();
    }

    else if (rcall < 0){
        interfaceText(NULL, "Error al establecer la conexión.", ERROR_TEXT, !MAIN_THREAD);
        interfaceText(NULL, their_calling_ip, ERROR_TEXT, MAIN_THREAD);
        end_call();
    }

    else{
        sprintf(message, "Llamada aceptada por %s. Conexión establecida.", recv_nick);
        interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);
    }


}

void command_pclose_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick){

    char message[BUFFER];

    /*Receive message*/
    strcpy(message, target_array[prefix+2]+sizeof(char));

    if(strlen(message) != strlen(PCLOSE_CMD_STR)){
        interfaceText(recv_nick, message, PRIVATE_TEXT, !MAIN_THREAD);
        return;
    }

    sprintf(message, "Llamada finalizada por %s. Conexión cerrada.", recv_nick);
    interfaceText(NULL, message, MSG_TEXT, !MAIN_THREAD);

    end_call();

}