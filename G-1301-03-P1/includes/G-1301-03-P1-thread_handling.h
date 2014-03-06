#include <pthread.h>

#define NOT_REFRESHED -1 /*value of array_first_free after the free position it announced has been occupied*/
#define FULL_ARRAY -2 /*if the current thread_array is full this will be the value returned by seek_array_first_free*/

typedef struct {
    pthread_t thread_id;
    int socket;
    char active;
} Thread_handler;
