#ifndef __CLIENT_UTILITY_FUNCTIONS_H
#define __CLIENT_UTILITY_FUNCTIONS_H


void *client_thread_listener(void *arg);

int client_connect_to_server(char* server_url, int port, void* (*thread_routine) (void *arg));

int client_new_session(char *nick, char *name, char *real_name, char *server_url);

int client_send_irc_command(char *command, char *parameters);

#endif