#ifndef _LOAD_FILE_H
#define _LOAD_FILE_H

#include "bcc.h"

#define END_OF_FILE 255

typedef struct _input_file_t {
	char* file_name;
	unsigned char* base;
	unsigned char *cursor;
	HANDLE h_file;
	HANDLE h_filemapping;
	int line;
	int size;
}input_file_t;

void load_file(char *file_name, input_file_t *input_file);

void unload_file(input_file_t *input_file);

#endif