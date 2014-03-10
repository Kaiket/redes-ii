#ifndef __SEMAPHORES_H
#define __SEMAPHORES_H

/*
 * Function: semaphore_new
 * Description:
 * 		It creates a new semaphore initialized with value 1.
 * Parameters:
 * 		none
 * Return value:
 *		The identifier of the created semaphore.
 *		If it fails -1 is returned and the error is logged in the system log.
 */
int semaphore_new();

/*
 * Function: semaphore_rm
 * Description:
 *		It removes the semaphore with "sem_id" identifier.
 *		All processed waiting for doing a down operation will be awoken.
 * Parameters:
 *		sem_id: the identifier of the semaphore returned by semaphores_new.
 * Return value:
 *		OK on success.
 *		ERR if arguments are invalid (non-existing semaphore with sem_id) 
 */
int semaphore_rm(int sem_id);


/*
 * Function: semaphore_down
 * Description:
 *		If the semaphore with id sem_id has a value grater than zero, then its value is decremented.
 *		Otherwise, the process/thread blocks waiting for the value of semaphore to be possitive.
 * Parameters:
 *		sem_id: the identifier of the semaphore returned by semaphores_new.
 * Return value:
 *		OK on success.
 *		ERR if arguments are invalid (non-existing semaphore with sem_id) 
 */
 int semaphore_down(int sem_id);

 /*
 * Function: semaphore_up
 * Description:
 *		The value of the semaphore with id sem_id is incremented by 1.
 * Parameters:
 *		sem_id: the identifier of the semaphore returned by semaphores_new.
 * Return value:
 *		OK on success.
 *		ERR if arguments are invalid (non-existing semaphore with sem_id) 
 */
 int semaphore_up(int sem_id);


/*
 * Function: semaphore_br
 * Description:
 *		Protects a critical section for being modified when reading.
 *		If the critical section is being modified or there is any process/thread
 *		waiting to modify it, then the calling process/thread will wait for
 *		the critical section to be freed, therefore, the writers have the
 *		preference on such critical section.
 *		This function must be called before the reading critical section starts.
 * Parameters:
 *		readers_num: must be a global pointer to the number of readers, initialized to 0.
 *		readers: a global semaphore initialized to 1.
 *		writer: a global semaphore initialized to 1.
 *		mutex_access: a global semaphore initialized to 1.
 *		mutex_rvariables: a global semaphore initialized to 1.
 * Return value:
 *		OK on success.
 *		ERR if arguments are invalid.
 */
int semaphore_br(int* readers_num, int readers, int writer, int mutex_access, int mutex_rvariables);


/*
 * Function: semaphore_ar
 * Description:
 *		Deprotects a critical section for being modified when reading.
 *		It means the calling process/thread has called semaphore_br before and 
 *		has just left it. If this function is not called after leaving a 
 *		critical section, then the rest of processes/thread could be blocked before
 *		accessing it.
 * Parameters:
 *		readers_num: must be a global pointer to the number of readers, initialized to 0.
 *		writer: a global semaphore initialized to 1.
 *		mutex_rvariables: a global semaphore initialized to 1.
 * Return value:
 *		OK on success.
 *		ERR if arguments are invalid.
 */
 int semaphore_ar(int* readers_num, int writer, int mutex_rvariables);

 /*
 * Function: semaphore_bw
 * Description:
 *		Protects a critical section for being accessed when writing.
 *		If the critical section is being read then the calling process/thread will wait for
 *		the critical section to be freed. The calling process/thread have prefefence over the 
 *		rest, this means that if after the call any other process/thread try to access the critical
 *		section, it will wait for the calling process to finish its tasks.
 *		This function must be called before the writing critical section starts.
 * Parameters:
 *		writer:. a global semaphore initialized to 1.
 *		readers: a global semaphore initialized to 1.
 * Return value:
 *		OK on success.
 *		ERR if arguments are invalid.
 */
 int semaphore_bw(int writer, int readers);


 /*
 * Function: semaphore_aw
 * Description:
 *		Deprotects a critical section for being accessed when writing.
 *		It means the calling process/thread has called semaphore_bw before and 
 *		has just left it. If this function is not called after leaving a 
 *		critical section, then the rest of processes/thread could be blocked before
 *		accessing it.
 * Parameters:
 *		writer:. a global semaphore initialized to 1.
 *		readers: a global semaphore initialized to 1.
 * Return value:
 *		OK on success.
 *		ERR if arguments are invalid.
 */
 int semaphore_aw(int writer, int readers);



#endif