#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/mman.h>
#include "my_malloc.h"


char *memory_resource;

typedef struct {
    int is_allocated;
    int thread_id;
    int thread_page_num;
}page_header;

typedef struct ptr_header{
	int free;
	int size;
	struct ptr_header* next;
}ptr_header;

const int page_size = 4096;
const int num_of_pages = (8 * 1024 * 1024) / 4096;



int Gthread_id = 0;

int num_of_page_headers;
int page_headers_per_page;
int pages_used_by_page_headers;
int total_usable_pages;
int remaining_pages;
bool initialized = false;
int hddPageIndex;
char* startingAddressOfPages;

FILE *swapFile;

void swapPage(int slotNumber1, int slotNumber2);

int getFreeDiskSlot()
{
	for (int i = num_of_pages; i < num_of_page_headers; i++) 
    {
        page_header* ptr = &((page_header*)memory_resource)[i];
        if(!ptr->is_allocated)
        {
        	return i - num_of_pages;
        }
    }
    return -1;
}

int getFreePageSlot()
{
	for(int i = 0; i < num_of_pages; i++)
	{
		page_header* ptr = &((page_header*)memory_resource)[i];
        if(!ptr->is_allocated)
        {
        	return i;
        }
    }
    return -1;
}

void evictPage()
{
	int freeDiskSpace, diskSlotNumber = getFreeDiskSlot(), slotNumber = total_usable_pages - 1;
	printf("%d, %d\n", diskSlotNumber + num_of_pages, slotNumber);
	if(diskSlotNumber == -1)
	{
		return;
	}
	fseek(swapFile, diskSlotNumber * page_size, SEEK_SET);
	fwrite(startingAddressOfPages + page_size * slotNumber, 1, page_size, swapFile);
	fseek(swapFile, 0, SEEK_SET);

	page_header* ptr = &((page_header*)memory_resource)[slotNumber];
	page_header* ptr2 = &((page_header*)memory_resource)[diskSlotNumber + num_of_pages];
	*ptr2 = *ptr;
	ptr->is_allocated = 0;

}

int readPageFromDisk(int diskSlotNumber)
{
	int slotNumber = getFreePageSlot();
	if(slotNumber == -1)
	{
		evictPage();
	}
	slotNumber = getFreePageSlot();
	if(slotNumber == -1)
	{
		return -1;
	}
	fseek(swapFile, diskSlotNumber * page_size, SEEK_SET);
	fread(startingAddressOfPages + page_size * slotNumber, 1, page_size, swapFile);
	fseek(swapFile, 0, SEEK_SET);
}


void my_malloc_init()
{
    num_of_page_headers = 3 * num_of_pages;
    page_headers_per_page = page_size / sizeof(page_header);
    printf("page headers per page: %d\n", page_headers_per_page);
    pages_used_by_page_headers = num_of_page_headers/page_headers_per_page;
    printf("pages used by page headers: %d\n", pages_used_by_page_headers);
    total_usable_pages = num_of_pages - pages_used_by_page_headers;
    printf("total usable pages: %d\n", total_usable_pages);
    remaining_pages = total_usable_pages;
    
    posix_memalign((void*)&memory_resource, 4096, num_of_pages*page_size);
    
    printf("Physical memory starts from %p\n", memory_resource);
    startingAddressOfPages = memory_resource + (pages_used_by_page_headers * page_size);
    printf("page memory starts from %p\n", startingAddressOfPages);


    for (int i = 0; i < num_of_page_headers; i++) 
    {
        page_header* ptr = &((page_header*)memory_resource)[i];
        ptr->is_allocated = 0;
        ptr->thread_page_num = 0;
    }
    hddPageIndex = num_of_pages;


    // Initialize the swap file.


	swapFile = fopen("swap.txt", "rw");
	fseek(swapFile, 16 * 1024 * 1024, SEEK_SET);
	fputc('\0', swapFile);

    printf("Init finished\n\n\n");
}

void loadPage(int tid, int pageNo)
{
	for (int i = 0; i < hddPageIndex; i++) 
    {
        page_header* ptr = &((page_header*)memory_resource)[i];
        if(ptr->is_allocated && ptr->thread_id == tid && ptr->thread_page_num == pageNo)
        {
        	swapPage(i, pageNo);
        	return;
        }
    }
    for (int i = hddPageIndex; i < num_of_pages; i++) 
    {
        page_header* ptr = &((page_header*)memory_resource)[i];
        if(ptr->is_allocated && ptr->thread_id == tid && ptr->thread_page_num == pageNo)
        {
        	int slotNumber = readPageFromDisk(i);
        	swapPage(slotNumber, pageNo);
        	return;
        }
    }

    int slotNumber = getFreePageSlot();
    if(slotNumber == -1)
    {
    	evictPage();
    	slotNumber = getFreePageSlot();
    }
    swapPage(slotNumber, pageNo);
    page_header* ptr = &((page_header*)memory_resource)[pageNo];
    ptr->is_allocated = 1;
    ptr->thread_id = tid;
    ptr->thread_page_num = pageNo;
    ptr_header* nodeHeader = (ptr_header*)(startingAddressOfPages + pageNo * page_size);
    nodeHeader->free = 1;
    nodeHeader->size = 0;
    nodeHeader->next = NULL;
}

bool canSatisfyRequirement(int size)
{
	for (int i = 0; i < num_of_page_headers; i++) 
    {
        page_header* ptr = &((page_header*)memory_resource)[i];
        if(!ptr->is_allocated)
        {
        	size--;
        }
    }

    if(size <= 1)
    {
    	return true;
    }

}

void swapPage(int slotNumber1, int slotNumber2)
{
	char* buffer = (char*)malloc(page_size);
	memcpy(buffer, startingAddressOfPages + slotNumber1 * page_size, page_size);
	memcpy(startingAddressOfPages + slotNumber1 * page_size, startingAddressOfPages + slotNumber2 * page_size, page_size);
	memcpy(startingAddressOfPages + slotNumber2 * page_size, buffer, page_size);

	page_header* headerArray = (page_header*)memory_resource;
	page_header temp = headerArray[slotNumber1];
	headerArray[slotNumber1] = headerArray[slotNumber2];
	headerArray[slotNumber2] = temp;

}



int reverseLookup(char* obj)
{
	return ((int)(obj - startingAddressOfPages)) / page_size;
}

void* allocateMemory(int thread_id, ptr_header* temp, int currentPageIndex, int size)
{
    char* currentPageStart = startingAddressOfPages + currentPageIndex * page_size;
    int sizeLeft = (int)(((char*)(temp + 1)) - currentPageStart);
    int sizeRequired = size - sizeLeft;
    if(sizeRequired > 0)
    {    
        int numberOfPagesRequired = (int)ceil(sizeRequired/(double)4096);
        for(int i = numberOfPagesRequired; i > 0; i--)
        {
            loadPage(thread_id, currentPageIndex + i);
            currentPageIndex++;
        }
    }
    temp->free = 0;
    temp->size = size - sizeof(ptr_header);
    if(temp->next == NULL)
    {
        temp->next = (ptr_header*)((char*)(temp + 1) + size);
        ptr_header* temp2 = temp->next;
        temp2->free = 1;
        temp2->size = 0;
        temp2->next = NULL;
    }
    else
    {
        char* endOfCurrentBlock = (char*)(temp + 1) + size;
        printf("%p, %p  AA\n", temp->next, endOfCurrentBlock);
        if(temp->next > endOfCurrentBlock)
        {

            ptr_header* temp2 = (ptr_header*)((char*)(temp + 1) + size);
            temp2->next = temp->next;
            temp->next = temp2;
            temp2->size = ((char*)temp2->next - (char*)temp2) - sizeof(ptr_header);
            printf("%x, %x, AAAA\n", temp2->next, temp2);
            temp2->free = 1;
        }
        
    }
    return (char*)(temp + 1);

}


void* myallocate(int num_of_bytes, char* file_name, int line_number, int thread_id)
{
	if(!initialized)
	{
		my_malloc_init();
		initialized = true;
	}
    //mprotect(memory_resource, num_of_pages * page_size, PROT_READ | PROT_WRITE);
    int num_of_pages_req = num_of_bytes/page_size + 1;
    if(num_of_pages_req > 1 && !canSatisfyRequirement(num_of_pages_req))
    {
    	return NULL;
    }

    int currentPageIndex = 0;
    loadPage(thread_id, currentPageIndex);

    ptr_header* temp = (ptr_header*)(startingAddressOfPages + currentPageIndex * page_size);
    while(true)
    {
    	if((temp->free && temp->size >= (sizeof(ptr_header) + num_of_bytes)) || temp->next == NULL)
    	{
            return allocateMemory(thread_id, temp, currentPageIndex, num_of_bytes);
    	}
    	else
    	{
            int nextPageIndex = reverseLookup((char*)temp->next);
    		if(nextPageIndex != currentPageIndex)
    		{
                loadPage(thread_id, nextPageIndex);
                temp = temp->next;
                currentPageIndex = nextPageIndex;
    			/*currentPageIndex = (temp->next - startingAddressOfPages) / page_size;
    			currentPage = getPageLocation(thread_id, currentPageIndex);
    			temp = (temp->next - (startingAddressOfPages  + currentPageIndex * page_size)) + currentPage;*/
    		}
    		else
    		{
    			temp = temp->next;
    		}
    	}
    }

    
}


/*void handler(int sig, siginfo_t *si, void *unused)
{
	int thread_id = Gthread_id;
	int frame = (int)(( (char*)si->si_addr - (memory_resource + pages_used_by_page_headers * page_size) )) / page_size;
    
    	int flag = 0;
    	int i;
	for (i = 0; i < num_of_page_headers; i++)
	{
		page_header* ptr = &((page_header*)memory_resource)[i];
		if(ptr->is_allocated == 1 && ptr->thread_id == thread_id && ptr->thread_page_num == frame)
		{
                	swapPage(ptr->thread_page_num,frame);
                	mprotect(memory_resource, page_size * num_of_pages, PROT_NONE);
                	mprotect(getPageLocation(thread_id, frame), page_size, PROT_READ | PROT_WRITE);
                	flag = 1;
                	break;
            	}
		
	}
    	if(!flag)
	{
        	//printf("Got SIGSEGV at address: 0x%lx\n",(long) si->si_addr);
        	exit(0);
    	}
}*/



int main() 
{
	my_malloc_init();

    //Load page test code.

    /*page_header *tempa = &((page_header*)memory_resource)[105];
    tempa->is_allocated = 1;
    tempa->thread_id = 99;
    tempa->thread_page_num = 101;

    ptr_header* to = (startingAddressOfPages + 105 * page_size);
    to->free = 1;
    to->size = 88;
    to->next = NULL;

    printf("%d, %d, %d\n", to->free, to->size, to->next);
	
    loadPage(99, 101);
    page_header* temp = &((page_header*)memory_resource)[101];
    printf("%d, %d, %d\n", temp->is_allocated, temp->thread_id, temp->thread_page_num);
    to = (startingAddressOfPages + 101 * page_size);
    printf("%d, %d, %d\n", to->free, to->size, to->next);
*/
    page_header *tempa = &((page_header*)memory_resource)[1];
    tempa->is_allocated = 1;
    tempa->thread_id = 2;
    tempa->thread_page_num = 1;
    
    ptr_header* to2 = (startingAddressOfPages + 1 * page_size + 700);
    to2->free = 1;
    to2->size = 200;
    to2->next = NULL;

    ptr_header* to = (startingAddressOfPages + 1 * page_size + 200);
    to->free = 0;
    to->size = 600 - sizeof(ptr_header);
    to->next = to2;
//void* allocateMemory(int thread_id, ptr_header* temp, int currentPageIndex, int size)

    printf("%d, %d, %p\n", to->free, to->size, to->next);
    allocateMemory(2, to, 1, 484);
    printf("%d, %d, %p\n", to->free, to->size, to->next);
    to = to->next;

    printf("%d, %d, %p\n", to->free, to->size, to->next);
    return 0;
}








