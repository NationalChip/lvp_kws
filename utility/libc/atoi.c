#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

int atoi(const char *nptr)
{
	return (int) strntoumax(nptr, (char **)NULL, 10, ~(size_t) 0);
}