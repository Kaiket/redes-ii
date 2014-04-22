#ifndef __SEMAPHORES_H
#define __SEMAPHORES_H

/**
 * @file G-2301-03-P1-semaphores.h
 * @details Set of functions for semaphores management.
 * @author Enrique Cabrerizo Fernandez and Guillermo Ruiz Alvarez
.*/

/**
 * @brief It creates a new semaphore initialized with value 1.
 * @details
 * @return The identifier of the created semaphore. If it fails -1 is returned and the error is logged in the system log.
 */
int semaphore_new();


 /**
  * @brief Removes a semaphore.
  * @details All processed waiting for doing a down operation will be awoken.
  * 
  * @param sem_id the identifier of the semaphore returned by semaphores_new.
  * @return OK on success. ERR if arguments are invalid (non-existing semaphore with sem_id) 
  */
int semaphore_rm(int sem_id);


 /**
  * @brief Protects a critical section for being modified when reading.
  * @details 
  * If the critical section is being modified or there is any process/thread
  *	waiting to modify it, then the calling process/thread will wait for
  *	the critical section to be freed, therefore, the writers have the
  *	preference on such critical section.
  *	This function must be called before the reading critical section starts.
  * 
  * @param readers_num must be a global pointer to the number of readers, initialized to 0.
  *	@param readers a global semaphore initialized to 1.
  *	@param writer a global semaphore initialized to 1.
  *	@param mutex_access a global semaphore initialized to 1.
  *	@param mutex_rvariables a global semaphore initialized to 1.
  * @return OK on success.
  * ERR if arguments are invalid.
  */
int semaphore_br(int* readers_num, int readers, int writer, int mutex_access, int mutex_rvariables);


/**
 * @brief Deprotects a critical section for being modified when reading.
 * @details 
 * It means the calling process/thread has called semaphore_br before and 
 * has just left it. If this function is not called after leaving a 
 * critical section, then the rest of processes/thread could be blocked before
 * accessing it.
 * 
 * @param readers_num must be a global pointer to the number of readers, initialized to 0.
 * @param writer semaphore initialized to 1.
 * @param mutex_rvariables a global semaphore initialized to 1.
 * @return OK on success.
 * ERR if arguments are invalid.
 */
 int semaphore_ar(int* readers_num, int writer, int mutex_rvariables);


 /**
  * @brief Protects a critical section for being accessed when writing.
  * @details 
  *	If the critical section is being read then the calling process/thread will wait for
  *	the critical section to be freed. The calling process/thread have prefefence over the 
  *	rest, this means that if after the call any other process/thread try to access the critical
  *	section, it will wait for the calling process to finish its tasks.
  *	This function must be called before the writing critical section starts.
  * 
  * @param writer a global semaphore initialized to 1.
  * @param readers a global semaphore initialized to 1.
  * 
  * @return OK on success.
  * ERR if arguments are invalid.
  */
 int semaphore_bw(int writer, int readers);


 /**
  * @brief Deprotects a critical section for being accessed when writing.
  * @details It means the calling process/thread has called semaphore_bw before and 
  *	has just left it. If this function is not called after leaving a 
  *	critical section, then the rest of processes/thread could be blocked before
  *	accessing it.
  * 
  * @param writer a global semaphore initialized to 1.
  * @param readers a global semaphore initialized to 1.
  * 
  * @return OK on success.
  * ERR if arguments are invalid.
  */
 int semaphore_aw(int writer, int readers);



#endif