/*
Funciones del ejercicio 3 y 4
*/

#include <stdio.h>
#include <G-1301-03-P1-connection.h>
#include <G-1301-03-P1-types.h>

/*FUNCIONES DE ALTO NIVEL*/

/*Función que iniciará servidor, es decir, abrirá el socket, realizará las asignación del socket al puerto desea-
do y abrirá una cola de escucha de una longitud determinada. Esta función devolverá un código de error y
tendrá dos parámetros:
1.1.– El número de puerto en el que estará esperando recibir las comunicaciones.
1.2.– La longitud máxima de la cola de espera de conexiones.
*/
int init_server (int port, int max_connections) {
	return OK;
}

/*Función que pondrá al servidor a “escuchar” peticiones de conexión. Devolverá un código de error si la
conexión no se ha realizado.
*/

int listen_connection () {
	return OK;
}

/*Función que cierra la comunicación. Tendrá como parámetro el handler de la conexión a cerrar y devolverá
un código de error.
*/

int close_connection (int handler) {
	return OK;
}

/*
La función que envíe los datos tendrá como parámetros un puntero a los datos (que será de tipo void *)
y la longitud en bytes a enviar. Devolverá la longitud en bytes enviada o un código de error (negativo). Enviará
los datos en paquetes del tamaño máximo de un segmento salvo el último que sólo enviará los datos restantes.
*/
int send_msg (int socket, void *data, int length) {
	return OK;
}

/*
La función que recibe los datos tendrá como parámetro un puntero a un puntero a los datos (que será
de tipo void **). Devolverá la longitud en bytes recibida o un código de error (negativo). Esta función hará sitio
en la memoria para un buffer del tamaño adecuado a esa longitud (cuidado con la terminación de cadena
de caracteres) y rellenará el buffer con los datos. 
*/
int receive_msg (int socket, void **data) {
}


/*FUNCIONES DE BAJO NIVEL*/

/*Función que abre un socket de servidor TCP. No tiene parámetros. Devolverá un código de error o el handler
del socket según corresponda.*/
int open_TCP_socket () {
	return OK;
}

/* Función que asigna un puerto al socket. Tendrá dos parámetros que serán el handler del socket y el número
de puerto. Devolverá un código de error.
*/
int set_port (int socket, int port) {
	return OK;
}


/*Función que determina la longitud de la cola. Tendrá dos parámetros que serán el handler del socket y la
longitud de la cola. Devolverá un código de error.
*/
int set_queue_length (int socket, int length) {
	return OK;
}

