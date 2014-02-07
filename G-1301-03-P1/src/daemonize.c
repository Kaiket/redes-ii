#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

void daemonize(){
    
    pid_t child_pid;
    pid_t session_pid;
    
    /*Ignoring signals*/
    if(signal(SIGTTOU, SIG_IGN) == SIG_ERR){
        perror("Error while ignoring signals");
        exit(EXIT_FAILURE);
    } 
    if(signal(SIGTTIN, SIG_IGN) == SIG_ERR){
        perror("Error while ignoring signals");
        exit(EXIT_FAILURE);
    }
    if(signal(SIGTSTP, SIG_IGN) == SIG_ERR){
        perror("Error while ignoring signals");
        exit(EXIT_FAILURE);
    }
    
    /*Creating child, killing parent*/
    child_pid = fork();
    if(child_pid == (pid_t) -1){
        perror("Error while creating child process");
        exit(EXIT_FAILURE);
    }
    else if (child_pid != (pid_t) 0){
        exit(EXIT_SUCCESS);
    }
    
    
    /*New session*/
    if(setsid() == (pid_t)-1){
        
    }
    
    /*Losing control of TTY*/
    if(signal(SIGHUP, SIG_IGN) == SIG_ERR){
        
    }
    
    /*Setting new file mode creation mask*/
    umask(0);
    
    /*Changing working directory*/
    chdir();
    
    /*Closing all files*/
    
    
    /*Waiting for being closed*/
}
