#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD1 "command1"
#define CMD2 "command2"
#define CMD3 "command3"
#define CMD1 "command4"

enum {
    cmd1,
    cmd2,
    cmd3,
    cmd4,
    TOTAL
} command;

char commands{"cmd1", "cmd2", "cmd3", "cmd4"};

int parser(int total, char* strings[], char* msg) {
    int i;
    for (i=0; i<total; ++i) {
        if (strncasecmp(msg, strings[i], strlen(strings[i]))==0) {
            break;
        }
    }
    return i;
}

int exec_cmd (int number) {
    switch(number) {
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
        case 4:
            break;
        default:
    }
}