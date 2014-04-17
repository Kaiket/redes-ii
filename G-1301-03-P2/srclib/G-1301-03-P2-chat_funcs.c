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

/*Global extern variables*/
extern int sfd;
extern int connected;
extern int in_channel;
extern int server_called;
extern int queried;
extern int port;
extern char nick[BUFFER];
extern char client_channel[BUFFER];
extern char client_server[BUFFER];
extern int readers_num;
extern int readers;
extern int writer;
extern int mutex_access;
extern int mutex_rvariables;

/**
 * @brief Sends a request to connect a client to a server using gtk fields.
 * @details 
 */
void connectClient(void){
	
	char *name, *real_name;
	char message[BUFFER];

    /*Checks if it is already connected*/
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
    semaphore_bw(writer, readers);
    if(server_called){
        server_called = 0;
    }
    else{
        strcpy(client_server, getServidor());
        port = getPuerto();
    }
    semaphore_aw(writer, readers);
    

    /* Empty nick, name, real_name or client_server field */
    semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if(!(strlen(nick) && strlen(name) && strlen(real_name) && strlen(client_server))) {
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
    sprintf(message, "Conectando con %s:%d ...", client_server, port);
    interfaceText(NULL, message, MSG_TEXT, MAIN_THREAD);

    /*Connecting to the server*/
    if (client_connect_to_server(client_server, port, client_thread_listener) == ERROR){
    	sprintf(message, "Fallo en la conexión con %s\nEn el sistema de logs ha quedado registrado el error" 
    					 "correspondiente a este intento de conexión.", client_server);
    	interfaceErrorWindow(message, MAIN_THREAD);
    	syslog(LOG_ERR, "Failed while connecting to %s: %s", client_server, strerror(errno));
    	interfaceText(NULL, "Error de conexión", ERROR_TEXT, MAIN_THREAD);
    	return;
    }

    /*New session*/
    semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if (client_new_session(nick, name, real_name, client_server) == ERROR){
    	sprintf(message, "Fallo en la conexión con %s\nEn el sistema de logs ha quedado registrado el error" 
    					 "correspondiente a este intento de conexión.", client_server);
    	interfaceErrorWindow(message, MAIN_THREAD);
    	syslog(LOG_ERR, "Failed while connecting to %s: %s", client_server, strerror(errno));
    	interfaceText(NULL, "Error de conexión", ERROR_TEXT, MAIN_THREAD);
    	close(sfd);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
    	return;
    }
    semaphore_ar(&readers_num, writer, mutex_rvariables);

    /*Connection successful*/
    semaphore_bw(writer, readers);
    connected = 1;
    semaphore_aw(writer, readers);

}

/**
 * @brief Send a request to disconnect from the server.
 * @details 
 */
void disconnectClient(void)
{

    /*Check if it is not connected yet.*/
    semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
    if(!connected){
        interfaceText(NULL, "No está conectado a ningún servidor.", ERROR_TEXT, MAIN_THREAD);
        semaphore_ar(&readers_num, writer, mutex_rvariables);
        return;
    }
    semaphore_ar(&readers_num, writer, mutex_rvariables);

    /*Sends the request of disconnection*/
    if(client_send_irc_command(QUIT_CMD_STR, ":Leaving") == ERROR){
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

/**
 * @brief Manage the text got as input from the user.
 * @details The message may be a command or a plain message.
 * If it is a command, it may be an user command or a standard command.
 * In case that it is an user command, the proper action is taken. If 
 * it is a server command, it is sent to the server.
 * If it is a plain message it is sent after check some conditions: the
 * user must be connected to a server and must be in a channel.
 * 
 * @param msg user input.
 */
void newText (const char *msg)
{
    char command[BUFFER];

    /*Empty message*/
    if(!msg){
        interfaceText(NULL, "Mensaje vacío.", ERROR_TEXT, MAIN_THREAD);
        return;
    }

    if(!strcmp(msg, "")){
        interfaceText(NULL, "Mensaje vacío.", ERROR_TEXT, MAIN_THREAD);
        return;
    }

    /*It is a command*/
    if(!strncmp(msg, "/", 1)){
        if(strlen(msg) == 1){
            interfaceText(NULL, "Comando vacío.", ERROR_TEXT, MAIN_THREAD);
        }
        else{
            strcpy(command, msg + sizeof(char));
            client_cmd_parsing(command, IRC_CMD);
        }
    }
    /*It is a message*/
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





