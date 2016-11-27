#ifndef SCHEDULER
#define SCHEDULER

#include "my_pthread_t.h"

void mySchedulerInit();
void scheduleForExecution(my_pthread_t* thread);
void abruptEnding();
void yield();
int getCurrentTid();

#endif