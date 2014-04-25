#include <stdio.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <syslog.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdlib.h>
#include <pulse/sample.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include "G-2301-03-P2-call_funcs.h"
#include "G-2301-03-P2-udp.h"
#include "G-2301-03-P3-SSL_ftransfer.h"
#include "G-2301-03-P3-SSL_funcs.h"
#include "G-2301-03-P1-connection.h"
#include "G-2301-03-P2-udp.h"
#include "G-2301-03-P2-sound.h"
#include "G-2301-03-P2-udp.h"

#define FINISHED_ERR -1
#define FINISHED_ERR_SSL -2
#define FINISHED_OK 1
#define MYCERTS "../cert/SCASignedCert.pem"
#define MYCERTC "../cert/CCASignedCert.pem"
#define CACERT "../cert/CACert.pem"
#define DOWNLOAD_DIR "download/"
#define BUF_SIZE 16000

char transfering=0;
char finished_transfer=0;
u_int32_t sender_ip=0;
SSL* transfer_ssl=NULL;
pthread_t transfer_thread=0;
int server_socket;
int peer_socket;
int transfer_side;
u_int32_t transfer_size=0;
char transfer_filename[255];
FILE* transfer_file;

char already_transfering() {
    return transfering;
}

char is_finished_transfer() {
    return finished_transfer;
}

int is_sender_ip(char *ip) {
    if (inet_addr(ip)==sender_ip) return 1;
    return 0;
}

void* trsender_thread_routine(void *arg) {
    SSL_CTX* ctx=NULL;    
    SSL* ssl;
    char buf[BUF_SIZE];
    size_t read=0;
    
    /*The sender acts as a server, so we perform the standard procedure to accept and verify SSL connection*/
    peer_socket = accept_connections(server_socket);
    if (peer_socket==ERROR) {
        syslog(LOG_NOTICE,"ERROR accepting connection\n");
        finished_transfer=FINISHED_ERR_SSL;
        pthread_exit(NULL);
    }
    if (!(ctx=fijar_contexto_SSL(MYCERTS, CACERT, &SSLv23_method, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT))) {
        syslog(LOG_NOTICE,"ERROR al fijar contexto\n");
        finished_transfer=FINISHED_ERR_SSL;
        pthread_exit(NULL);
    }
    if (!(ssl=aceptar_canal_seguro_SSL(ctx, peer_socket))) {
        close(peer_socket);
        SSL_CTX_free(ctx);
        finished_transfer=FINISHED_ERR_SSL;
        pthread_exit(NULL);
    }
    transfer_ssl=ssl;
    if (evaluar_post_conectar_SSL(ssl)!=OK) {
        syslog(LOG_NOTICE,"El certificado del peer no es valido\n");
        finished_transfer=FINISHED_ERR_SSL;
        cerrar_canal_SSL(transfer_ssl);
        pthread_exit(NULL);
    }
    /*connection succesful, sending file*/
    if (!(transfer_file=fopen((char*)arg, "r"))) {
        finished_transfer=FINISHED_ERR;
        pthread_exit(NULL);
    }
    while (!feof(transfer_file) && !finished_transfer) {/* get request */
        read=fread(buf, sizeof(char), BUF_SIZE, transfer_file);
        enviar_datos_SSL(transfer_ssl, buf, read, BUF_SIZE);
    }
    fclose(transfer_file);
    finished_transfer=FINISHED_OK;
    pthread_exit(NULL);
}

void* trreceiver_thread_routine(void *arg) {
    SSL_CTX* ctx=NULL;
    char localname[255]="\0", *fn;
    SSL* ssl;
    char *buf=NULL;
    size_t read=0;
    u_int32_t transfered=0;
    
    /*The receiver acts as a client, so we perform the standard procedure to accept and verify SSL connection*/
    if (!(ctx=fijar_contexto_SSL(MYCERTC, CACERT, &SSLv23_method, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT))) {
        syslog(LOG_NOTICE,"ERROR al fijar contexto\n");
        finished_transfer=FINISHED_ERR_SSL;
        pthread_exit(NULL);
    }
    if (!(ssl=conectar_canal_seguro_SSL(ctx, peer_socket))) {
        close(peer_socket);
        SSL_CTX_free(ctx);
        finished_transfer=FINISHED_ERR_SSL;
        pthread_exit(NULL);
    }
    transfer_ssl=ssl;
    if (evaluar_post_conectar_SSL(ssl)!=OK) {
        syslog(LOG_NOTICE,"El certificado del peer no es valido\n");
        finished_transfer=FINISHED_ERR_SSL;
        cerrar_canal_SSL(transfer_ssl);
        pthread_exit(NULL);
    }
    strcat(localname, DOWNLOAD_DIR);
    if ((fn=strrchr(transfer_filename, '/'))) {
        strcat(localname, fn+1);
    }
    else {
        strcat(localname, transfer_filename);
    }
    strcpy(transfer_filename, localname);
    /*connection succesful, sending file*/
    if (!(transfer_file=fopen( transfer_filename, "w+"))) {
        finished_transfer=FINISHED_ERR;
        pthread_exit(NULL);
    }
    while (transfered<transfer_size && !finished_transfer) {/* get request */
        read=recibir_datos_SSL(transfer_ssl, (void**)&buf, BUF_SIZE+1, "\0", 1);
        fwrite(buf, sizeof(char), read, transfer_file);
        transfered+=read;
        free(buf);
        buf=NULL;
    }
    fclose(transfer_file);
    finished_transfer=FINISHED_OK;
    pthread_exit(NULL);
}

/*ip of caller and port in which he is expecting packets*/
long transfer(char* ip, u_int16_t port, int side, char* filename, u_int32_t size) {
    in_addr_t c_ip=0;
    char service[PORT_LEN];
    struct addrinfo hints, *result, *rp;
    
    if (transfering) return ERROR_ALREADY_TRANSFERING;
    transfer_side=side;
    transfer_size=size;
    transfering=1;
    
    if (side==SENDER_SIDE) { /*SENDER SIDE*/
        if ((server_socket=init_server(port, 1))==ERROR) {
            return ERROR_SOCKET;
        }
        if (pthread_create(&transfer_thread, NULL, trsender_thread_routine, (void*)filename)) {
            transfer_thread=0;
            syslog(LOG_ERR, "Client: ERROR creating file sender thread.");
            end_transfer();
            return ERROR_THREAD;
        }
        return get_UDP_port(server_socket);
    }
    else { /*RECEIVER SIDE*/
        if ((c_ip=inet_addr(ip))==INADDR_NONE) {
            syslog(LOG_ERR, "Client: ERROR while trying to call IP %s", ip);
            end_transfer();
            return ERROR_PARAM;
        }
        sender_ip=c_ip;
        strcpy(transfer_filename, filename);

        /*Hints initialization*/
        bzero(&hints, sizeof(struct addrinfo));
        hints.ai_flags = 0;                 /*No flags*/
        hints.ai_family = AF_UNSPEC;        /*Either IPv4 or IPv6*/
        hints.ai_socktype = SOCK_STREAM;    /*TCP socket*/
        hints.ai_protocol = IPPROTO_TCP;    /*TCP protocol*/
        hints.ai_addrlen = 0;               /*Must be zero/NULL to call getaddrinfo*/
        hints.ai_addr = NULL;               /*Must be zero/NULL to call getaddrinfo*/
        hints.ai_canonname = NULL;          /*Must be zero/NULL to call getaddrinfo*/
        hints.ai_next = NULL;               /*Must be zero/NULL to call getaddrinfo*/

        sprintf(service, "%d", port);
        if(getaddrinfo(ip, service, &hints, &result) != 0){
            end_transfer();
            return ERROR_SOCKET;
        }
        
        for (rp = result; rp != NULL; rp = rp->ai_next) {
            peer_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (peer_socket != -1){
                if (connect(peer_socket, rp->ai_addr, rp->ai_addrlen) != -1){
                    freeaddrinfo(result);
                    if(pthread_create(&transfer_thread, NULL, &trreceiver_thread_routine, NULL) == ERROR){
                        close(peer_socket);
                        end_transfer();
                        return ERROR_SOCKET;
                    }
                    return OK;
                }
                close(peer_socket);
            }
        }
        freeaddrinfo(result);
        return ERROR_SOCKET;
    }
    return OK;
}

/*ends a transfer*/
void end_transfer() {
    
    if (!finished_transfer || finished_transfer==FINISHED_ERR || finished_transfer==FINISHED_OK) {
        if (transfer_ssl) {
                cerrar_canal_SSL(transfer_ssl);
                transfer_ssl=NULL;
        }
        finished_transfer=1;
    }
    if (transfer_thread) {
        pthread_join(transfer_thread, NULL);
        transfer_thread=0;
    }
    sender_ip=0;
    if (server_socket) close(server_socket);
    server_socket=0;
    peer_socket=0;
    transfer_side=0;
    transfer_file=NULL;
    finished_transfer=0;
    transfering=0;
    return;
}
