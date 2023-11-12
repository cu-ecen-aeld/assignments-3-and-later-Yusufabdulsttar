/** *********************************
* Author by : Yusuf Abdulsttar
* **********************************/

#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

 // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
	DEBUG_LOG("start the thread");
	
	// wait to obtain
	if ( usleep(1000*(thread_func_args->wait_to_obtain_ms)) != 0){
		DEBUG_LOG("faild to wait");
		thread_func_args->thread_complete_success = false;
	}
	// obtain mutex
	else if ( pthread_mutex_lock(thread_func_args->mutex) != 0 ){
		DEBUG_LOG("faild to lock the mutex");
		thread_func_args->thread_complete_success = false;
	}
	else {
		// wait to release
		if ( usleep(1000*(thread_func_args->wait_to_release_ms)) != 0){
			DEBUG_LOG("faild to wait");
			thread_func_args->thread_complete_success = false;
		}	
		// release mutex
		else if ( pthread_mutex_unlock(thread_func_args->mutex) != 0 ){
			DEBUG_LOG("faild to unlock the mutex");
			thread_func_args->thread_complete_success = false;
		}
		else {
			thread_func_args->thread_complete_success = true;
		}
		
	}
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
	 struct thread_data *thread_1 = (struct thread_data *) malloc(sizeof(struct thread_data));
	 
	 if (thread_1 == NULL) {
	 	DEBUG_LOG("failed to allocate memory");
	 }
 	 else {
	 	//initialization
		thread_1->wait_to_obtain_ms = wait_to_obtain_ms;
		thread_1->wait_to_release_ms = wait_to_release_ms;
		thread_1->mutex = mutex;
		if ( pthread_create(thread, NULL, threadfunc, thread_1) == 0 ){
			return true;
		}
	 }	
     return false;
}

