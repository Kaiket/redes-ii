#ifndef __DAEMONIZER_H
#define __DAEMONIZER_H

/*
 * Function: daemonizer
 * Parameters:
 *      ident: name of the log file that will be opened.
 * Description:
 *      It opens a log file named with the string pointed by "ident".
 *      After it, it starts a daemon following these steps:
 *              -Ignores SIGTTOU, SIGTTIN and SIGTSTP signals.
 *              -Creates a new session, becomes INIT child.
 *              -Loses the control of tty.
 *              -Closes all the files, ignores SIGCHLD and SIGPWR.
 *              -Change the working directory to / and sets the
 *               file mode creation mask providing all permissions.
 * Return value:
 *      OK on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file and closes it.
 */
int daemonize(const char *ident);

#endif