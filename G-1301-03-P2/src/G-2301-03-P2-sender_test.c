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
#include <time.h>
#include "../includes/G-1301-03-P2-call_funcs.h"
#include "../includes/G-1301-03-P2-udp.h"
#include "../includes/G-1301-03-P2-sound.h"

int main (int argc, char *argv[]) {
    
    char rec[RECORD_SIZE], msg[MAX_BUF];
    rtp_header h;
    u_int16_t seq, port;
    u_int32_t timestamp, ssrc;
    struct sockaddr_in *caller_params=NULL;
    in_addr_t c_ip=0;
    char finished_call=0;
    int call_socket;
    
    
    call_socket=open_UDP_socket();
    bind_UDP(call_socket, 0);
    port=get_UDP_port(call_socket);
    printf("Sender in port %u\n", port);
    
    
    if ((c_ip=inet_addr(argv[1]))==INADDR_NONE) {
        syslog(LOG_ERR, "Client: ERROR while trying to call IP %s", argv[1]);
        return ERROR_PARAM;
    }
    if (!(caller_params=(struct sockaddr_in*)malloc(sizeof(struct sockaddr_in)))) {
        syslog(LOG_ERR, "Client: ERROR allocating memory for socket.");
        end_call();
        return ERROR_SOCKET;
    }
    memset((void*)caller_params, 0, sizeof(struct sockaddr_in));
    caller_params->sin_addr.s_addr=c_ip;
    caller_params->sin_port=htons(atol(argv[2]));
    printf("Calling to port %u in ip %s\n", atol(argv[2]), argv[1]);
    caller_params->sin_family=AF_INET;
    
    srand(time(NULL));
    setup_rtp_header(&h, 2, 0, 0, 0, 0, sampleFormat(PA_SAMPLE_ULAW, 1)); /*arguments are: header, v, p, x, cc, m, pt*/
    seq=rand();
    timestamp=rand();
    ssrc=rand();
    
    if (openRecord("Sender")) {
        printf("Client: ERROR Couldn't openRecord");
        finished_call=1;
        return 1;
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
        if (sendto(call_socket, msg, sizeof(rtp_header)+RECORD_SIZE, 0, (struct sockaddr *)caller_params, sizeof(struct sockaddr_in))<0) { /**/
            syslog(LOG_ERR, "Client: Sender Thread: Call ended");
            finished_call=1;
        }
    }
    closeRecord();
    free(caller_params);
    return 0;    
}