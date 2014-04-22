#ifndef __CALL_FUNCS_H
#define __CALL_FUNCS_H

#include <sys/types.h>

#ifndef OK
        #define OK 0
#endif
#ifndef ERROR
        #define ERROR -1
#endif
#define ERROR_ALREADY_CALLING -2
#define ERROR_ALREADY_SETUP -3
#define ERROR_SOCKET -4
#define ERROR_BINDING -5
#define ERROR_TIMEOUTSET -6
#define ERROR_GETPORTBOUND -7
#define ERROR_PARAM -8
#define ERROR_THREAD -9


#define TIMEOUT_DEFAULT 10000
#define MAX_BUF 256
#define RECORD_SIZE 160


/*
 * Estructura para la cabecera RTP.
 */

/*
 * 
     0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                           timestamp                           |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           synchronization source (SSRC) identifier            |
   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
   |            contributing source (CSRC) identifiers             |
   |                             ....                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 
 * 
 */
typedef struct __attribute__ ((__packed__)) struct_rtp{
    u_int8_t v_p_x_cc;
    u_int8_t m_pt;
    u_int16_t sequence_number;
    u_int32_t timestamp;
    u_int32_t ssrc;
} rtp_header;

#define RTP_SETV(h, v) ((h)->v_p_x_cc = (((h)->v_p_x_cc&0x3F)|((v)*64)))
#define RTP_GETV(h) (((h)->v_p_x_cc >> 6)&0x03)

#define RTP_SETP(h, p) ((h)->v_p_x_cc = (((h)->v_p_x_cc&0xDF)|(((p)*32)&0x20)))
#define RTP_GETP(h) (((h)->v_p_x_cc >> 5)&0x01)

#define RTP_SETX(h, x) ((h)->v_p_x_cc = (((h)->v_p_x_cc&0xEF)|(((x)*16)&0x10)))
#define RTP_GETX(h) (((h)->v_p_x_cc >> 4)&0x01)

#define RTP_SETCC(h, cc) ((h)->v_p_x_cc = (((h)->v_p_x_cc&0xF0)|((cc)&0x0F)))
#define RTP_GETCC(h) (((h)->v_p_x_cc)&0x0F)

#define RTP_SETM(h, m) ((h)->m_pt = (((h)->m_pt&0x7F)|((m)&0x80)))
#define RTP_GETM(h) (((h)->m_pt >> 7)&0x01)

#define RTP_SETPT(h, pt) ((h)->m_pt = (((h)->m_pt&0x80)|((pt)&0x7F)))
#define RTP_GETPT(h) (((h)->m_pt)&0x7F)


/** 
 * @brief Setups a socket to make a call
 * 
 * @details Creates an UDP socket and binds it to the port passed (0 for any) and
 * sets its timeout to "timeout" miliseconds (0 for no timeout).
 * Leaves the socket ready to "call"
 * 
 * @param port port to bind the socket to.
 * @param timeout time in miliseconds before socket closure when performing a blocking recv.
 * 
 * @return port to which the socket was bound.
 *      ERROR_ALREADY_CALLING  A call is in progress, must end it first
 *      ERROR_ALREADY_SETUP  A Socket unused already exists, use it or end_call() first.
 *      ERROR_SOCKET Error creating the socket.
 *      ERROR_BINDING Error binding socket
 *      ERROR_TIMEOUTSET Error setting timeout for the socket.
 *      ERROR_GETPORTBOUND Error getting port to which the socket was bound.
 *
 * @see call(), end_call()
 */
u_int16_t setup_call(u_int16_t port, long int timeout);

/** 
 * @brief Makes a call to an IP and port
 * 
 * @details Uses the already set up socket (see setup_call) to
 * make a call to the IP and port specified.
 * Creates two threads that start sending/playing audio recorded/received
 * An error in parameters passed (ERROR_PARAM) lets socket still usable if this function
 * is called again with correct parameters.
 * Any other error, closes the socket and a setup_call must be done before trying to call again.
 * 
 * @param ip string containing ip to call (i.e "194.12.1.2")
 * @param port Port number in which such IP is expecting the connection.
 * 
 * @return 0 on success.
 *      ERROR_PARAM ip specified is not correct (or not IPv4)
 *      ERROR_SOCKET Error alocating memory for sockaddr_in variables
 *      ERROR_THREAD Error creating threads to manage the call.
 *      
 *
 * @see setup_call(), end_call()
 */
int call(char* ip, u_int16_t port);

/** 
 * @brief If a call is in progress
 *  
 * @details A global variable is set to 1 when a conversation starts ("call" function succeded).
 * Thus, with this function can be checked if a call is in progress.
 * 
 * @return 0 if no call is in progress, 1 if a call is in progress
 *
 */
char already_calling();

/** 
 * @brief If a call has finished 
 * 
 * @details If a call has finished (because of the socket timing out) and no end_call()
 * has been performed, another call cannot be set up.
 * This function checks if a call has already ended so the needed end_call() is done by the developer.
 * 
 * @return 0 if no call has finished, 1 if a call has finished
 *
 * @see end_call, call(), setup_call()
 */
char is_finished_call();

/** 
 * @brief If the ip passed maches caller ip
 * 
 * @details This functions checks if the ip passed matches the ip address of the caller
 * of the call already in progress(if no call in progress it will compare to ip 0.0.0.0)
 * 
 * @param ip String representing ip to check (i.e "154.1.123.5")
 * 
 * @return 0 if IPs doesn't match, 1 if they do.
 *
 */
int is_caller_ip(char *ip);

/** 
 * @brief Ends a call
 * 
 * @details Closes socket and terminates threads that manages the call.
 * Sets every global variable to 0 so another call can be initiated (after another setup_call)
 * 
 */
void end_call();

/** 
 * @brief Builds a part of an rtp_header
 * 
 * @details Sets the part of the rtp header to the values passed as argument.
 * See rfc 3550 for details on rtp header..
 * Usually these parameters are not modified during connection, thats why they can be fixed and just
 * call build_rtp_header to complete the header for every packet sent.
 * 
 * This function is not needed for the standard procedure of a call, but it can be used to build rtp headers with other purposes
 * 
 * @param h rtp header to fill with paramters
 * @param v v field in rtp header
 * @param p p field in rtp header
 * @param x x field in rtp header
 * @param cc cc field in rtp header
 * @param m m field in rtp header
 * @param pt pt field in rtp header
 *
 * @see build_rtp_header
 */
void setup_rtp_header(rtp_header* h, int v, int p, int x, int cc, int m, int pt);

/** 
 * @brief Builds a part of an rtp_header
 * 
 * @details Sets the part of the rtp header to the values passed as argument.
 * See rfc 3550 for details on rtp header..
 * These parameters usually change with every new packet sent.
 * 
 * This function is not needed for the standard procedure of a call, but it can be used to build rtp headers with other purposes
 * 
 * @param h rtp header to fill with paramters
 * @param seq seq field in rtp header
 * @param timestamp timestamp field in rtp header
 * @param ssrc ssrc field in rtp header
 *
 * @see setup_rtp_header
 */
void build_rtp_header (rtp_header* h, u_int16_t seq, u_int32_t timestamp, u_int32_t ssrc);


#endif