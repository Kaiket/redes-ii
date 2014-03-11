#ifndef __IRC_UTIL_H
#define __IRC_UTIL_H

#include "G-1301-03-P1-ircserver.h"

int user_registered_flag(char flags);
int is_letter_char(char c);
int is_special_char(char c);
int is_digit_char(char c);
int is_valid_nick(char* nick);
int is_valid_chname(char* chname);

void user_hasht_add(user *item);
void user_hasht_remove(user *item);
user* user_hasht_find(char key[IRC_MAX_NICK_LENGTH + 1]);
void channel_hasht_add(channel *item);
void channel_hasht_remove(channel *item);
channel* channel_hasht_find(char *key);

int add_nick_to_llist(char nick[IRC_MAX_NICK_LENGTH + 1], active_nicks** llist);
int remove_nick_from_llist(char nick[IRC_MAX_NICK_LENGTH + 1], active_nicks** llist);
active_nicks* find_nick_in_llist(char nick [IRC_MAX_NICK_LENGTH + 1], active_nicks** llist);
int add_chname_to_llist(char *name, channel_lst** llist);
int remove_chname_from_llist(char *name, channel_lst** llist);
channel_lst* find_chname_in_llist(char *name, channel_lst** llist);

#endif
