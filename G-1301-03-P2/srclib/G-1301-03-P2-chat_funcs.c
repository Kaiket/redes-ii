#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "G-1301-03-P2-chat_funcs.h"
#include "G-1301-03-P2-chat.h"

void connectClient(void)
{
	
	int port;
	char *nick, *name, *real_name, *server_url;
	
	/* Getting text of fields */
	nick = getApodo();
	name =  getNombre();
	real_name = getNombreReal();
    server_url =  getServidor();
    port = getPuerto();

    /* Empty nick, name, real_name or server_url field */
    if(!(strlen(nick) && strlen(name) && strlen(real_name) && strlen(server_url))) {
    	errorWindow("Los campos\n\t-Apodo\n\t-Nombre\n\t-Nombre real\n\t-Servidor\nno pueden estar vacíos");
    }

    /* Empty port field */
    if(port <= 0){
    	port = DFLT_PORT;
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


