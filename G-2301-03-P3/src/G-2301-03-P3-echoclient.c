#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <arpa/inet.h>
#include "G-2301-03-P1-connection.h"
#include "G-2301-03-P3-SSL_funcs.h"

#define MAX_BUF 1000

int OpenConnection(const char *hostname, int port)
{   int sd;
    struct sockaddr_in addr;
    socklen_t addrlen=sizeof(addr);
    
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, addrlen);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(hostname, &addr.sin_addr);
    if ( connect(sd, (struct sockaddr*)&addr, addrlen) != 0 )
    {
        close(sd);
        perror(hostname);
        return -1;
    }
    return sd;
}


int main (int argc, char *argv[]) {
    char servIP[] = "127.0.0.1";
    int port=16000;
    int fd=0;
    char buf[MAX_BUF];
    void* data_received;
    SSL* ssl=NULL;
    SSL_CTX* ctx=NULL;
    
    
    if (!(fd=OpenConnection(servIP, port))) {
        printf("Error creando socket\n");
        return -1;
    }
    
    if (argc<3) {
        printf("Especifique como primer argumento, la ruta del certificado propio a usar \n"\
               "Especifique como segundo argumento, el certificado autofirmado de la CA confiable\n");
        return -1;
    }
    
    inicializar_nivel_SSL();
    if (!(ctx=fijar_contexto_SSL(argv[1], argv[2], &SSLv23_method, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT))) {
        printf("ERROR al fijar contexto\n");
        return -1;
    }
    
    if (!(ssl=conectar_canal_seguro_SSL(ctx, fd))) {
        printf("ERROR al conectar canal seguro\n");
        return -1;
    }
    if (evaluar_post_conectar_SSL(ssl)!=OK) {
        printf("El certificado del peer no es valido\n");
        cerrar_canal_SSL(ssl);
        return -1;
    }
    while (fgets( buf, 1000, stdin) != NULL) {
        data_received=NULL;
        buf[strlen(buf)-1]=0;
        enviar_datos_SSL(ssl, buf, strlen(buf), 1000); /* send msg */
        if (recibir_datos_SSL(ssl, &data_received, 1000, "\0", 1)<0) { /*recibiendo respuesta*/
            break;
        }
        printf("Recibido: %s\n", (char *)data_received);
        if ((strcmp(data_received, "squit")==0) || (strcmp(data_received, "cquit")==0)) {free(data_received); break;}
        free(data_received);
    }
    cerrar_canal_SSL(ssl);
    ERR_free_strings();
    return 0;    
}
