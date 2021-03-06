#define _XOPEN_SOURCE
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "G-2301-03-P1-types.h"
#include "G-2301-03-P1-daemonize.h"

/*
 * Private function
 * 
 * Function: daemon_exit_handler
 * Implementation comments:
 *      None.
 */
void daemon_exit_handler(int signal){
    
    syslog(LOG_NOTICE, "Server: Received signal %d. Closing log file. Killing daemon.", signal);
    closelog();
    exit(EXIT_SUCCESS);
}

/*
 * Private function
 * 
 * Function: daemon_ignore_signals
 * Implentation comments:
 *      None.
 */
int daemon_ignore_signals(){
    
    if (signal(SIGTTOU, SIG_IGN) == SIG_ERR){
        return ERROR;
    } 
    if (signal(SIGTTIN, SIG_IGN) == SIG_ERR){
        return ERROR;
    }
    if (signal(SIGTSTP, SIG_IGN) == SIG_ERR){
        return ERROR;
    }
    
    return OK;
}

/*
 * Private function
 * 
 * Function: daemon_kill_parent
 * Implentation comments:
 *      Creates a child process and kills calling process using exit. By this 
 *      way, the child's parents will be the init process.
 */
int daemon_kill_parent(){
    
    pid_t child_pid;
    
    child_pid = fork();
    if (child_pid == (pid_t) -1){
        return ERROR;
    }
    else if (child_pid != (pid_t) 0){
        exit(EXIT_SUCCESS);
    }
    
    return OK;
    
}

/*
 * Private function
 * 
 * Function: dameon_close_files
 * Implentation comments:
 *      Close all files that could be opened with a loop from file descriptor 0 
 *      to the largest posible value for a file descriptor, which is got using
 *      "getdtablesize".
 * 
 *      It uses sysconf instead of getdtablesize for portability.
 */
int daemon_close_files(){
    
    int fd;
    long int tablesize = sysconf(_SC_OPEN_MAX);
    
    if (tablesize == -1){
        return ERROR;
    }
    
    for(fd = 0; fd < tablesize; ++fd){
        if(close(fd) == -1){
            break;
        }
    }
    
    return OK;
}

/*
 * Private function
 * 
 * Function: daemon_exiting_signals
 * Inplementation comments:
 *      It uses "sigaction" instead of "signal" due to the fact that signal is 
 *      being deprecated and its only portable use is for ignoring or setting
 *      the default action.
 */
int daemon_exiting_signals(){
    
    struct sigaction act;
    act.sa_handler = daemon_exit_handler;
    
    if(sigaction(SIGCHLD, &act, NULL) == -1){
        return ERROR;
    }
    if(sigaction(SIGPWR, &act, NULL) == -1){
        return ERROR;
    }
    
    return OK;
    
}

/*
 * Function: daemonizer
 * Implementation comments:
 *      -Errors will be reported using errno value.
 *      -If some operation fails, it closes the log file and return ERROR:
 *      -It calls "signal" function just to ignore signals because it is its 
 *      only portable use.
 */
int daemonize(const char *ident){
    
    /*Initializing logs*/
    openlog(ident, LOG_PID, LOG_DAEMON);
    syslog(LOG_NOTICE, "Server: Initializing daemon.\n");
    
    /*Ignoring signals*/
    if (daemon_ignore_signals() == ERROR){
        syslog(LOG_ERR, "Server: Failed while capturing signals: %s\n", strerror(errno));
        closelog();
        return ERROR;
    }
    
    /*Creating child, killing parent*/
    if (daemon_kill_parent() == ERROR){
        syslog(LOG_ERR, "Server: Failed while creating a child and killing the calling process: %s\n", strerror(errno));
        closelog();
        return ERROR;
    }
    
    /*Creating new session*/
    if(setsid() == -1){
       syslog(LOG_ERR, "Server: Failed while creating new session: %s\n", strerror(errno));
       closelog();
       return ERROR;
    }
    
    /*Losing tty control*/
    if(signal(SIGHUP, SIG_IGN) == SIG_ERR){
       syslog(LOG_ERR, "Server: Failed while capturing signal: %s\n", strerror(errno));
       closelog();
       return ERROR;
    }
    
    /*Changing mode creation mask*/
    umask(0);
    
    /*Changing current working directory*/
    if(chdir("/") == -1){
        syslog(LOG_ERR, "Server: Failed while changing the current working directory: %s\n", strerror(errno));
        closelog();
        return ERROR;
    }
    
    /*Closing files*/
    if(daemon_close_files() == ERROR){
        syslog(LOG_ERR, "Server: Failed while closing opened files: %s\n", strerror(errno));
        closelog();
        return ERROR;
    }
    
    /*Capturing exiting signals*/
    if(daemon_exiting_signals() == ERROR){
        syslog(LOG_ERR, "Server: Failed while capturing signals: %s\n", strerror(errno));
        closelog();
        return ERROR;
    }
    
    syslog(LOG_NOTICE, "Server: Daemon initialized correctly.\n");
    return OK;
}
