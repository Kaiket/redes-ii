#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include "G-2301-03-P1-connection.h"
#include "G-2301-03-P3-SSL_funcs.h"

#define MAX_BUF 1000

int main (int argc, char *argv[]) {
    int fd=0;
    int port=16000, leido=0, salir=0;
    int client_socket;
    void *data;
    SSL* ssl=NULL;
    SSL_CTX* ctx=NULL;
    
    if (argc<3) {
        printf("Especifique como primer argumento, la ruta del certificado propio a usar \n"\
               "Especifique como segundo argumento, el certificado autofirmado de la CA confiable\n");
        return -1;
    }
    
    if (!(fd=init_server(port, 100))) {
        printf("ERROR al inicializar servidor\n");
        return -1;
    }
    inicializar_nivel_SSL();
    while (salir==0) {
        client_socket = accept_connections(fd);
        ctx=NULL;
        if (!(ctx=fijar_contexto_SSL(argv[1], argv[2], &SSLv23_method))) {
            printf("ERROR al fijar contexto\n");
            continue;
        }
        if (!(ssl=aceptar_canal_seguro_SSL(ctx, client_socket))) {
            close(client_socket);
            SSL_CTX_free(ctx);
            printf("ERROR al aceptar canal seguro\n");
            continue;
        }
        if (evaluar_post_conectar_SSL(ssl)!=OK) {
            printf("El certificado del peer no es valido\n");
            cerrar_canal_SSL(ssl);
            continue;
        }
        while ((leido=recibir_datos_SSL(ssl, &data, 900, "\0", 1)) > 0) {/* get request */
            printf("Client msg: \"%s\"\n", (char*)data);
            enviar_datos_SSL(ssl, data, strlen(data), 1000); /* send reply */
            if (strncmp(data, "cquit", strlen("cquit"))==0) {
                free(data); 
                break;
            }
            if (strncmp(data, "squit", strlen("squit"))==0) {
                free(data); 
                salir=1; 
                break;
            }     
            free(data);
        }
        cerrar_canal_SSL(ssl);
    }
    close(fd);
    ERR_free_strings();
    return 0;
}
