#include <stdio.h>
#include "memManager.h"

void main()
{
	char*a = (char*)malloc(20);
	printf("%c\n", a[0]);
}