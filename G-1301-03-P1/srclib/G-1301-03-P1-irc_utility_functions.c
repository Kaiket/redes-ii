#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-ircserver.h"
#include "../includes/uthash.h"
#include "../includes/utlist.h"

#define TRUE 1
#define FALSE 0

int user_mode_a(char flags) {
    if (flags & US_MODE_a) return TRUE;
    return FALSE;
}

int user_mode_i(char flags) {
    if (flags & US_MODE_i) return TRUE;
    return FALSE;
}

int user_mode_w(char flags) {
    if (flags & US_MODE_w) return TRUE;
    return FALSE;
}

int user_mode_r(char flags) {
    if (flags & US_MODE_r) return TRUE;
    return FALSE;
}

int user_mode_o(char flags) {
    if (flags & US_MODE_o) return TRUE;
    return FALSE;
}

int user_mode_O(char flags) {
    if (flags & US_MODE_O) return TRUE;
    return FALSE;
}

int user_mode_s(char flags) {
    if (flags & US_MODE_s) return TRUE;
    return FALSE;
}

int user_registered(char flags) {
    if (flags & USER_REGISTERED) return TRUE;
    return FALSE;
}

int chan_mode_O(unsigned int flags) {
    if (flags & CH_MODE_O) return TRUE;
    return FALSE;
}

int chan_mode_o(unsigned int flags) {
    if (flags & CH_MODE_o) return TRUE;
    return FALSE;
}

int chan_mode_v(unsigned int flags) {
    if (flags & CH_MODE_v) return TRUE;
    return FALSE;
}

int chan_mode_a(unsigned int flags) {
    if (flags & CH_MODE_a) return TRUE;
    return FALSE;
}

int chan_mode_i(unsigned int flags) {
    if (flags & CH_MODE_i) return TRUE;
    return FALSE;
}

int chan_mode_m(unsigned int flags) {
    if (flags & CH_MODE_m) return TRUE;
    return FALSE;
}

int chan_mode_n(unsigned int flags) {
    if (flags & CH_MODE_n) return TRUE;
    return FALSE;
}

int chan_mode_q(unsigned int flags) {
    if (flags & CH_MODE_q) return TRUE;
    return FALSE;
}

int chan_mode_p(unsigned int flags) {
    if (flags & CH_MODE_p) return TRUE;
    return FALSE;
}

int chan_mode_s(unsigned int flags) {
    if (flags & CH_MODE_s) return TRUE;
    return FALSE;
}

int chan_mode_r(unsigned int flags) {
    if (flags & CH_MODE_r) return TRUE;
    return FALSE;
}

int chan_mode_t(unsigned int flags) {
    if (flags & CH_MODE_t) return TRUE;
    return FALSE;
}

int chan_mode_k(unsigned int flags) {
    if (flags & CH_MODE_k) return TRUE;
    return FALSE;
}

int chan_mode_l(unsigned int flags) {
    if (flags & CH_MODE_l) return TRUE;
    return FALSE;
}

int chan_mode_b(unsigned int flags) {
    if (flags & CH_MODE_b) return TRUE;
    return FALSE;
}

int chan_mode_e(unsigned int flags) {
    if (flags & CH_MODE_e) return TRUE;
    return FALSE;
}

int chan_mode_I(unsigned int flags) {
    if (flags & CH_MODE_I) return TRUE;
    return FALSE;
}

int is_letter_char(char c) {
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) return TRUE;
    return FALSE;
}

int is_special_char(char c) {
    if ((c >= 0x5B && c <= 0x60) || (c >= 0x7B && c <= 0x7D)) return TRUE;
    return FALSE;
}

int is_digit_char(char c) {
    if ((c >= 0x30) && (c <= 0x39)) return TRUE;
    return FALSE;
}

int is_valid_nick(char* nick) {
    int i = 1;

    if (!is_letter_char(nick[0]) && !is_special_char(nick[0])) return FALSE;

    while (nick[i] != '\0') {
        if (i > IRC_MAX_NICK_LENGTH) return FALSE;
        if (!is_letter_char(nick[i]) && !is_special_char(nick[i]) && !is_digit_char(nick[i]) && (nick[i] != '-'))
            return FALSE;
        ++i;
    }
    return TRUE;
}

int is_valid_pass(char* pass) {
    int i = 0;

    while (pass[i] != '\0') {
        if (i > IRC_MAX_PASS_LENGTH) return FALSE;
        if ((pass[i] < 0x01 || pass[i] > 0x05) && \
            (pass[i] < 0x07 || pass[i] > 0x08) && \
            (pass[i] < 0x0E || pass[i] > 0x1F) && \
            (pass[i] < 0x21 || pass[i] > 0x7F) && \
            (pass[i] != 0x0C)) return FALSE;
        ++i;
    }
    if (i == 0) return FALSE;
    return TRUE;
}

int is_valid_chname(char* chname) {
    int i = 1;

    if (chname[0] != '#' && chname[0] != '&' && chname[0] != '+') return FALSE;

    while (chname[i] != '\0') {
        if ((chname[i] < 0x01 || chname[i] > 0x07) && \
            (chname[i] < 0x08 || chname[i] > 0x09) && \
            (chname[i] < 0x0B || chname[i] > 0x0C) && \
            (chname[i] < 0x0E || chname[i] > 0x1F) && \
            (chname[i] < 0x21 || chname[i] > 0x2B) && \
            (chname[i] < 0x2D || chname[i] > 0x39) && \
            (chname[i] < 0x3B || chname[i] > 0xFF)) return FALSE;
        ++i;
    }
    if (i == 1) return FALSE;
    return TRUE;
}

int is_empty_channel(channel* ch) {
    if (ch->users_llist == NULL && ch->operators_llist == NULL) return TRUE;
    return FALSE;
}

char* user_mode_string(char modes) {
    char* str = NULL;
    int i = 1;
    if (!(str = (char*) malloc(1 + US_MODE_NUMBER + 1))) return NULL;
    str[0] = '+';
    if (user_mode_a(modes)) {
        str[i] = 'a';
        ++i;
    }
    if (user_mode_i(modes)) {
        str[i] = 'i';
        ++i;
    }
    if (user_mode_w(modes)) {
        str[i] = 'w';
        ++i;
    }
    if (user_mode_r(modes)) {
        str[i] = 'r';
        ++i;
    }
    if (user_mode_o(modes)) {
        str[i] = 'o';
        ++i;
    }
    if (user_mode_O(modes)) {
        str[i] = 'O';
        ++i;
    }
    if (user_mode_s(modes)) {
        str[i] = 's';
        ++i;
    }
    str[i] = '\0';
    return str;
}

char* user_mode_change_string(char modes, char op) {
    char* str = NULL;
    int i = 1;
    if (!(str = (char*) malloc(1 + US_MODE_NUMBER + 1))) return NULL;
    str[0] = op;
    if (user_mode_a(modes)) {
        str[i] = 'a';
        ++i;
    }
    if (user_mode_i(modes)) {
        str[i] = 'i';
        ++i;
    }
    if (user_mode_w(modes)) {
        str[i] = 'w';
        ++i;
    }
    if (user_mode_r(modes)) {
        str[i] = 'r';
        ++i;
    }
    if (user_mode_o(modes)) {
        str[i] = 'o';
        ++i;
    }
    if (user_mode_O(modes)) {
        str[i] = 'O';
        ++i;
    }
    if (user_mode_s(modes)) {
        str[i] = 's';
        ++i;
    }
    str[i] = '\0';
    return str;
}

char user_mode_from_str(char* str, char* unk, char* oper) {
    char mode = 0;
    int i = 0;
    *unk=0;
    if (str[0] == '-') {
        *oper = '-';
        ++i;
    } else {
        if (str[0] == '+') ++i;
        *oper = '+';
    }
    while (str[i] != '\0') {
        if (str[i] == 'a') mode = (mode | US_MODE_a);
        else if (str[i] == 'i') mode = (mode | US_MODE_i);
        else if (str[i] == 'w') mode = (mode | US_MODE_w);
        else if (str[i] == 'r') mode = (mode | US_MODE_r);
        else if (str[i] == 'o') mode = (mode | US_MODE_o);
        else if (str[i] == 'O') mode = (mode | US_MODE_O);
        else if (str[i] == 's') mode = (mode | US_MODE_s);
        else *unk = 1;
        ++i;
    }
    return mode;
}

char* chan_mode_string(unsigned int modes) {
    char* str = NULL;
    int i = 1;
    if (!(str = (char*) malloc(1 + CH_MODE_NUMBER + 1))) return NULL;
    str[0] = '+';
    if (chan_mode_i(modes)) {
        str[i] = 'i';
        ++i;
    }
    if (chan_mode_n(modes)) {
        str[i] = 'n';
        ++i;
    }
    if (chan_mode_t(modes)) {
        str[i] = 't';
        ++i;
    }
    if (chan_mode_k(modes)) {
        str[i] = 'k';
        ++i;
    }
    if (chan_mode_l(modes)) {
        str[i] = 'l';
        ++i;
    }
    str[i] = '\0';
    return str;
}

unsigned int chan_mode_from_str(char* str, char* unk, char* oper) {
    unsigned int mode = 0;
    int i = 0;
    *unk=0;
    if (str[0] == '-') {
        *oper = '-';
        ++i;
    } else {
        if (str[0] == '+') ++i;
        *oper = '+';
    }
    while (str[i] != '\0') {
        if (str[i] == 'i') mode = (mode | CH_MODE_i);
        else if (str[i] == 'n') mode = (mode | CH_MODE_n);
        else if (str[i] == 't') mode = (mode | CH_MODE_t);
        else if (str[i] == 'k') mode = (mode | CH_MODE_k);
        else if (str[i] == 'l') mode = (mode | CH_MODE_l);
        else *unk = 1;
        ++i;
    }
    return mode;
}

/*
 * hash tables manipulation functions
 */
void user_hasht_add(user *item) {
    HASH_ADD_STR(server_data.users_hasht, nick, item);
}

void user_hasht_remove(user *item) {
    HASH_DEL(server_data.users_hasht, item);
}

user* user_hasht_find(char key[IRC_MAX_NICK_LENGTH + 1]) {
    user* found = NULL;
    HASH_FIND_STR(server_data.users_hasht, key, found);
    return found;
}

void channel_hasht_add(channel *item) {
    HASH_ADD_KEYPTR(hh, server_data.channels_hasht, item->name, strlen(item->name), item);
}

void channel_hasht_remove(channel *item) {
    HASH_DEL(server_data.channels_hasht, item);
}

channel* channel_hasht_find(char *key) {
    channel* found = NULL;
    HASH_FIND_STR(server_data.channels_hasht, key, found);
    return found;
}

/*
 * linked lists manipulation functions
 */
int add_nick_to_llist(char nick[IRC_MAX_NICK_LENGTH + 1], active_nicks** llist) {
    active_nicks* new = NULL;
    if (!(new = (active_nicks*) malloc(sizeof (active_nicks)))) return ERROR;
    if (!memcpy(new->nick, nick, IRC_MAX_NICK_LENGTH + 1)) {
        free(new);
        return ERROR;
    }
    new->next = NULL;
    LL_PREPEND(*llist, new);
    return OK;
}

/*
 * OK on success
 * ERROR if there was not such item
 */
int remove_nick_from_llist(char nick[IRC_MAX_NICK_LENGTH + 1], active_nicks** llist) {
    active_nicks *elem = NULL, *tmp;

    LL_FOREACH_SAFE(*llist, elem, tmp) {
        if (!strncmp(nick, elem->nick, strlen(nick))) {
            LL_DELETE(*llist, elem);
            free(elem);
            return OK;
        }
    }
    return ERROR;
}

active_nicks* find_nick_in_llist(char nick [IRC_MAX_NICK_LENGTH + 1], active_nicks** llist) {
    active_nicks *elem = NULL;
    LL_FOREACH(*llist, elem) {
        if (!strncmp(nick, elem->nick, strlen(nick))) {
            return elem;
        }
    }
    return NULL;
}

int add_chname_to_llist(char *ch_name, channel_lst** llist) {
    channel_lst *new = NULL;
    if (!(new = (channel_lst*) malloc(sizeof (channel_lst)))) return ERROR;
    new->ch_name = NULL;
    if (!(new->ch_name = strdup(ch_name))) {
        free(new);
        return ERROR;
    }
    new->next = NULL;
    LL_PREPEND(*llist, new);
    return OK;
}

int remove_chname_from_llist(char *ch_name, channel_lst** llist) {
    channel_lst *elem = NULL, *tmp;

    LL_FOREACH_SAFE(*llist, elem, tmp) {
        if (!strncasecmp(ch_name, elem->ch_name, strlen(ch_name))) {
            LL_DELETE(*llist, elem);
            free(elem->ch_name);
            free(elem);
            return OK;
        }
    }
    return ERROR;
}

channel_lst* find_chname_in_llist(char *ch_name, channel_lst** llist) {
    channel_lst *elem = NULL;

    LL_FOREACH(*llist, elem) {
        if (!strncasecmp(ch_name, elem->ch_name, strlen(ch_name))) {
            return elem;
        }
    }
    return NULL;
}