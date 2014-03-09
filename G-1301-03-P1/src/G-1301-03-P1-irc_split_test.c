#include "../includes/G-1301-03-P1-ircserver.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* comandos_prueba[6]={ 
    " :ab    cd   ef      ghi : j kl", //ejemplo cmd bien formado con prefijo
    " :ab : j kl ", //ejemplo comando mal formado (no hay comando, pero el programa no dara error: posicion del comando -1)
    "cd   ef      g:hi : j kl ", //ejemplo comando bien formado sin prefijo
    ":ab cd   ef      ghi j kl mno pqrst u vw xyz 012 3 45 6789 10 11 ", //ejemplo: 15 argumentos
    ":ab cd   ef      ghi j kl mno pqrst u vw xyz 012 3 45 6789 10 11 argumentNo16 ", //ejemplo: mas de 15 argumentos y prefijo
    "cd   ef      ghi j kl mno pqrst u vw xyz 012 3 45 6789 10 11 argumentNo16 " //ejemplo: mas de 15 argumentos sin prefijo
};

int main () {
	int n_args=0, prefijo=0, i, j;
	char *args[17];
	char *comando;
        comando=(char*)malloc(100);
        for (i=0; i<6; i++){
            strcpy(comando, comandos_prueba[i]);
            printf("Comando %d: \'%s\'\n", i ,comando);
            printf("Posicion del primer caracter del comando (-1 indica mensaje sin comando, mal formado): %d\n", irc_get_cmd_position(comando));
            if (irc_split_cmd(comando, args, &prefijo, &n_args)==ERROR_WRONG_SYNTAX) {
                printf("Mala sintaxis\n");
                continue;
            }
            printf("Prefijo: %s\n", prefijo==1?"si":"no");
            printf("Numero de argumentos: %d \n", n_args);
            for (j=0; j<n_args; j++) {
                printf("\t\'%s\'", args[j]);
            }
            printf("\n\n\n");
        }
        free(comando);

        return (EXIT_SUCCESS);
}
