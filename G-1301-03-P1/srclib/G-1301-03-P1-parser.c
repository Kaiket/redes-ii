#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int parser(int total, char* strings[], char* msg) {
    int i;
    for (i=0; i<total; ++i) {
        if (strncasecmp(msg, strings[i], strlen(strings [i]))==0) {
            break;
        }
    }
    return i;
}