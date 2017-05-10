#ifndef _MEMORY_H
#define _MEMORY_H

#include "bcc.h"


/*basic memroy operation*/
void* bcc_malloc(unsigned int size);

void* bcc_calloc(unsigned int count, unsigned int size);

void *bcc_realloc(void *ptr, unsigned int size);

char* bcc_strcpy(char *dest, const char *src);

char* bcc_strncpy(char *dest, const char *src, int len);

char *bcc_memcpy(char *dest, const char *str, int len);

void bcc_free(void *ptr);


/*vector*/

#define DEFAULT_VECTOR_SIZE	20

typedef struct _vector_t {
	char **data;
	int len;
	int size;
}vector_t;

vector_t *create_vector(int size);

BOOL vector_full(vector_t *vector);

void resize_vector(vector_t *vector);

void insert_vector(vector_t *vector, char *item);

void out_vector(vector_t *vector);
#endif