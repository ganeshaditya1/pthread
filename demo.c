#include "my_pthread_t.h"
#include "timer.h"
#include <ucontext.h>
#include "scheduler.h"
#include "my_malloc.h"
#define STACKSIZE 64000

void* func(void * value) 
{
	int v = (int)value;
	printf("Value Recieved2: %d\n", v);
	//my_pthread_yield();
	printf("%d getCurrentTid\n", getCurrentTid());
	char *a = malloc(400);
	a[2] = 'c';
	while(1){
		printf("%c THREAD: %d", a[2], getCurrentTid());
	}	
}

void* func2(void * value) 
{
	printf("%d getCurrentTid\n", getCurrentTid());
	char *a = malloc(400);
	a[2] = 'd';
	while(1){
		printf("%c THREAD: %d", a[2], getCurrentTid());
	}	
}


void* func3(void * value) 
{
	printf("%d getCurrentTid\n", getCurrentTid());
	char *a = malloc(400);
	free(a);
	a = malloc(400);
	a[2] = 'd';
	while(1){
		printf("%c THREAD: %d", a[2], getCurrentTid());
	}	
}


int main()
{	
	printf("Main initialized\n");
	my_pthread_t t1, t2, t3, t4, t5,t6,t7;
	my_pthread_create(&t1,NULL, &func, (void*)7);
	my_pthread_create(&t2,NULL, &func2, NULL);
	//my_pthread_create(&t3,NULL, &func3, NULL);

	while(1)
	{

		//printf("Main Thread%d\n", getCurrentTid());
		//printf("Main thread%d\n", getStatus(&t7));
	}
}
