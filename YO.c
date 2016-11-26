#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "memManager.h"



char storage[1024*1024*8];

void *myallocate(int size, int a, int b, int c)
{
	printf("YO!");
	return NULL;
}

void mydeallocate(void *ptr, int a, int b, int c)
{
	
}
void main()
{

}