
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <ucontext.h>
#include "queue.h"
#include "multiLevelQueue.h"
#include "my_pthread_t.h"
#include "common.h"
#include "scheduler.h"
#include "context.h"
#include "threadStructure.h"

void my_pthread_create(my_pthread_t* thread, pthread_attr_t* attr,void *(*function)(void*),void* arg)
{
	populateThread(thread, makeContext(function, arg));
	
	scheduleForExecution(thread);
}

void my_pthread_yield()
{
	yield();
}

int my_pthread_exit(void *value_ptr)
{
	abruptEnding(value_ptr);
}

int my_pthread_join(my_pthread_t thread, void ** value_ptr)
{

	printf("the join has started%d", thread.st);
	//printf("%d\n", getStatus(&thread));
	//target thread
	
	while(thread.st != 4)
	{
			//printf("calling thread  is waiting for inner to execute%d\n", getStatus(&thread));
	//	statusChange(WAITING);

	//	yield();
		printf("%d\n", thread.st);
	}
	//
	//value_ptr = &(thread.retval);
	printf("Join ended");
	return 0;
}

int my_pthread_mutex_init(my_pthread_mutex_t* mutex,const my_pthread_mutexattr_t* mutex_attr)
{
	mutex = (my_pthread_mutex_t*)calloc(1, sizeof(my_pthread_mutex_t));
	mutex->mutex = 0; //initialized
	mutex->mutexattr_t = mutex_attr; //making mutex attributes to be the attributes passed
	return 1;
}

int my_pthread_mutex_lock(my_pthread_mutex_t* mutex)
{

	int mutexLocked = 1;	
	while(mutexLocked == 1)
	{
		mutexLocked = __sync_lock_test_and_set(&(mutex->mutex),1);
		if(mutexLocked == -1)
		{
			//mutex is not initialized
			return -1;
		}
		 if(mutexLocked==0)
		{
			break; //mutex received	
		}
		
		yield();
	}
	return 1;
}

int my_pthread_mutex_unlock(my_pthread_mutex_t * mutex)
{
	__sync_lock_test_and_set(&(mutex->mutex),0);
}

int my_pthread_mutex_destroy(my_pthread_mutex_t * mutex)
{
	int mutexFree = -1;
	mutexFree = __sync_lock_test_and_set(&(mutex->mutex),0);
	if(mutexFree==-1)
	{
		//mutex cannot be freed as it is being used
		return -1;
	}
	else {
		free(&mutex);
	}
}

