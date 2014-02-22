/*
 * Function: daemonizer
 * Parameters:
 *      ident: name of the log file that will be opened.
 * Description:
 *      It opens a log file named with the string contained in "ident".
 *      After it, it starts a daemon following these steps:
 *              -Ignores SIGTTOU, SIGTTIN and SIGTSTP signals
 *              -Creates a new session and loses the control of tty
 *              -Closes all the files, ignores SIGCHLD and SIGPWR
 *              -Change the working directory to / and sets the 
 *               file mode creation mask providing all permissions.
 * Return value:
 *      OK on success.
 *      ERROR on failure, and it writes the description of the error in the log
 *      file.
 */
int daemonizer(const char *ident);