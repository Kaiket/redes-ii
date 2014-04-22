#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include "G-2301-03-P1-types.h"
#include "G-2301-03-P1-connection.h"
#include "G-2301-03-P1-thread_handling.h"
#include "G-2301-03-P1-ircserver.h"
#include "G-2301-03-P1-daemonize.h"

int main(int argc, char **argv) {

    int socket, client_socket;
    int port = IRC_DEFAULT_PORT;

    /*Parameters processing*/
    if (argc != 1 && argc != 2) {
        printf("%s <port (optional)>\n", argv[0]);
        return (EXIT_FAILURE);
    } else if (argc == 2) {
        port = atoi(argv[1]);
    }

    /*Runs in background*/
    if (daemonize(SERVER_LOG_IDENT) == ERROR) {
        irc_exit_message();
        return (EXIT_FAILURE);
    }

    /*Initializes server*/
    socket = init_server(port, SERVER_MAX_CONNECTIONS);
    if (socket <= 0) {
        irc_exit_message();
        exit(EXIT_FAILURE);
    }

    /*Initializes IRC server data*/
    irc_server_data_init();
    
    /*Accepts connexions and takes charge of them*/
    while (1) {
        client_socket = accept_connections(socket);
        nbjoin_threads();
        if (launch_thread(client_socket, irc_thread_routine) != OK) {
            syslog(LOG_ERR, "Server: Failed while launching a thread: %s", strerror(errno));
        }
    }

    return (EXIT_SUCCESS);
}