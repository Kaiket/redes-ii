/***************************************************************************************************
* Redes de Comunicaciones II                                                                       *
* 25/2/2014                                                                                        *
*                                                                                                  *
*     Include necesario para el uso de las funciones públicas definidas para la práctica           *
*                                                                                                  *
***************************************************************************************************/

#ifndef _CHAT
#define _CHAT

#ifndef FALSE
	#define FALSE	0
#endif
#ifndef TRUE
	#define TRUE	1
#endif

#define MSG_TEXT 0
#define ERROR_TEXT 1
#define PRIVATE_TEXT 2
#define PUBLIC_TEXT 3
#define MAIN_THREAD 1

#include <glib.h>

/* Función de ventana de error */
/**
 * @brief Show an error window with a message.
 * @details The window shown contains a error message written by the user.
 * 
 * @param msg message to show.
 * @param mainthread If the calling process is the main process (gtk main process) it must be
 * diferent than cero. Else, it must be zero.
 */
void interfaceErrorWindow(char *msg, int mainthread);
void errorWindow(char *msg);

/* Interfaz de impresión de textos */
/**
 * @brief Prints a message on the interface.
 * @details The message may be plain text, error text, private text from an user or
 * public text. If the calling thread is the main thread, the last argument must not 
 * be zero. If the calling process is not the main thread, the last argument must be
 * zero.
 * 
 * @param username name of the user for public and private text types. It may be null.
 * @param message message to print.
 * @param type It the macro *_TEXT. Depending on the type of message the asterisk can be MSG,
 * ERROR, PRIVATE or PUBLIC.
 * @param mainthread If the calling process is the main process (gtk main process) it must be
 * diferent than cero. Else, it must be zero.
 */
void interfaceText(char *username, char *message, int type, int mainthread);
void publicText(char *username, char *text);
void privateText(char *username, char *text);
void errorText(char *errormessage);
void messageText(char *message);

/* Funciones de conexión y desconexión */
void connectClient(void);
void disconnectClient(void);

/* Funciones llamadas cuando se produce un cambio de estado */
void topicProtect(gboolean state);
void externMsg(gboolean state);
void secret(gboolean state);
void guests(gboolean state);
void privated(gboolean state);
void moderated(gboolean state);

/* Funciones para obtener los campos */
char *getApodo();
char *getNombre();
char *getNombreReal();
char *getServidor();
int getPuerto();

/* Funciones para cambiar el estado */
void setTopicProtect(gboolean state);
void setExternMsg(gboolean state);
void setSecret(gboolean state);
void setGuests(gboolean state);
void setPrivated(gboolean state);
void setModerated(gboolean state);

/* Función llamada cuando se introduce una entrada */
void newText (const char *msg);

#endif
