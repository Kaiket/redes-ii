#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include "G-1301-03-P2-chat.h"
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P2-chat_funcs.h"
#include "G-1301-03-P2-client_utility_functions.h"

void connectClient(void){
	
	int port;
	char *nick, *name, *real_name, *server_url;
	char message[BUFFER];
	
	/* Getting text of fields */
	nick = getApodo();
	name =  getNombre();
	real_name = getNombreReal();
    server_url =  getServidor();
    port = getPuerto();

    /* Empty nick, name, real_name or server_url field */
    if(!(strlen(nick) && strlen(name) && strlen(real_name) && strlen(server_url))) {
    	interfaceErrorWindow("Los campos\n\t-Apodo\n\t-Nombre\n\t-Nombre real\n\t-Servidor\nno pueden estar vacíos", MAIN_THREAD);
    	return;
    }

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
    if (client_new_session(nick, name, real_name, server_url) == ERROR){
    	sprintf(message, "Fallo en la conexión con %s\nEn el sistema de logs ha quedado registrado el error" 
    					 "correspondiente a este intento de conexión.", server_url);
    	interfaceErrorWindow(message, MAIN_THREAD);
    	syslog(LOG_ERR, "Failed while connecting to %s: %s", server_url, strerror(errno));
    	interfaceText(NULL, "Error de conexión", ERROR_TEXT, MAIN_THREAD);
    	close(sfd);
    	return;
    }
}

void disconnectClient(void)
{
	errorWindow("Función no implementada");
	errorText("Función no implementada");
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
	errorText("Función no implementada");
}





