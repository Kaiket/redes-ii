/*
 * Thread handling library for server connections
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-thread_handling.h"

/*
 * Global variables
 */

Thread_handler *thread_array = NULL;

long unsigned int n_threads = 0;
long unsigned int already_allocated = 0;
long unsigned int array_first_free = 0;
long threads_per_page = 0;



/*
 * Functions
 */
/*
 * Function: launch_thread
 * Implementation comments:
 *      none.
 */
int launch_thread(int client_sock, void* (*thread_routine) (void *arg)) {
    pthread_t thread;

    Thread_handler* t_aux = NULL;
    syslog(LOG_NOTICE, "Creating a thread to handle connection in socket %d", client_sock);

    threads_per_page = (sysconf(_SC_PAGE_SIZE) / sizeof (Thread_handler));
    
    if ((n_threads % threads_per_page == 0)) {
        if (n_threads == already_allocated) {
            t_aux = (Thread_handler*) realloc(thread_array, (already_allocated + threads_per_page) * sizeof (Thread_handler));
            if (!t_aux) {
                syslog(LOG_ERR, "Could not allocate memory for the new thread (socket %d).", client_sock);
                return ERROR;
            }
            bzero(&t_aux[already_allocated], threads_per_page * sizeof (Thread_handler) ); /*initialize the newly reserved memory*/
            already_allocated+=threads_per_page;
            thread_array=t_aux;
        }
    }
    
    if ((array_first_free == NOT_REFRESHED) || (array_first_free == FULL_ARRAY)) {
        if (seek_array_first_free() == FULL_ARRAY) {
            syslog(LOG_ERR, "Could not create a thread to handle connection in socket %d", client_sock);
        }
    }
    
    thread_array[array_first_free].active=1;
    thread_array[array_first_free].socket=client_sock;
    
    if (pthread_create(&thread, NULL, thread_routine, ((void *) &thread_array[array_first_free])) < 0) {
        syslog(LOG_ERR, "Could not create a thread to handle connection in socket %d", client_sock);
        thread_array[array_first_free].active=0;
        return ERROR;
    }
    
    thread_array[array_first_free].thread_id=thread;
    array_first_free=NOT_REFRESHED;
    n_threads++;
    
    return OK;
}

/*
 * Performs a non-blocking join for every active thread currently in the array.
 * Sets the value of array-first-free global variable to the first non-active thread after the join
 */
int nbjoin_threads (void) {
    int i;
    if (!thread_array) return ERROR;
    for (i=0; i<already_allocated; ++i) {
        if (thread_array[i].active) {
            if (pthread_tryjoin_np(thread_array[i].thread_id, NULL) == 0) { /*thread terminated*/
                syslog(LOG_NOTICE, "Thread %lu in socket %d terminated\n", thread_array[i].thread_id, thread_array[i].socket);
                n_threads--;
                thread_array[i].active=0;
                if ((i<array_first_free) || (array_first_free < 0)) array_first_free=i;
            }
        }
        else {
            if ((i<array_first_free) || (array_first_free < 0)) array_first_free=i;
        }
    }
    return array_first_free;
}

/*
 * Seeks the thread_array for the first non-active thread and sets array_first_free value accordingly.
 * Use not recommended, it is better to perform a nbjoin_threads call and let it set the value.
 */
int seek_array_first_free (void) {
    int i;
    for (i=0; i<already_allocated; ++i) {
        if (!thread_array[i].active) {
            array_first_free=i;
            return array_first_free;
        }
    }
    return FULL_ARRAY;
}
