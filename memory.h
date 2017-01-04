#ifndef _MEMORY_H
#define _MEMORY_H

#include "bcc.h"

void* bcc_malloc(unsigned int size);

void* bcc_calloc(unsigned int count, unsigned int size);

void bcc_free(void *ptr);

#endif