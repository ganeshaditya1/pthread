/**(startingAddressOfPages + (total_usable_pages - 1) * page_size) = 'o';
	page_header* ptr = &((page_header*)memory_resource)[total_usable_pages - 1];
	ptr->is_allocated = 1;
	ptr->thread_id = 200;
	ptr->thread_page_num = 5;
	printf("%d\n", getFreePageSlot());*/
	//printf("%d, %d, %d", startingAddressOfPages, startingAddressOfPages + 200*page_size, reverseLookup(startingAddressOfPages + 200*page_size));
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