#include <stdio.h>
#include "G-2301-03-P3-SSL_ftransfer.h"
#include <unistd.h>
#include <syslog.h>

int main (int argc, char *argv[]) {
    FILE* fp=NULL;
    long file_size;
    long port;
    int i=0;
    fp=fopen("prueba2.txt", "r");
    if (NULL == fp) {
        printf("no such file\n");
        return -1;
    }
    if (fseek(fp, 0 , SEEK_END) != 0) {
        printf("fseek error\n");
        return -1;
    }
    file_size = ftell(fp);
    fclose(fp);
    printf("Size: %ld\n", file_size);
    
    //transfer(char* ip, u_int16_t port, int side, char* filename, u_int32_t size)
    inicializar_nivel_SSL();
    port=transfer("127.0.0.1", 0, SENDER_SIDE, "prueba2.txt", file_size);
    if (port<=0) {printf("Transfer dying\n"); return -1;}
    printf("Esperando conexion en %ld\n", port);
    while(!is_finished_transfer() && i<20) {sleep(1); i++;}
    end_transfer();
    printf("Fin\n");
    return 0;
}