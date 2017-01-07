#ifndef _MEMORY_H
#define _MEMORY_H

#include "bcc.h"

void* bcc_malloc(unsigned int size);

void* bcc_calloc(unsigned int count, unsigned int size);

char* bcc_strcpy(char *dest, const char *src);

char* bcc_strncpy(char *dest, const char *src, int len);

void bcc_free(void *ptr);

#endif