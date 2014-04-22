#ifndef __CLIENT_COMMANDS_H
#define __CLIENT_COMMANDS_H

#define TIMEOUT 10000

void command_join_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_nick_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_part_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_kick_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_quit_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_privmsg_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_invite_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_ping_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_mode_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_error_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void rpl_who_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void rpl_list_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void rpl_invite_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void rpl_away_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_query_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_me_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_part_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_mode_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_server_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_exit_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

int command_pcall_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

int command_paccept_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

int command_pclose_out(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void command_pcall_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick);

void command_paccept_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick);

void command_pclose_in(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings, char *recv_nick);



#endif
