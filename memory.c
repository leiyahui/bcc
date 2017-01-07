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

char* bcc_strcpy(char *dest, const char *src)
{
	if (dest == NULL || src == NULL) {
		return NULL;
	}
	return strcpy(dest ,src);
}

char *bcc_strncpy(char *dest, const char *src, int len)
{
	if (dest == NULL || src == NULL) {
		return NULL;
	}
	return strncpy(dest, src, len);
}