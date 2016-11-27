#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ucontext.h>

#include "scheduler.h"
#include "common.h"
#include "timer.h"
#include "multiLevelQueue.h"
#include "my_pthread_t.h"
#include "threadStructure.h"
#include "queue.h"
#include "context.h"
#include "my_malloc.h"


boolean initialized = false;
boolean pauseAlarms = false;
queueNode_t* currentlyExecuting = NULL;

int getCurrentTid()
{
	if(currentlyExecuting == NULL)
	{
		// Too bad if you spawned 9999 threads before the main thread got swapped out.
		return 9999;
	}
	else
	{
		return getThread(currentlyExecuting)->tid;
	}
}

void timeSliceExpired ()
{
	//Some sort of a crappy issue with timers. Only the last set one is firing.
	static int counter = 0;
	counter = (++counter)%3;
	if(counter == 2)
	{
		maintainenceCycle();
	}
	//printf("Scheduler ran\n");
	// Ugly hack to ensure we don't get interupted while switching contexts.
	if(pauseAlarms) return;
	// Pick the next node to execute
	queueNode_t* nextNodeToExecute = mDeque();
	// check if the queue was empty.
	if(nextNodeToExecute == NULL)
 	{
 		return;
 	}
 	// Prepare to swap out the current context unless, you are instructed otherwise.
 	boolean newlyCreated = false; 	
 	if(currentlyExecuting == NULL)
	{
		my_pthread_t* currentThread = (my_pthread_t *)malloc(sizeof(my_pthread_t));
		populateThread(currentThread, makeEmptyContext());
		currentlyExecuting = createNode(currentThread);
		newlyCreated = true;
	}
	my_pthread_t* currentThread = getThread(currentlyExecuting);
	if(!newlyCreated)
	{
		decrementPriority(currentThread);
	}
	mEnque(currentlyExecuting);
	currentlyExecuting = nextNodeToExecute;	
	my_pthread_t* nextThread = getThread(nextNodeToExecute);
	
	setStatus(nextThread, RUNNING);
	setStatus(currentThread, READY);
	// Do the swaping.
	protect_pages(currentThread->tid);
	unprotect_pages(nextThread->tid);
	swapcontext(currentThread->context, nextThread->context);
}

void mySchedulerInit()
{	
	setTimer(&timeSliceExpired, 50);
	initMQ();
}

void scheduleForExecution(my_pthread_t* thread)
{
	if(!initialized)
	{
		// Todo: This is wrong. Figure out a thread safe way to prevent the scheduler from getting initialized twice.
		srand(time(0));	
		usleep((rand()%100));
		initialized = true;
		printf("Scheduler initialized\n");
		mySchedulerInit();
		//setcontext(&currentContext);
	}
	// Added a createNode function in queue.c that takes a thread and returns a queue node.
	queueNode_t * qn = createNode(thread);
	mEnque(qn);
}

void loadNewThread(int resheduleFlag)
{
		
	queueNode_t* nextNodeToExecute = mDeque();
	
	// check if the queue was empty.
	if(nextNodeToExecute == NULL)
 	{
 		// Could be that we are at the 4th time slice and the multiLevelQueue wants us to continue executing what
 		// we got. The only way to be sure is to Deque again and see.
 		
 		nextNodeToExecute = mDeque();
 		if(nextNodeToExecute == NULL)
 		{
 			// Can't imagine a situation where we could reach this point. But hell, if we do! It's worth
 			// checking out with a debugger to see what the hell is happening.
 			// Probably some really wierd race condition that we didn't account for.
 			printf("What in the name of everything good is happening??\n");
 			return;
 		}
 	}
 	my_pthread_t* currentThread = getThread(currentlyExecuting);
 	if(resheduleFlag)
 	{
 		setStatus(currentThread, WAITING);
 		mEnque(currentlyExecuting);
 	}
 	else
 	{
 		//printf("%d x\n", currentThread->st);
 		setStatus(currentThread, FINISHED);	
 	}
 	my_pthread_t* nextThread = getThread(nextNodeToExecute);
 	currentlyExecuting = nextNodeToExecute;
 	/*unprotect_pages(nextThread->tid);
 	protect_pages(currentThread->tid);*/
 	swapcontext(currentThread->context, nextThread->context);
}

void abruptEnding(void* output)
{
	loadNewThread(0);
}

void yield()
{
	loadNewThread(1);
}