#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-semaphores.h"
#include "G-1301-03-P1-thread_handling.h"

#define THREAD_MSG_LEN 64
#define THREAD_NUMBER 10

int number = 1;
char string[THREAD_MSG_LEN];
int readers_num = 0; 
int readers, writer, mutex_access, mutex_rvariables;

/*Thread routine*/
void *thread_routine(void *thread){

	char strnumber[THREAD_MSG_LEN];
	
	semaphore_br(&readers_num, readers, writer, mutex_access, mutex_rvariables);
	printf("I (%d) am reading the variable number, and it says: %d\n", (int) pthread_self(), number);
	printf("I (%d) am reading the variable string, and it says: %s\n", (int) pthread_self(), string);
	semaphore_ar(&readers_num, writer, mutex_rvariables);

	semaphore_bw(writer, readers);
	printf("I (%d) am going to concatenate the string with the number\n", (int) pthread_self());
	sprintf(strnumber, "%d ", number);
	strcat(string, strnumber);
	printf("Now string is: %s\n", string);
	printf("I (%d) am going to modify the variable number from %d to %d\n", (int) pthread_self(), number, number+1);
	++number;
	semaphore_aw(writer, readers);

	pthread_exit(NULL);

}

int main(int argc, char const *argv[]) {
	
	int i;
	pthread_t thread[THREAD_NUMBER];
	strcpy(string, "Numbers: ");

	/*Semaphores*/
	if ((readers = semaphore_new()) == ERROR){
		printf("Failed while creating a new semaphore: %s\n", strerror(errno));
			return EXIT_FAILURE;
	}

	if ((writer = semaphore_new()) == ERROR){
		printf("Failed while creating a new semaphore: %s\n", strerror(errno));
			return EXIT_FAILURE;
	}

	if ((mutex_access = semaphore_new()) == ERROR){
		printf("Failed while creating a new semaphore: %s\n", strerror(errno));
			return EXIT_FAILURE;
	}

	if ((mutex_rvariables = semaphore_new()) == ERROR){
		printf("Failed while creating a new semaphore: %s\n", strerror(errno));
			return EXIT_FAILURE;
	}

	/*Threads*/
	for (i = 0; i < THREAD_NUMBER; ++i)	{
		
		if (pthread_create(&(thread[i]), NULL, thread_routine, &(thread[i])) != 0){
			printf("Failed while creating a new thread: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
	}

	/*Join of threads*/
	for (i = 0; i < THREAD_NUMBER; ++i){
		pthread_join(thread[i], NULL);
	}

	/*Removing semaphores*/
	semaphore_rm(readers);
	semaphore_rm(writer);
	semaphore_rm(mutex_access);
	semaphore_rm(mutex_rvariables);

	/*Exiting*/
	printf("Finished\n");

	return EXIT_SUCCESS;
}