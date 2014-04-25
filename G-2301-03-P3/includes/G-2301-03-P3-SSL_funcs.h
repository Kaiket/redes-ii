#ifndef __SSL_FUNCS_H
#define __SSL_FUNCS_H

#ifndef OK
        #define OK 0
#endif
#ifndef ERROR
        #define ERROR -1
#endif
#define ERROR_CTX -2
#define ERROR_LOADVERIFY -3 /*Error loading a verify location*/
#define ERROR_SETPATHS -4 /*Error setting default verify paths*/
#define ERROR_CHAINFILE -5 /*Error un use chain file*/
#define ERROR_PRKEYFILE -6 /*Error setting key to use*/
#define ERROR_SSLNEW -7
#define ERROR_HANDSHAKE -8
#define ERROR_SETFD -9
#define ERROR_NOPEERCERT -10
#define ERROR_VERIFY -11 /*peer couldn't be verified*/

/** 
 * @brief Realiza un setup del nivel SSL
 * 
 * @details Es necesario llamar a esta funcion en toda aplicacion que quiera
 * utilizar SSL. Solo hace falta llamarla una vez, no requiere parametros ni tiene valor de retorno.
 *
 */
void inicializar_nivel_SSL();

/** 
 * @brief Inicializa un contexto SLL
 * 
 * @details toma los certificados pasados como argumentos para configurar dicho contexto
 * El método debe ser un puntero a una funcion válida para inicializar un metodo SSL (ver ssl(3))
 * Verify_mode indica las banderas de inicializacion del contexto
 * 
 * @param mycert ruta al certificado propio a usar
 * @param CAcert ruta al certificado de la CA utilizado para validar mycert (puede ser null)
 * @param method metodo de inicializacion del contexto
 * @param verify_mode banderas de inicializacion del contexto
 * 
 * @return puntero al contexto o NULL si hubo error.
 *
 * @see ssl(3)
 */
SSL_CTX* fijar_contexto_SSL(char* mycert, char* CAcert, const SSL_METHOD* (*method)(void), int verify_mode);

/** 
 * @brief Establece una sesion SSL en el socket pasado con el contexto pasado
 * 
 * @details Vincula la conexion activa en socket con el contexto para mantener una sesion SSL
 * Esta funcion se llama desde el lado del CLIENTE TCP
 * 
 * @param ctx contexto usado para la sesion SSL
 * @param socket socket con la conexion activa
 * 
 * @return puntero al SSL* si todo OK o NULL si hubo error (establece errno al valor apropiado)
 *
 */
SSL* conectar_canal_seguro_SSL (SSL_CTX* ctx, int socket);

/** 
 * @brief Establece una sesion SSL en el socket pasado con el contexto pasado
 * 
 * @details Vincula la conexion activa en socket con el contexto para mantener una sesion SSL
 * Esta funcion se llama desde el lado del SERVIDOR TCP
 * 
 * @param ctx contexto usado para la sesion SSL
 * @param socket socket con la conexion activa
 * 
 * @return puntero al SSL* si todo OK o NULL si hubo error (establece errno al valor apropiado)
 *
 */
SSL* aceptar_canal_seguro_SSL (SSL_CTX* ctx, int socket);

/** 
 * @brief Evalua los certificados del otro lado de la sesion SSL
 * 
 * @details Comprueba si el remitente ha enviado certificados y si estos pasan la validacion
 * 
 * @param ssl puntero a la estructura de sesion ssl
 * 
 * @return OK si pasó la validacion o varios tipos de error si no (ver defines)
 *
 */
int evaluar_post_conectar_SSL(SSL* ssl);


/** 
 * @brief ENvia datos en una sesion SSL
 * 
 * @details Envia la cantidad de datos especificada en length en mensajes de tamaño segmentsize(si procede)
 * de el buffer data a una conexion ssl activa
 * 
 * @param ssl sesion ssl
 * @param data puntero al buffer de datos
 * @param length tamaño de datos a enviar
 * @param segmentsize maximo tamaño de segmento a enviar, si length es mayor que este parámetro
 *              se realizaran los envios en varios paquetes
 * 
 * @return numero de bytes realmente enviados
 *
 */
int enviar_datos_SSL(SSL* ssl, void *data, size_t length, size_t segmentsize);

/** 
 * @brief Recibe datos en una sesion SSL
 * 
 * @details Recibe en *data los datos leidos del canal ssl.
 * Segmentsize debe indicar el maximo tamaño de paquetes que se recibe,
 * cuando una recepcion es menor en tamaño que segmentsize se para de leer
 * 
 * @param ssl sesion ssl
 * @param data puntero al buffer de datos
 * @param segmentsize maximo tamaño de segmento a recibir
 * @param enddata cadena indicando el final de mensaje. Si se ha leido un tamaño igual a segmentsize
 * se comprueba si los ultimos bytes leiidos se corresponden con esta cadena (para no hacer mas lecturas y dar por terminado el mensaje)
 * @param enddata_len tamaño de la cadena pasada.
 *  
 * @return numero de bytes recibidos.
 *
 */
int recibir_datos_SSL(SSL* ssl, void** data, size_t segmentsize, void* enddata, size_t enddata_len);

/**
 * @brief Cierra un canal ssl de forma segura
 * 
 * @details Cierra de forma ordenada el canal SSL liberando todos los recursos (contexto y socket)
 * 
 * @param ssl el canal ssl a cerrar
 */
void cerrar_canal_SSL(SSL* ssl);

#endif