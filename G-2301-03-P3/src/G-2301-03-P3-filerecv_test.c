#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "G-2301-03-P3-SSL_ftransfer.h"

int main (int argc, char *argv[]) {
    long file_size;
    long port;
    int i=0;
    
    file_size=atol(argv[1]);
    port=atol(argv[2]);
    //transfer(char* ip, u_int16_t port, int side, char* filename, u_int32_t size)
    inicializar_nivel_SSL();
    port=transfer("127.0.0.1", port, RECEIVER_SIDE, "./hola/prueba2.txt", file_size);
    if (port < 0) {printf("Transfer dying\n"); return -1;}
    printf("Downloading\n");
    while(!is_finished_transfer() && i<20) {sleep(1); i++;}
    end_transfer();
    printf("Fin\n");
    return 0;
}