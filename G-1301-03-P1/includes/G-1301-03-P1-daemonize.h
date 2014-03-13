#ifndef __DAEMONIZER_H
#define __DAEMONIZER_H

/**
 * @file   G-1301-03-P1-daemonice.c¡h
 * @author Enrique Cabrerizo Fernandez, Guillermo Ruiz Alvarez
 * @date   Feb, 2013
.*/

/**
 * @brief runs in background
 * 
 * @details Opens a connection with the system logger. The 
 * string  pointed to by ident is prepended to every message.
 * After that, runs the calling process in background mode.
 * Signals SIGCHLD and SIGPWR will kill the process.
 * 
 * To test the function the program G-1301-03-P1-daemonize_test has been made.
 * This program runs a process in background mode and pauses itself waiting for
 * a signal to kill it.
 * It shows the PID and PPID of the process. If the tty is closed after executing this
 * program, the process won't stop until a signal (SIFPWR or SIGCHLD) is delivered. 
 * Any of this facts will be registered in the log system.
 * 
 * @param ident name of the log identity
 * 
 * @return 0 on success. -1 on failure, and it writes the description 
 * of the error in the log file closing the connection after it.
 */
int daemonize(const char *ident);

#endif