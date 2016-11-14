void *myallocate(int size, int a, int b, int c);
void mydeallocate(void *ptr, int a, int b, int c);
#define malloc(x) myallocate(x, __FILE__, __LINE__, 1)
#define free(x) mydeallocate(x, __FILE__, __LINE__, 1)