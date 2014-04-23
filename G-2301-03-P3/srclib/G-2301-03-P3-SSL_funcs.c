#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <inttypes.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <resolv.h>
#include <syslog.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include "G-2301-03-P3-SSL_funcs.h"
/*inicializar_nivel_SSL : Esta función se encargará de realizar todas las llamadas necesarias para que la apli-
cación pueda usar la capa segura SSL.
 * */
void inicializar_nivel_SSL() {
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    SSL_library_init();
    return;
}


SSL_CTX* fijar_contexto_SSL(char* mycert, char* CAcert, const SSL_METHOD* (*method)(void)) {
    SSL_CTX *ctx=NULL;
    /*creating ctx*/
    ctx = SSL_CTX_new(method());
    if (!(ctx)) {
        ERR_print_errors_fp(stderr);
        errno=ERROR_CTX;
        return NULL;
    }
    /*loading verify location if CAcert is not null*/
    if (CAcert) {
        if (SSL_CTX_load_verify_locations(ctx, CAcert, NULL) != 1) {
                ERR_print_errors_fp(stderr);
                errno=ERROR_LOADVERIFY;
                return NULL;
        }
    }
    /*setting the default verify paths*/
    if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
        ERR_print_errors_fp(stderr);
        errno=ERROR_SETPATHS;
        return NULL;
    }
    /*specify which certificate the application is going to use*/
    if (SSL_CTX_use_certificate_chain_file(ctx, mycert) != 1) {
        ERR_print_errors_fp(stderr);
        errno=ERROR_SETPATHS;
        return NULL;
    }
    /**/
    if (SSL_CTX_use_PrivateKey_file(ctx, mycert, SSL_FILETYPE_PEM) != 1) {
        ERR_print_errors_fp(stderr);
        errno=ERROR_PRKEYFILE;
        return NULL;
    }
    SSL_CTX_set_verify(ctx , SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

    return ctx;
}


SSL* conectar_canal_seguro_SSL (SSL_CTX* ctx, int socket) {
    SSL* ssl;
    if (!(ssl = SSL_new(ctx))) {      /*new SSL connection*/
        ERR_print_errors_fp(stderr);
        errno=ERROR_SSLNEW;
        return NULL;
    }
    if (SSL_set_fd(ssl, socket) != 1) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        errno=ERROR_SETFD;
        return NULL;
    } 
    if ( SSL_connect(ssl) != 1 ) {  /* perform the handshake */
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        errno=ERROR_HANDSHAKE;
        return NULL;
    }
    return ssl;
}

SSL* aceptar_canal_seguro_SSL (SSL_CTX* ctx, int socket) {
    SSL* ssl;
    if (!(ssl = SSL_new(ctx))) {      /*new SSL connection*/
        ERR_print_errors_fp(stderr);
        errno=ERROR_SSLNEW;
        return NULL;
    }
    if (SSL_set_fd(ssl, socket) != 1) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        errno=ERROR_SETFD;
        return NULL;
    } 
    if ( SSL_accept(ssl) != 1 ) {  /* perform the handshake */
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        errno=ERROR_HANDSHAKE;
        return NULL;
    }
    return ssl;
}

int evaluar_post_conectar_SSL(SSL* ssl) {
    X509* peer_cert=NULL;
    peer_cert=SSL_get_peer_certificate(ssl);
    if (!peer_cert) {
        return ERROR_NOPEERCERT;
    }
    if (SSL_get_verify_result(ssl)!=X509_V_OK) return ERROR;
    X509_free(peer_cert);
    return OK;            
}

int enviar_datos_SSL(SSL* ssl, void *data, size_t length, size_t segmentsize) {
    int sended = 0;

    if (segmentsize <= 0) {
        segmentsize = TCP_MSS_DEFAULT;
    }

    while (length) {

        if (length <= segmentsize) {
            if (SSL_write(ssl, data + sended, length) != length) {
                syslog(LOG_ERR, "Failed while sending msg. %s", strerror(errno));
                return ERROR;
            }
            sended += length;
            return sended;
        }

        if (SSL_write(ssl, data + sended, segmentsize) != segmentsize) {
            syslog(LOG_ERR, "Failed while sending msg. %s", strerror(errno));
            return ERROR;
        }
        sended += segmentsize;
        length -= segmentsize;
    }

    return sended;
}

int recibir_datos_SSL(SSL* ssl, void** data, size_t segmentsize, void* enddata, size_t enddata_len) {
    short finished_flag = 0;
    int total_received = 0;
    int just_received;
    void *buffer=NULL, *memalloc;

    if (segmentsize <= 0) {
        segmentsize = TCP_MSS_DEFAULT;
    }

    buffer = malloc(segmentsize);
    if (!buffer) {
        syslog(LOG_ERR, "Failed while allocating memory. %s", strerror(errno));
        return ERROR;
    }

    *data=NULL;
    while (!finished_flag) {
        memalloc=NULL;
        if ((just_received = SSL_read(ssl, buffer, segmentsize)) <= 0) {
            syslog(LOG_ERR, "Failed while receiving. %s", strerror(errno));
            return ERROR;
        }

        memalloc = realloc(*data, total_received + just_received + 1);
        if (!(memalloc)) {
            syslog(LOG_ERR, "Failed while allocating memory. %s", strerror(errno));
            return ERROR;
        }
        *data=memalloc;
        memcpy(*data+total_received, buffer, just_received);
        total_received += just_received;

        if (just_received < segmentsize || !memcmp(*data+(total_received-enddata_len), enddata, enddata_len)) {
            finished_flag = 1;
        }

    }

    free(buffer);
    return total_received;
}


void cerrar_canal_SSL(SSL* ssl) {
    int fd;
    fd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_shutdown(ssl);
    SSL_CTX_free(ssl->ctx);
    SSL_free(ssl);         
    close(fd);          /* close socket */
}

/*
• fijar_contexto_SSL : Esta función se encargará de inicializar correctamente el contexto que será utilizado para
la creación de canales seguros mediante SSL. Deberá recibir información sobre las rutas a los certificados y
claves con los que vaya a trabajar la aplicación.
• conectar_canal_seguro_SSL : Dado un contexto SSL y un descriptor de socket esta función se encargará de
obtener un canal seguro SSL inciando el proceso de handshake con el otro extremo.
• aceptar_canal_seguro_SSL : Dado un contexto SSL y un descriptor de socket esta función se encargará de
bloquear la aplicación, que se quedará esperando hasta recibir un handshake por parte del cliente.
• evaluar_post_connectar_SSL : Esta función comprobará una vez realizado el handshake que el canal de co-
municación se puede considerar seguro.
• enviar_datos_SSL : Esta función será el equivalente a la función de envío de mensajes que se realizó en la
práctica 1, pero será utilizada para enviar datos a través del canal seguro. Es importante que sea genérica y
pueda ser utilizada independientemente de los datos que se vayan a enviar.
• recibir_datos_SSL : Esta función será el equivalente a la función de lectura de mensajes que se realizó en la
práctica 1, pero será utilizada para enviar datos a través del canal seguro. Es importante que sea genérica y
pueda ser utilizada independientemente de los datos que se vayan a recibir.
• cerrar_canal_SSL : Esta función liberará todos los recursos y cerrará el canal de comunicación seguro creado
previamente.
*/
