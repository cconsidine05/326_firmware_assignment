#include <stdint.h>

int _write(int file, char *ptr, int len)
{
	(void)file;
	// TODO: route to UART for real output
	return len;
}

int _read(int file, char *ptr, int len)
{
	(void)file;
	(void)ptr;
	(void)len;
	return 0;
}