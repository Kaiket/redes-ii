#include <pthread.h>

#define NOT_REFRESHED -1 /*value of array_first_free after the free position it announced has been occupied*/
#define FULL_ARRAY -2 /*if the current thread_array is full this will be the value returned by seek_array_first_free*/

/**
 * @file G-2301-03-P1-thread_handling.h
 * @details Set of functions for thread handling.
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/

typedef struct {
    pthread_t thread_id;
    int socket;
    char active;
} Thread_handler;

/**
 * @brief Launch a new thread for socket operation management.
 * @details The thread launched execute the thread_routine.
 * 
 * @param client_sock the socket of the client which is going to be attended.
 * @param thread_routine the routine to execute.
 * 
 * @return On success, it returns OK. On failure, it returns ERROR and log the error.
 */
int launch_thread(int client_sock, void* (*thread_routine) (void *arg));

/**
 * @brief Performs a non-blocking join for every active thread currently in the array.
 * @details 
 * Sets the value of array-first-free global variable to the first non-active thread after the join
 * @return the fisrt available slot on the array.
 */
int nbjoin_threads (void);

/**
 * @brief Seeks the thread_array for the first non-active thread and sets array_first_free value accordingly.
 * @details 
 * Use not recommended, it is better to perform a nbjoin_threads call and let it set the value.
 * @return The first available slot on the array or FULL_ARRAY if it is full.
 */
int seek_array_first_free (void);