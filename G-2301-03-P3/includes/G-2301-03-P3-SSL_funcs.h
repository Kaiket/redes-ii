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

void inicializar_nivel_SSL();
SSL_CTX* fijar_contexto_SSL(char* mycert, char* CAcert, const SSL_METHOD* (*method)(void), int verify_mode);
SSL* conectar_canal_seguro_SSL (SSL_CTX* ctx, int socket);
SSL* aceptar_canal_seguro_SSL (SSL_CTX* ctx, int socket);
int evaluar_post_conectar_SSL(SSL* ssl);
int enviar_datos_SSL(SSL* ssl, void *data, size_t length, size_t segmentsize);
int recibir_datos_SSL(SSL* ssl, void** data, size_t segmentsize, void* enddata, size_t enddata_len);
void cerrar_canal_SSL(SSL* ssl);

#endif