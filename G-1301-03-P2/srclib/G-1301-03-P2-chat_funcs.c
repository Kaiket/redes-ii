#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include "G-1301-03-P2-chat.h"
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-semaphores.h"
#include "G-1301-03-P2-chat_funcs.h"
#include "G-1301-03-P2-client_utility_functions.h"

extern int sfd;
extern int connected;
extern char nick[BUFFER];
extern char client_channel[BUFFER];
extern int readers_num;
extern int readers;
extern int writer;
extern int mutex_access;
extern int mutex_rvariables;

void connectClient(void){
	
	int port;
	char *name, *real_name, *server_url;
	char message[BUFFER];

    semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if(connected){
        interfaceText(NULL, "Ya está conectado a un servidor.", ERROR_TEXT, MAIN_THREAD);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
        return;
    }
    semaphore_ar(&readers_num, writer, mutex_rvariables);
	
	/* Getting text of fields */
	semaphore_bw(writer, readers);
    strcpy(nick, getApodo());
    semaphore_aw(writer, readers);
	name =  getNombre();
	real_name = getNombreReal();
    server_url =  getServidor();
    port = getPuerto();

    /* Empty nick, name, real_name or server_url field */
    semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if(!(strlen(nick) && strlen(name) && strlen(real_name) && strlen(server_url))) {
    	interfaceErrorWindow("Los campos\n\t-Apodo\n\t-Nombre\n\t-Nombre real\n\t-Servidor\nno pueden estar vacíos", MAIN_THREAD);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
    	return;
    }
    semaphore_ar(&readers_num, writer, mutex_rvariables);

    /* Empty port field */
    if(port <= 0){
    	port = DFLT_PORT;
    }

    /*Beginning of connection*/
    sprintf(message, "Conectando con %s:%d ...", server_url, port);
    interfaceText(NULL, message, MSG_TEXT, MAIN_THREAD);

    /*Connecting to the server*/
    if (client_connect_to_server(server_url, port, client_thread_listener) == ERROR){
    	sprintf(message, "Fallo en la conexión con %s\nEn el sistema de logs ha quedado registrado el error" 
    					 "correspondiente a este intento de conexión.", server_url);
    	interfaceErrorWindow(message, MAIN_THREAD);
    	syslog(LOG_ERR, "Failed while connecting to %s: %s", server_url, strerror(errno));
    	interfaceText(NULL, "Error de conexión", ERROR_TEXT, MAIN_THREAD);
    	return;
    }

    /*New session*/
    semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if (client_new_session(nick, name, real_name, server_url) == ERROR){
    	sprintf(message, "Fallo en la conexión con %s\nEn el sistema de logs ha quedado registrado el error" 
    					 "correspondiente a este intento de conexión.", server_url);
    	interfaceErrorWindow(message, MAIN_THREAD);
    	syslog(LOG_ERR, "Failed while connecting to %s: %s", server_url, strerror(errno));
    	interfaceText(NULL, "Error de conexión", ERROR_TEXT, MAIN_THREAD);
    	close(sfd);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
    	return;
    }
    semaphore_ar(&readers_num, writer, mutex_rvariables);

    semaphore_bw(writer, readers);
    connected = 1;
    semaphore_aw(writer, readers);

}

void disconnectClient(void)
{

    semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if(!connected){
        interfaceText(NULL, "No está conectado a ningún servidor.", ERROR_TEXT, MAIN_THREAD);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
        return;
    }
    semaphore_ar(&readers_num, writer, mutex_rvariables);

    if(client_send_irc_command("QUIT", ":Leaving") == ERROR){
        interfaceErrorWindow("Error al desconectar, inténtelo de nuevo.En el sistema de logs ha quedado registrado el error" 
                             "correspondiente a este intento de conexión.", MAIN_THREAD);
        return;
    }

}

void topicProtect(gboolean state)
{
	errorWindow("Función no implementada");
	errorText("Función no implementada");
}

void externMsg(gboolean state)
{
	errorWindow("Función no implementada");
	errorText("Función no implementada");
}

void secret(gboolean state)
{
	errorWindow("Función no implementada");
	errorText("Función no implementada");
}

void guests(gboolean state)
{
	errorWindow("Función no implementada");
	errorText("Función no implementada");
}

void privated(gboolean state)
{
	errorWindow("Función no implementada");
	errorText("Función no implementada");
}

void moderated(gboolean state)
{
	errorWindow("Función no implementada");
	errorText("Función no implementada");
}

void newText (const char *msg)
{
    char command[BUFFER];

    if(!msg){
        return;
    }

    if(!strcmp(msg, "")){
        return;
    }

    if(!strncmp(msg, "/", 1)){
        strcpy(command, msg + sizeof(char));
        client_cmd_parsing(command, IRC_CMD);
    }
    else{
        semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
        if(!connected){
            interfaceText(NULL, "No está conectado a ningún servidor.", ERROR_TEXT, MAIN_THREAD);
            semaphore_ar(&readers_num, writer, mutex_rvariables);
            return;
        }
        semaphore_ar(&readers_num, writer, mutex_rvariables);
        strcpy(command, msg);
        client_cmd_parsing(command, IRC_MSG);
    }

    

    

}





