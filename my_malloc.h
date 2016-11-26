
#ifndef my_malloc
#define my_malloc

void* myallocate(int size, char* file_name, int line_number, int ThreadReq);

// No need to make these part of the public interface.

//void my_malloc_init();
//page_header* get_header_for_next_usable_page(int thread_id, int *num_of_bytes, int *old_offset);
//page_header* get_last_header_for_current_thread(int thread_id);

#endif



