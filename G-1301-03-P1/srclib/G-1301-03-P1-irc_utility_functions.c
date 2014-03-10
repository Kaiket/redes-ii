#include <stdio.h>
#include <string.h>
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-ircserver.h"

int is_letter_char(char c) {
    if ((c>='A' && c<='Z') || (c>='a' && c<='z')) return OK;
    return ERROR;
}

int is_special_char(char c) {
    if ((c>=0x5B && c<=0x60) || (c>=0x7B && c<=0x7D)) return OK;
    return ERROR;
}

int is_digit_char(char c) {
    if ((c>=0x30) && (c<=0x39)) return OK;
    return ERROR;
}

int is_valid_nick(char* nick) {
    int i=1;
    
    if (!is_letter_char(nick[0]) && !is_special_char(nick[0])) return ERROR;
    
    while (nick[i]!='\0') {
        if (i>IRC_MAX_NICK_LENGTH) return ERROR;
        if (!is_letter_char(nick[i]) && !is_special_char(nick[i]) && !is_digit_char(nick[i]) && (nick[i]!='-')) 
            return ERROR;
        ++i;
    }
    return OK;
}

