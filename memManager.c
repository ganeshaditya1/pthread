#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "memManager.h"



char storage[1024*1024*8];

typedef struct{
	int pid;
	void *start, *top;
	header* next;
}header;
header *startNode = NULL;

void *myallocate(int size, int a, int b, int c)
{
	header *start = (header*)storage;
	start->pid = 0;
	start->top = start + sizeof(header);
	((char*)start->top)[0] = 'q';
	return start->top;
}

void mydeallocate(void *ptr, int a, int b, int c)
{
	
}