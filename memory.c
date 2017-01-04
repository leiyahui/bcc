#include "bcc.h"

void* bcc_malloc(unsigned int size)
{
	if (size <= 0) {
		return NULL;
	}
	return malloc(size);
}

void* bcc_calloc(unsigned int count, unsigned int size)
{
	if (count <= 0 || size <= 0) {
		return NULL;
	}
	return bcc_calloc(count, size);
}

void bcc_free(void *ptr)
{
	if (ptr != NULL) {
		free(ptr);
	}
	ptr = NULL;
}
