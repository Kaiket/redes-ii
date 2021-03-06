#ifndef __IRC_UTIL_H
#define __IRC_UTIL_H

#include "G-2301-03-P1-ircserver.h"

/*
 * These functions return the  user mode.
 */
int user_mode_a(char flags);
int user_mode_i(char flags);
int user_mode_w(char flags);
int user_mode_r(char flags);
int user_mode_o(char flags);
int user_mode_O(char flags);
int user_mode_s(char flags);
int user_registered(char flags);

/*
 * These functions return the channel mode.
 */
int chan_mode_O(unsigned int flags);
int chan_mode_o(unsigned int flags);
int chan_mode_v(unsigned int flags);
int chan_mode_a(unsigned int flags);
int chan_mode_i(unsigned int flags);
int chan_mode_m(unsigned int flags);
int chan_mode_n(unsigned int flags);
int chan_mode_q(unsigned int flags);
int chan_mode_p(unsigned int flags);
int chan_mode_s(unsigned int flags);
int chan_mode_r(unsigned int flags);
int chan_mode_t(unsigned int flags);
int chan_mode_k(unsigned int flags);
int chan_mode_l(unsigned int flags);
int chan_mode_b(unsigned int flags);
int chan_mode_e(unsigned int flags);
int chan_mode_I(unsigned int flags);

/*
 * Auxiliar functions for getting information.
 */
int is_letter_char(char c);
int is_special_char(char c);
int is_digit_char(char c);
int is_valid_nick(char* nick);
int is_valid_chname(char* chname);
int is_valid_pass(char* pass);
int is_empty_channel(channel* ch);
char* user_mode_string (char modes);
char* user_mode_change_string(char modes, char op);
char user_mode_from_str (char* str, char* unk, char* oper);
char* chan_mode_string (unsigned int modes);
unsigned int chan_mode_from_str(char* str, char* unk, char* oper);


/*
 * Functions for managing hash tables
 */
void user_hasht_add(user *item);
void user_hasht_remove(user *item);
user* user_hasht_find(char key[IRC_MAX_NICK_LENGTH + 1]);
void channel_hasht_add(channel *item);
void channel_hasht_remove(channel *item);
channel* channel_hasht_find(char *key);

/*
 * Functions for managing lists
 */
int add_nick_to_llist(char nick[IRC_MAX_NICK_LENGTH + 1], active_nicks** llist);
int remove_nick_from_llist(char nick[IRC_MAX_NICK_LENGTH + 1], active_nicks** llist);
active_nicks* find_nick_in_llist(char nick [IRC_MAX_NICK_LENGTH + 1], active_nicks** llist);
int add_chname_to_llist(char *name, channel_lst** llist);
int remove_chname_from_llist(char *name, channel_lst** llist);
channel_lst* find_chname_in_llist(char *name, channel_lst** llist);

#endif
