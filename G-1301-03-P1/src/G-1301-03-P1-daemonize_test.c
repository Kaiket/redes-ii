#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>
#include "../includes/G-1301-03-P1-types.h"
#include "../includes/G-1301-03-P1-daemonize.h"


int main(int argc, char **argv){
    
    if(argc != 2){
        printf("Error de argumentos.\nUso:\n\t %s nombre_log\n", argv[0]);
        return ERROR;
    }
    
    printf("Inicializando daemon.\nLog: %s.\n", argv[1]);
    printf("Para matar al demonio, enviar señal SIGCHLD o SIGPWR.\n");
    if(daemonize(argv[1]) == ERROR){
        printf("Error al daemonizar.\n");
        return ERROR;
    }

    syslog(LOG_INFO, "Proceso con PID %d y PPID %d esperando SIGCHLD o SIGPWR.", getpid(), getppid());
    pause();

    return OK;
    
}
