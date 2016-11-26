#include <stdio.h>

void main()
{
	FILE *swapFile;
	swapFile = fopen("swap.txt", "w");
	fseek(swapFile, 8 * 1024 * 1024, SEEK_SET);
	fputc('\0', swapFile);
	fclose(swapFile);
}