#include <pthread.h>

#define NOT_REFRESHED -1 /*value of array_first_free after the free position it announced has been occupied*/
#define FULL_ARRAY -2 /*if the current thread_array is full this will be the value returned by seek_array_first_free*/

/**
 * @file G-1301-03-P1-thread_handling.h
 * @details Set of functions for thread handling.
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/

typedef struct {
    pthread_t thread_id;
    int socket;
    char active;
} Thread_handler;

/**
 * @brief [brief description]
 * @details [long description]
 * 
 * @param client_sock [description]
 * @param thread_routine [description]
 * 
 * @return [description]
 */
int launch_thread(int client_sock, void* (*thread_routine) (void *arg));

/**
 * @brief [brief description]
 * @details [long description]
 * @return [description]
 */
int nbjoin_threads (void);

/**
 * @brief [brief description]
 * @details [long description]
 * @return [description]
 */
int seek_array_first_free (void);