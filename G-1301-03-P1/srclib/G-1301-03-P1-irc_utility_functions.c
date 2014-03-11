#include <stdio.h>
#include <string.h>
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-ircserver.h"
#include "../includes/uthash.h"
#include "../includes/utlist.h"

#define TRUE 1
#define FALSE 0

int user_registered_flag(char flags) {
    if (flags & USER_REGISTERED) return TRUE;
    return FALSE;
}

int is_letter_char(char c) {
    if ((c>='A' && c<='Z') || (c>='a' && c<='z')) return TRUE;
    return FALSE;
}

int is_special_char(char c) {
    if ((c>=0x5B && c<=0x60) || (c>=0x7B && c<=0x7D)) return TRUE;
    return FALSE;
}

int is_digit_char(char c) {
    if ((c>=0x30) && (c<=0x39)) return TRUE;
    return FALSE;
}

int is_valid_nick(char* nick) {
    int i=1;
    
    if (!is_letter_char(nick[0]) && !is_special_char(nick[0])) return FALSE;
    
    while (nick[i]!='\0') {
        if (i>IRC_MAX_NICK_LENGTH) return FALSE;
        if (!is_letter_char(nick[i]) && !is_special_char(nick[i]) && !is_digit_char(nick[i]) && (nick[i]!='-')) 
            return FALSE;
        ++i;
    }
    return TRUE;
}

int is_valid_pass(char* pass) {
    int i=0;
    
    while (pass[i]!='\0') {
        if (i>IRC_MAX_PASS_LENGTH) return FALSE;
        if ((pass[i]<0x01 || pass[i]>0x05) && \
            (pass[i]<0x07 || pass[i]>0x08) && \
            (pass[i]<0x0E || pass[i]>0x1F) && \
            (pass[i]<0x21 || pass[i]>0x7F) && \
            (pass[i]!=0x0C)) return FALSE;
        ++i;
    }
    if (i==0) return FALSE;
    return TRUE;
}

/*
 * hash tables manipulation functions
 */
void user_hasht_add (user *item) {
    HASH_ADD_STR(server_data.users_hasht, nick, item);
}

void user_hasht_remove(user *item) {
    HASH_DEL(server_data.users_hasht, item);
}

user* user_hasht_find(char key[IRC_MAX_NICK_LENGTH + 1]) {
    user* found=NULL;
    HASH_FIND_STR(server_data.users_hasht, key, found);
    return found;
}

void channel_hasht_add (channel *item) {
    HASH_ADD_KEYPTR(hh, server_data.channels_hasht, item->name, strlen(item->name), item);
}

void channel_hasht_remove(channel *item) {
    HASH_DEL(server_data.channels_hasht, item);
}

channel* channel_hasht_find(char *key) {
    channel* found=NULL;
    HASH_FIND_STR(server_data.channels_hasht, key, found);
    return found;
}


/*
 * linked lists manipulation functions
 */
int add_nick_to_llist (char nick[IRC_MAX_NICK_LENGTH + 1], active_nicks** llist) {
    active_nicks* new=NULL;
    if (!(new=(active_nicks*)malloc(sizeof(active_nicks)))) return ERROR;
    if (!memcpy(new->nick, nick, IRC_MAX_NICK_LENGTH + 1)) {
        free(new);
        return ERROR;
    }
    new->next=NULL;
    LL_PREPEND(*llist, new);
    return OK;
}

/*
 * OK on success
 * ERROR if there was not such item
 */
int remove_nick_from_llist (char nick[IRC_MAX_NICK_LENGTH + 1], active_nicks** llist) {
    active_nicks *elem=NULL, *tmp;
    LL_FOREACH_SAFE(*llist ,elem, tmp) { 
        if (!strncmp(nick, elem->nick, strlen(nick))) {
            LL_DELETE(*llist, elem);
            free(elem);
            return OK;
        }
    }
    return ERROR;
}

active_nicks* find_nick_in_llist (char nick [IRC_MAX_NICK_LENGTH + 1], active_nicks** llist) {
    active_nicks *elem=NULL;
    LL_FOREACH(*llist ,elem) { 
        if (!strncmp(nick, elem->nick, strlen(nick))) {
            return elem;
        }
    }
    return NULL;
}

int add_chname_to_llist (char *ch_name, channel_lst** llist) {
    channel_lst *new=NULL;
    if (!(new=(channel_lst*)malloc(sizeof(channel_lst)))) return ERROR;
    new->ch_name=NULL;
    if (!(new->ch_name=strdup(ch_name))) {
        free(new);
        return ERROR;
    }
    new->next=NULL;
    LL_PREPEND(*llist, new);
    return OK;
}

int remove_chname_from_llist (char *ch_name, channel_lst** llist) {
    channel_lst *elem=NULL, *tmp;
    LL_FOREACH_SAFE(*llist ,elem, tmp) { 
        if (!strncasecmp(ch_name, elem->ch_name, strlen(ch_name))) {
            LL_DELETE(*llist, elem);
            free(elem->ch_name);
            free(elem);
            return OK;
        }
    }
    return ERROR;
}

channel_lst* find_chname_in_llist (char *ch_name, channel_lst** llist) {
    channel_lst *elem=NULL;
    LL_FOREACH(*llist ,elem) { 
        if (!strncasecmp(ch_name, elem->ch_name, strlen(ch_name))) {
            return elem;
        }
    }
    return NULL;
}