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
#include "../includes/G-1301-03-P2-call_funcs.h"
#include "../includes/G-1301-03-P2-udp.h"
#include "../includes/G-1301-03-P2-sound.h"

char calling=0;
char finished_call=0;
u_int32_t caller_ip=0;
int call_socket=0;
pthread_t sender_thread=0;
pthread_t receiver_thread=0;

/*
 * Creates an udp socket in the global variable call_socket.
 * Sets a timeout for the socket if the corresponding argument is > 0
 * Returns port number or several types of error
 */
u_int16_t setup_call(u_int16_t port, long int timeout) {
    u_int16_t port_bound=0;
    
    if (calling) {
        syslog(LOG_ERR, "Client: ERROR Already calling.");
        return ERROR_ALREADY_CALLING;
    }
    
    if (call_socket) {
        syslog(LOG_ERR, "Client: ERROR Already set up.");
        return ERROR_ALREADY_SETUP;
    }
    
    if ((call_socket=open_UDP_socket()) < 0) {
        syslog(LOG_ERR, "Client: ERROR creating UDP socket for the call.");
        call_socket=0;
        return ERROR_SOCKET;
    }
    
    if (bind_UDP(call_socket, port)<0) {
        syslog(LOG_ERR, "Client: ERROR Binding UDP port.");
        close_UDP_socket(call_socket);
        call_socket=0;
        return ERROR_BINDING;
    }
    
    if (timeout>0) {
        if (set_UDP_rcvtimeout(call_socket, timeout)==ERROR) {
            syslog(LOG_ERR, "Client: ERROR setting timeout for UDP socket");
            close_UDP_socket(call_socket);
            call_socket=0;
            return ERROR_TIMEOUTSET;
        }
    }
    
    if ((port_bound=get_UDP_port(call_socket)) < 0) {
        syslog(LOG_ERR, "Client: ERROR getting port number for UDP socket");
        close_UDP_socket(call_socket);
        call_socket=0;
        return ERROR_GETPORTBOUND;
    }
    return port_bound;    
}

char already_calling() {
    return calling;
}

char is_finished_call() {
    return finished_call;
}

int is_caller_ip(char *ip) {
    if (inet_addr(ip)==caller_ip) return 1;
    return 0;
}


/*
 * sets several default values of the rtp header which doesn't need to be modified more than once.
 */
void setup_rtp_header(rtp_header* h, int v, int p, int x, int cc, int m, int pt) {
    RTP_SETV(h, v);
    RTP_SETP(h, p);
    RTP_SETX(h, x);
    RTP_SETCC(h, cc);
    RTP_SETM(h, m);
    RTP_SETPT(h, pt);
    return;
}

/*
 * sets the rest of the values for the rtp header
 */
void build_rtp_header (rtp_header* h, u_int16_t seq, u_int32_t timestamp, u_int32_t ssrc) {
    h->sequence_number=seq;
    h->timestamp=timestamp;
    h->ssrc=ssrc;
    return;
}

void* sender_thread_routine(void *arg) {
    char rec[RECORD_SIZE], msg[MAX_BUF];
    rtp_header h;
    u_int16_t seq;
    u_int32_t timestamp, ssrc;
    struct sockaddr_in *caller_data=(struct sockaddr_in *) arg;
    
    srand(time(NULL));
    setup_rtp_header(&h, 2, 0, 0, 0, 0, sampleFormat(PA_SAMPLE_ULAW, 1)); /*arguments are: header, v, p, x, cc, m, pt*/
    seq=rand();
    timestamp=rand();
    ssrc=rand();
    
    if (openRecord("Sender")) {
        syslog(LOG_ERR, "Client: ERROR Couldn't openRecord");
        finished_call=1;
        pthread_exit(NULL);
    }
    
    while (!finished_call) {
        bzero(rec, RECORD_SIZE);
        bzero(msg, MAX_BUF);
        recordSound(rec, RECORD_SIZE);
        build_rtp_header(&h, htons(seq), htonl(timestamp), htonl(ssrc));
        memcpy(msg, &h, sizeof(rtp_header));
        memcpy(msg + sizeof(rtp_header), rec, RECORD_SIZE);
        seq++;
        timestamp+=20;
        if (sendto(call_socket, msg, sizeof(rtp_header)+RECORD_SIZE, 0, (struct sockaddr *)caller_data, sizeof(struct sockaddr_in))<0) { /**/
            syslog(LOG_ERR, "Client: Sender Thread: Call ended");
            finished_call=1;
        }
    }
    closeRecord();
    free(caller_data);
    pthread_exit(NULL);
}

void* receiver_thread_routine(void *arg) {
    char *rec, msg[MAX_BUF];
    rtp_header *h;
    struct sockaddr_in caller_data;
    socklen_t addrlen=(socklen_t)sizeof(caller_data);
    u_int32_t ssrc; 
    u_int16_t latest_seq_num, packet_seq_num;
    char first=1;
    
    if (openPlay("Receiver")) {
        syslog(LOG_ERR, "Client: ERROR Couldn't openPlay");
        finished_call=1;
        pthread_exit(NULL);
    }
    while (!finished_call) {
        if (recvfrom(call_socket, msg, MAX_BUF, 0, (struct sockaddr *)&caller_data, &addrlen) < 0) {
            syslog(LOG_ERR, "Client: Receiver Thread: Call ended");
            finished_call=1;
        }
        else {
            if (first==1) { /*first packet, check ip and get seq number and ssrc*/
                if (caller_data.sin_addr.s_addr==caller_ip) {
                    h=(rtp_header*)msg;
                    rec=msg+sizeof(rtp_header)+RTP_GETCC(h);
                    if (RTP_GETPT(h)!=0) continue; /*0 is ULAW, 1channel at 8khz*/
                    latest_seq_num=ntohs(h->sequence_number);
                    ssrc=ntohl(h->ssrc);
                    first=0;
                    usleep(100000); /*sleep for 100ms before playing first packet*/
                    playSound(rec, RECORD_SIZE);
                }
            }
            else {
                if (caller_data.sin_addr.s_addr==caller_ip) {
                    h=(rtp_header*)msg;
                    rec=msg+sizeof(rtp_header)+RTP_GETCC(h);
                    if ((RTP_GETPT(h)!=0) || (ntohl(h->ssrc)!=ssrc)) continue;
                    packet_seq_num=ntohs(h->sequence_number);
                    if ((packet_seq_num <= latest_seq_num) && (packet_seq_num > latest_seq_num+100)) continue; /*the second condition tries to deal with overflow of int types if we got a sequence number too close to the max it can store*/
                    latest_seq_num=packet_seq_num;
                    playSound(rec, RECORD_SIZE);
                }
            }
        }
    }
    closePlay();
    pthread_exit(NULL);
}

/*ip of caller and port in which he is expecting packets*/
int call(char* ip, u_int16_t port) {
    struct sockaddr_in *caller_params=NULL;
    in_addr_t c_ip=0;
    if (!ip) {
        return ERROR_PARAM;
    }
    if ((c_ip=inet_addr(ip))==INADDR_NONE) {
        syslog(LOG_ERR, "Client: ERROR while trying to call IP %s", ip);
        return ERROR_PARAM;
    }
    if (!(caller_params=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in)))) {
        syslog(LOG_ERR, "Client: ERROR allocating memory for sockaddr.");
        end_call();
        return ERROR_SOCKET;
    }
    memset((void*)caller_params, 0, sizeof(struct sockaddr_in));
    caller_params->sin_addr.s_addr=c_ip;
    caller_params->sin_port=htons(port);
    caller_params->sin_family=AF_INET;

    caller_ip=c_ip;
    calling=1;
    /*create sender thread*/
    if (!pthread_create(&sender_thread, NULL, sender_thread_routine, (void*)caller_params)) {
        sender_thread=0;
        syslog(LOG_ERR, "Client: ERROR creating audio recorder/sender thread.");
        end_call();
        return ERROR_THREAD;
    }
    
    /*create receiver thread*/
    if (!pthread_create(&receiver_thread, NULL, receiver_thread_routine, NULL)) {
        receiver_thread=0;
        syslog(LOG_ERR, "Client: ERROR creating audio player/receiver thread.");
        end_call();
        return ERROR_THREAD;
    }
    return OK;
}

/*ends a call*/
void end_call() {
    if (!finished_call) {
        shutdown(call_socket, SHUT_RDWR); /*shutdown to unlock receiving operations*/
        finished_call=1;
    }
    else {
        close(call_socket);
    }
    if (sender_thread) {
        pthread_join(sender_thread, NULL);
    }
    if (receiver_thread) {
        pthread_join(receiver_thread, NULL);
    }
    calling=0;
    call_socket=0;
    caller_ip=0;
    finished_call=0;
    sender_thread=0;
    receiver_thread=0;
    return;
}
