#ifndef __CLIENT_UTILITY_FUNCTIONS_H
#define __CLIENT_UTILITY_FUNCTIONS_H

#include "G-1301-03-P1-ircserver.h"


void *client_thread_listener(void *arg);

int client_receive_data_management(char *data);

void client_print_full_mesage(char *target_array[MAX_CMD_ARGS + 2], int prefix, int n_strings);

void client_cmd_parsing(char *string, int type);

int client_connect_to_server(char* server_url, int port, void* (*thread_routine) (void *arg));

int client_new_session(char *nick, char *name, char *real_name, char *server_url);

int client_send_irc_command(char *command, char *parameters);

#endif