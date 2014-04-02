#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include "G-1301-03-P2-chat.h"
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P2-chat_funcs.h"
#include "G-1301-03-P1-connection.h"

void connectClient(void){
	
	int sfd, port;
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
    	errorWindow("Los campos\n\t-Apodo\n\t-Nombre\n\t-Nombre real\n\t-Servidor\nno pueden estar vacíos");
    	return;
    }

    /* Empty port field */
    if(port <= 0){
    	port = DFLT_PORT;
    }

    /*Connecting to the server*/
    sprintf(message, "Conectando con %s:%d ...", server_url, port);
    messageText(message);
    if((sfd = connect_to_server(server_url, port)) == ERROR){
    	sprintf(message, "Fallo en la conexión con %s\nEn el sistema de logs ha quedado registrado el error" 
    					 "correspondiente a este intento de conexión.", server_url);
    	errorWindow(message);
    	syslog(LOG_ERR, "Failed while connecting to %s: %s", server_url, errno);
    	return;
    }

    messageText("Conectado");
    return; /*Connected*/


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





