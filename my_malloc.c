#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/mman.h>
#include "my_malloc.h"
#include "scheduler.h"


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




int num_of_page_headers;
int page_headers_per_page;
int pages_used_by_page_headers;
int total_usable_pages;
int remaining_pages;
boolean initialize = false;
int hddPageIndex;
char* startingAddressOfPages;

FILE *swapFile;

void swapPage(int slotNumber1, int slotNumber2);
void handler(int sig, siginfo_t *si, void *unused);

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
	//printf("%d, %d\n", diskSlotNumber + num_of_pages, slotNumber);
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
    if (swapFile != 0)
    {
        fseek(swapFile, 16 * 1024 * 1024 - 1, SEEK_SET);
        fputc('\0', swapFile);
        printf("swap file initialized\n\n");
    }
    else
        perror("fopen");
    

    //signal handler
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;
    if (sigaction(SIGSEGV, &sa, NULL) == -1)
    {
        perror("sigaction");
    }
    printf("signal handler registered\n\n");

    printf("Init finished\n\n\n");
}

void loadPage(int tid, int pageNo)
{
    //protect pages
    mprotect(startingAddressOfPages + pageNo*page_size, page_size, PROT_READ | PROT_WRITE);
    //protect_pages(tid);


	for (int i = 0; i < hddPageIndex; i++) 
    {
        page_header* ptr = &((page_header*)memory_resource)[i];
        if(ptr->is_allocated && ptr->thread_id == tid && ptr->thread_page_num == pageNo)
        {
        	swapPage(i, pageNo);
            mprotect(startingAddressOfPages + i*page_size, page_size, PROT_NONE);
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
            mprotect(startingAddressOfPages + slotNumber*page_size, page_size, PROT_NONE);
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
    mprotect(startingAddressOfPages + slotNumber*page_size, page_size, PROT_NONE);
    page_header* ptr = &((page_header*)memory_resource)[pageNo];
    ptr->is_allocated = 1;
    ptr->thread_id = tid;
    ptr->thread_page_num = pageNo;
    ptr_header* nodeHeader = (ptr_header*)(startingAddressOfPages + pageNo * page_size);
    nodeHeader->free = 1;
    nodeHeader->size = 0;
    nodeHeader->next = NULL;

    //unprotect pages
    //mprotect(startingAddressOfPages + pageNo*page_size, page_size, PROT_NONE);
    //unprotect_pages(tid);
}

boolean canSatisfyRequirement(int size)
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
	char* buffer = (char*)calloc(1, page_size);
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
    char* currentPageEnd = startingAddressOfPages + (currentPageIndex + 1) * page_size;
    int sizeLeft = (int)(currentPageEnd - (char*)(temp + 1));
    int sizeRequired = size - sizeLeft + sizeof(ptr_header);
    if(sizeRequired > 0)
    {    
        int numberOfPagesRequired = (int)ceil(sizeRequired/(double)4096);
        for(int i = numberOfPagesRequired; i > 0; i--)
        {
            loadPage(thread_id, ++currentPageIndex);
            //currentPageIndex++;
        }
    }
    temp->free = 0;
    temp->size = size;
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
        if((char*)temp->next > endOfCurrentBlock + sizeof(ptr_header))
        {

            ptr_header* temp2 = (ptr_header*)((char*)(temp + 1) + size);
            temp2->next = temp->next;
            temp->next = temp2;
            temp2->size = ((char*)temp2->next - (char*)temp2) - sizeof(ptr_header);
            temp2->free = 1;
        }
        
    }
    return (char*)(temp + 1);

}


void* myallocate(int num_of_bytes, char* file_name, int line_number, int thread)
{

    int thread_id = getCurrentTid();
	if(!initialize)
	{
		my_malloc_init();
		initialize = true;
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
    	if((temp->free && temp->size >= (num_of_bytes)) || temp->next == NULL)
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

void mydeallocate(void* ptr, char* file_name, int line_number, int ThreadReq)
{
    int thread_id = getCurrentTid();
    loadPage(thread_id, reverseLookup(ptr));
    ptr_header* temp = (ptr_header*)ptr - 1;
    temp->free = 1;
}

void protect_pages(int thread_id)
{
    for (int i = 0; i < hddPageIndex; i++)
    {
        page_header *ptr = &((page_header*)memory_resource)[i];
        if (ptr -> thread_id == thread_id)
            mprotect(startingAddressOfPages + i*page_size, page_size, PROT_NONE);    
    }
}

void unprotect_pages(int thread_id)
{
    for (int i = 0; i < hddPageIndex; i++)
    {
        page_header *ptr = &((page_header*)memory_resource)[i];
        if (ptr -> thread_id == thread_id)
            mprotect(startingAddressOfPages + i*page_size, page_size, PROT_READ | PROT_WRITE);    
    }
}



int getPageLocation(int tid, int pageNo)
{
    for (int i = 0; i < hddPageIndex; i++) 
    {
        page_header* ptr = &((page_header*)memory_resource)[i];
        if(ptr->is_allocated && ptr->thread_id == tid && ptr->thread_page_num == pageNo)
        {
            return i;
        }
    }
    return -1;
    // return NULL;
}

void handler(int sig, siginfo_t *si, void *unused)
{
	int thread_id = getCurrentTid();
    char* addressAccessed = si->si_addr;
    int slotNumber = reverseLookup(addressAccessed);
    mprotect(startingAddressOfPages + slotNumber * page_size, page_size, PROT_READ | PROT_WRITE);
    int currentSlotNumber = getPageLocation(thread_id, slotNumber);
    printf("%d, %d, %d slots\n", thread_id, slotNumber, currentSlotNumber);
    mprotect(startingAddressOfPages + slotNumber * page_size, page_size, PROT_READ | PROT_WRITE);    
    swapPage(slotNumber, currentSlotNumber);
    mprotect(startingAddressOfPages + currentSlotNumber * page_size, page_size, PROT_NONE);
	/*int frame = (int)(( (char*)si->si_addr - (memory_resource + pages_used_by_page_headers * page_size) )) / page_size;
    
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
    printf("YOO\n");*/
}




