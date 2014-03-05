#include "../includes/G-1301-03-P1-types.h"
#include "../includes/G-1301-03-P1-thread_handling.h"
#include "../includes/G-1301-03-P1-ircserver.h"
#include <syslog.h>

/*
 * Function: thread_routine
 * Implementation comments:
 * 
 */
void *irc_thread_routine(void *arg) {

    int received;
    void *data;
    Thread_handler *settings = (Thread_handler *) arg;


    syslog(LOG_NOTICE, "Server: New thread created for socket %d\n", settings->socket);

    while ((received = receive_msg(settings->socket, &data, IRC_MSG_LENGTH, IRC_MSG_END, strlen(IRC_MSG_END))) != ERROR) {

        /*Do things*/
        free(data);
    }

    pthread_exit(NULL);
}

/*
 * Function: irc_exit_message
 * Implementation comments:
 *      none
 */
void irc_exit_message() {
    syslog(LOG_ALERT, "Server: finished.");
}