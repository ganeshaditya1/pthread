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

int readPageFromDisk(char* buffer, int diskSlotNumber)
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
    
    posix_memalign(&memory_resource, 4096, num_of_pages*page_size);
    
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

void* getPageLocation(int tid, int pageNo)
{
	for (int i = 0; i < hddPageIndex; i++) 
    {
        page_header* ptr = &((page_header*)memory_resource)[i];
        if(ptr->is_allocated && ptr->thread_id == tid && ptr->thread_page_num == pageNo)
        {
        	return startingAddressOfPages + i * page_size;
        }
    }
    return NULL;
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

void swapPageDisk(){}


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
    void* currentPage = getPageLocation(thread_id, currentPageIndex);

    ptr_header* temp = (ptr_header*)currentPage;
    /*while(temp->next != NULL)
    {
    	if(temp->free && temp->size >= (sizeof(ptr_header) + num_of_bytes))
    	{
    		//allocate memory here itself
    	}
    	else
    	{
    		if(temp->next - currentPage >= page_size)
    		{
    			currentPageIndex = (temp->next - startingAddressOfPages) / page_size;
    			currentPage = getPageLocation(thread_id, currentPageIndex);
    			temp = (temp->next - (startingAddressOfPages  + currentPageIndex * page_size)) + currentPage;
    		}
    		else
    		{
    			temp = temp->next;
    		}
    	}
    }*/


    /*int size =  num_of_bytes;
    
    if(num_of_pages_req > remaining_pages)
        return NULL;
    
    int first_page_found = -1;
    page_header* ptr = NULL;
    page_header* first_ptr = NULL;
    void* page_ptr = NULL;
    
    while (size > 0) 
    {
        int old_offset = -1;
        printf("get a header for page in which data will be written\n");
        ptr = get_header_for_next_usable_page(thread_id, &size, &old_offset);
        printf("got a header for page in which data will be written\n");
        printf("first frame found? %d\n", first_page_found);
        if(first_page_found == -1) 
        {
            first_ptr = ptr;
            first_page_found = 1;
            page_ptr = memory_resource + (pages_used_by_page_headers + first_ptr->thread_header_num)*page_size;
            page_ptr += old_offset + 1;
        }
        
    }
    //mprotect(memory_resource, num_of_pages * page_size, PROT_NONE);
    return page_ptr;*/
}


void handler(int sig, siginfo_t *si, void *unused)
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
}



int main() 
{
	my_malloc_init();
	*(startingAddressOfPages + (total_usable_pages - 1) * page_size) = 'o';
	page_header* ptr = &((page_header*)memory_resource)[total_usable_pages - 1];
	ptr->is_allocated = 1;
	ptr->thread_id = 200;
	ptr->thread_page_num = 5;
	printf("%d\n", getFreePageSlot());
	//printf("%d\n", getFreeDiskSlot());
	/* Testing evict page mechanism 

	*(startingAddressOfPages + (total_usable_pages - 1) * page_size) = 'o';
	page_header* ptr = &((page_header*)memory_resource)[total_usable_pages - 1];
	ptr->is_allocated = 1;
	ptr->thread_id = 200;
	ptr->thread_page_num = 5;
	evictPage();

	page_header* ptr2 = &((page_header*)memory_resource)[hddPageIndex];
	printf("%d, %d, %d, %d, %d\n", ptr2->is_allocated, ptr2->thread_id, ptr2->thread_page_num, hddPageIndex);
	*/

	/*Testing the disk file functions  

	*(startingAddressOfPages + 90*page_size) = 'o';

	*(startingAddressOfPages + 33*page_size) = 'p';

	writePageToDisk(90, 0);
	writePageToDisk(33, 1);
	char* buffer = malloc(page_size);
	readPageFromDisk(buffer, 0);
	printf("%c\n", buffer[0]);
	readPageFromDisk(buffer, 1);
	printf("%c\n", buffer[0]);
	*/

	/* Testing swap Page function 
	*(startingAddressOfPages + 90*page_size) = 'o';
	page_header* ptr = &((page_header*)memory_resource)[90];
	ptr->is_allocated = 1;
	ptr->thread_id = 200;
	ptr->thread_page_num = 5;

	*(startingAddressOfPages + 33*page_size) = 'p';
	page_header* ptr2 = &((page_header*)memory_resource)[33];
	ptr2->is_allocated = 1;
	ptr2->thread_id = 20;
	ptr2->thread_page_num = 15;

	printf("%c, %c\n", *(startingAddressOfPages + 90 * page_size), *(startingAddressOfPages + 33 * page_size));
	swapPage(90, 33);
	printf("%c, %c\n", *(startingAddressOfPages + 90 * page_size), *(startingAddressOfPages + 33 * page_size));

	//printf("%d\n", getPageLocation(200, 5) == getPageLocation(20, 15));
	printf("%c, %c\n", *((char*)getPageLocation(200, 5)), *((char*)getPageLocation(20, 15)));

	*/
	/**(startingAddressOfPages + 90*page_size) = 'o';
	page_header* ptr = &((page_header*)memory_resource)[90];
	ptr->is_allocated = 1;
	ptr->thread_id = 200;
	ptr->thread_page_num = 5;

	printf("%c YO\n", *((char*)getPageLocation(200, 5)));*/

	//printf("%d YO", canSatisfyRequirement(total_usable_pages);

    //mprotectFunc(getPhyMem(),8388608,PROT_NONE);
    // Not sure if we need this. Will uncomment this later.
    //mprotect(memory_resource, 2048 * 4096, PROT_NONE);
    /*char* test = myallocate(4099, __FILE__, __LINE__ , 1);
    Gthread_id = 1;
    *(test) = 'a';
    *(test+1) = 'b';
    *(test+2) = 'c';
    *(test+4098) = 'r';*/
    /*
    printf("test is: %p %c%c%c\n",test,*(test),*(test+1),*(test+4098));
    
    char* test1 = myallocate(4093, __FILE__, __LINE__ , 1);
    Gthread_id=1;
    mprotect(memory_resource, 2048 * 4096, PROT_NONE);
    //printf("test is: %p\n",test);
    *(test1) = 'd';
    *(test1+1) = 'e';
    *(test1+2) = 'f';
    printf("test is:%p %c%c%c\n",test1,*(test1),*(test1+1),*(test1+2));
    
    char* test2 = myallocate(3, __FILE__, __LINE__ , 2);
    mprotect(memory_resource, 2048 * 4096, PROT_NONE);
    Gthread_id = 2;
    *(test2) = 't';
    *(test2+1) = 'u';
    *(test2+2) = 'v';
    printf("test is: %p %c%c%c\n",test2,*(test2),*(test2+1),*(test2+2));
    */
    return 0;
}








