#ifndef _LOAD_FILE_H
#define _LOAD_FILE_H

#include "bcc.h"

#define END_OF_FILE 255

typedef struct _input_file_t {
	char* file_name;
	char* base;
	char *cursor;
#ifdef _WIN32
	HANDLE h_file;
	HANDLE h_filemapping;
#else
	int fd;
#endif
	int line;
	int colum;
	int size;
}input_file_t;

extern input_file_t g_input_file;

#define G_CURSOR	g_input_file.cursor
#define G_LINE		g_input_file.line
#define G_COLUM		g_input_file.colum

void load_file(char *file_name);

void unload_file();

#endif