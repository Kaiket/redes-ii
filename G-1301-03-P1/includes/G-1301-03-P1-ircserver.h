#ifndef __IRCSERVER_H
#define __IRCSERVER_H

#include <limits.h>

#define SERVER_LOG_IDENT "IRC_SERVER"
#define SERVER_MAX_CONNECTIONS INT_MAX
#define IRC_DEFAULT_PORT 6667
#define IRC_MSG_LENGTH 512
#define IRC_MSG_END "\r\n"

/*
 * Function: thread_routine
 * Parameters:
 *      arg: not used.
 * Description:
 *      Take charge of a client, executing the commands received by them.
 * Return value:
 *      void *: not used.
 */
void *irc_thread_routine(void *arg);


/*
 * Function: irc_exit_message
 * Parameters:
 *      none
 * Description:
 *      Writes on the log that the server is being closed.
 * Return value:
 *      none
 */
void irc_exit_message();

#endif