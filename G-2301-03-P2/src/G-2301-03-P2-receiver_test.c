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
#include <errno.h>
#include "../includes/G-1301-03-P2-call_funcs.h"
#include "../includes/G-1301-03-P2-udp.h"
#include "../includes/G-1301-03-P2-sound.h"

int main (int argc, char *argv[]) {
    int fd;
    u_int16_t port;
    char f_call=0;
    char *rec, msg[MAX_BUF];
    rtp_header *h;
    struct sockaddr_in caller_data;
    socklen_t addrlen=sizeof(caller_data);
    u_int32_t ssrc; 
    u_int16_t latest_seq_num, packet_seq_num;
    char first=1;
    in_addr_t caller_ip=inet_addr(argv[1]);
    
    openlog("RECEIVER", LOG_PID, LOG_LOCAL1);
    fd=open_UDP_socket();
    bind_UDP(fd, 0);
    port=get_UDP_port(fd);
    set_UDP_rcvtimeout(fd, 20000);
    printf("Receiver in port %u\n", port);
    
    bzero((void*)&caller_data, sizeof(caller_data));
    
    if (openPlay("Receiver")) {
        printf("Client: ERROR Couldn't openPlay");
        f_call=1;
        close(fd);
        return -1;
    }
    while (!f_call) {
        if (recvfrom(fd, msg, MAX_BUF, 0, (struct sockaddr *)&caller_data, &addrlen) < 0) {
            printf("Client: Receiver Timed out Call ended, port %u %d\n", ntohs(caller_data.sin_port), errno);
            f_call=1;
        }
        else {
            if (first==1) { /*first packet, check ip and get seq number and ssrc*/
                if (caller_data.sin_addr.s_addr==caller_ip) {
                    h=(rtp_header*)msg;
                    rec=msg+sizeof(rtp_header)+RTP_GETCC(h);
                    if (RTP_GETPT(h)!=0) continue;
                    latest_seq_num=ntohs(h->sequence_number);
                    ssrc=ntohl(h->ssrc);
                    first=0;
                    usleep(120000);
                    playSound(rec, RECORD_SIZE);
                }
            }
            else {
                if (caller_data.sin_addr.s_addr==caller_ip) {
                    h=(rtp_header*)msg;
                    rec=msg+sizeof(rtp_header)+RTP_GETCC(h);
                    if ((RTP_GETPT(h)!=0) || (ntohl(h->ssrc)!=ssrc)) continue;
                    packet_seq_num=ntohs(h->sequence_number);
                    if ((packet_seq_num <= latest_seq_num) && (packet_seq_num > latest_seq_num+100)) 
                    continue; /*the second condition tries to deal with overflow of int types if we got a sequence number too close to the max it can store*/
                    latest_seq_num=packet_seq_num;
                    playSound(rec, RECORD_SIZE);
                }
            }
        }
    }
    closePlay();    
    closelog();
    return 0;
}