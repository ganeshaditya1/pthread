
#ifndef my_malloc
#define my_malloc

void* myallocate(int size, char* file_name, int line_number, int ThreadReq);
void mydeallocate(void* ptr, char* file_name, int line_number, int ThreadReq);

// No need to make these part of the public interface.

//void my_malloc_init();
//page_header* get_header_for_next_usable_page(int thread_id, int *num_of_bytes, int *old_offset);
//page_header* get_last_header_for_current_thread(int thread_id);

void protect_pages(int thread_id);

void unprotect_pages(int thread_id);

void handler(int sig, siginfo_t *si, void *unused);

#define malloc(x) myallocate(x, __FILE__, __LINE__, 1)
#define free(x) mydeallocate(x, __FILE__, __LINE__, 1)

#endif



