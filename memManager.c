#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include "memManager.h"

#define BUFFER_SIZE 8388608



typedef struct pgHead{
	int pid;
	void *start;
	struct pgHead* next;
}pageHeader;
pageHeader *startNode = NULL, *tailNode = NULL;

int initializied = 0;
char storage[BUFFER_SIZE];
int pageSize, totalPagesAllocated = 0, totalCapacity;

void makeNode(pageHeader* node, int pid)
{
	static char a = 's';
	node->pid = pid;
	node->start = ((char*)node) + sizeof(pageHeader);
	((char*)(node->start))[0] = a++;
	//node->top = node->start;
	node->next = NULL;
	totalPagesAllocated++;
}

int canAccomodate(pageHeader* target, int size)
{
	return 1;
}

pageHeader* getPage(int pid, int size)
{
	pageHeader* iterator = startNode;
	while(iterator != NULL)
	{
		if(iterator->pid == pid && canAccomodate(iterator, size))
		{
			return iterator;
		}
		iterator = iterator->next;
	}
	if(startNode == NULL)
	{
		startNode = storage;
		makeNode(startNode, pid);
		tailNode = startNode;
		return startNode;
	}
	else
	{
		if(totalPagesAllocated + 1 >= totalCapacity)
		{
			return NULL;
		}
		else
		{
			tailNode->next = (pageHeader*)(tailNode->start + pageSize);
			tailNode = tailNode->next;
			makeNode(tailNode, pid);
			return tailNode;
		}
	}
}


void *myallocate(int size, int a, int b, int c)
{
	if(!initializied)
	{
		pageSize = sysconf(_SC_PAGESIZE);
		totalCapacity = BUFFER_SIZE/(pageSize + sizeof(pageHeader));
		initializied++;
	}

	/*pageHeader* temp = getPage(0, 200), *temp2 = getPage(1, 200), *temp3 = getPage(0, 200), *temp4 = getPage(1, 200);
	printf("%c\n", ((char*)(temp->start))[0]);
	printf("%c\n", ((char*)(temp2->start))[0]);
	printf("%c\n", ((char*)(temp3->start))[0]);
	printf("%c\n", ((char*)(temp4->start))[0]);*/
	/*
	pageHeader* temp = (pageHeader*)storage;
	makeNode(storage, 789);*/

	//printf("%d %d %d %d %d %d %d\n", temp->pid, temp->start, temp->top, temp, sizeof(pageHeader), temp->next, totalPagesAllocated);
	return NULL;
	
}

void mydeallocate(void *ptr, int a, int b, int c)
{
	
}