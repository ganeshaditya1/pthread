
#define malloc(x) myallocate(x, __FILE__, __LINE__, 1)
#define free(x) mydeallocate(x, __FILE__, __LINE__, 1)

void *myallocate(int size, const int a, const int b, const int c);
void mydeallocate(void *ptr, int a, int b, int c);