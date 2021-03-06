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
	return calloc(count, size);
}

void *bcc_realloc(void *ptr, unsigned int size)
{
	if (ptr == NULL || size <= 0) {
		return NULL;
	}
	return realloc(ptr, size);
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
	if (dest == NULL || src == NULL || len <= 0) {
		return NULL;
	}
	return strncpy(dest, src, len);
}

char *bcc_memcpy(char *dest, const char *src, int len)
{
	if (dest == NULL || src == NULL || len <= 0) {
		return NULL;
	}
	return memcpy(dest, src, len);
}

/*vector*/

vector_t *create_vector(int size)
{
	vector_t *vector;

	vector = (vector_t *)bcc_malloc(sizeof(vector_t));

	if (size <= 0) {
		size = DEFAULT_VECTOR_SIZE;
	}
	vector->data = (char *)bcc_malloc(sizeof(char*) * size);
	vector->len = 0;
	vector->size = size;

	return vector;
}

BOOL vector_full(vector_t *vector)
{
	if (vector->len == vector->size) {
		return TRUE;
	}
	return FALSE;
}

void resize_vector(vector_t *vector)
{
	int new_size;
	char **new_data;

	new_size = vector->size * 2;
	new_data = (char *)bcc_malloc(sizeof(char*) * new_size);
	bcc_memcpy(new_data, vector->data, vector->len);
	bcc_free(vector->data);
	vector->size = new_size;
	vector->data = new_data;
}

void insert_vector(vector_t *vector, char *item)
{
	if (vector_full(vector)) {
		resize_vector(vector);
	}
	vector->data[vector->len] = item;
	vector->len++;
}

void out_vector(vector_t *vector)
{
	vector->len--;
	bcc_free(vector->data[vector->len]);
}