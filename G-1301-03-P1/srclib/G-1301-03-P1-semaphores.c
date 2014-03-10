#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include "G-1301-03-P1-types.h"
#include "G-1301-03-P1-semaphores.h"


/*
 * Function: semaphore_new
 * Inplementation comments: 
 *		none
 */
int semaphore_new(){
	
	int semid;

	if ((semid = semget(IPC_PRIVATE, 1, S_IRWXU)) == -1) {
		syslog(LOG_ERR, "Failed while creating a new semaphore: %s", strerror(errno));
		return ERROR;
	}

	if (semctl(semid, 0, SETVAL, (int) 1) == -1){
		syslog(LOG_ERR, "Failed while creating a new semaphore: %s", strerror(errno));
		return ERROR;
	}

	return semid;
}

/*
 * Function: semaphore_rm
 * Implementation comments
 *		none
 */
int semaphore_rm(int sem_id){

	return semctl(sem_id, 0, IPC_RMID) != -1? OK : ERROR;

}


 /*
 * Function: semaphore_down
 * Implementation comments
 *		none
 */
 int semaphore_down(int sem_id){

 	struct sembuf sops;
 	sops.sem_num = 0;
 	sops.sem_op = -1;
 	sops.sem_flg = SEM_UNDO;

 	return semop(sem_id, &sops, 1);

 }

  /*
 * Function: semaphore_up
 * Implementation comments
 *		none
 */
 int semaphore_up(int sem_id){

 	struct sembuf sops;
 	sops.sem_num = 0;
 	sops.sem_op = 1;
 	sops.sem_flg = 0;

 	return semop(sem_id, &sops, 1);

 }


 /*
 * Function: semaphore_br
 * Implementation comments
 *		none
 */
 int semaphore_br(int* readers_num, int readers, int writer, int mutex_access, int mutex_rvariables){

 	if (semaphore_down(mutex_access) == ERROR) return ERROR;
 		if (semaphore_down(readers) == ERROR) return ERROR;
 			if (semaphore_down(mutex_rvariables) == ERROR) return ERROR;

 				++(*readers_num);
 				if ((*readers_num) == 1){
	 				if (semaphore_down(writer) == ERROR) return ERROR;
 				}

			if (semaphore_up(mutex_rvariables) == ERROR) return ERROR;
 		if (semaphore_up(readers) == ERROR) return ERROR;
 	if (semaphore_up(mutex_access) == ERROR) return ERROR;

 	return OK;

 }


/*
 * Function: semaphore_ar
 * Implementation comments
 *		none
 */
 int semaphore_ar(int* readers_num, int writer, int mutex_rvariables){

 	if (semaphore_down(mutex_rvariables) == ERROR) return ERROR;
 		--(*readers_num);
 		if((*readers_num) == 0){
 			if (semaphore_up(writer) == ERROR) return ERROR;
 		}
 	if (semaphore_up(mutex_rvariables) == ERROR) return ERROR;

 	return OK;

 }

 /*
 * Function: semaphore_bw
 * Implementation comments
 *		none
 */
 int semaphore_bw(int writer, int readers){

 	if (semaphore_down(readers) == ERROR) return ERROR;
 		if (semaphore_down(writer)  == ERROR) return ERROR;

 	return OK;

 }


 /*
 * Function: semaphore_aw
 * Implementation comments
 *		none
 */
 int semaphore_aw(int writer, int readers){

 		if (semaphore_up(writer) == ERROR) return ERROR;
 	if (semaphore_up(readers) == ERROR) return ERROR;

 	return OK;

 }