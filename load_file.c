#include "bcc.h"



void load_file(char *file_name, input_file_t *input_file)
{
	HANDLE file_handle, file_mapping_handle;
	int file_size;


	file_handle = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	file_size = GetFileSize(file_handle, NULL);

	file_mapping_handle = CreateFileMapping(file_handle, NULL, PAGE_READWRITE, 0, file_size + 1, NULL);

	input_file->cursor = input_file->base = (unsigned char*)MapViewOfFile(file_mapping_handle, FILE_MAP_WRITE, 0, 0, 0);
	input_file->file_name = file_name;
	input_file->h_file = file_handle;
	input_file->h_filemapping = file_mapping_handle;
	input_file->line = 1;
	input_file->base[file_size] = END_OF_FILE;
}

void unload_file(input_file_t *input_file)
{
	UnmapViewOfFile(input_file->base);
	CloseHandle(input_file->h_filemapping);
	SetFilePointer(input_file->h_file, input_file->size, NULL, FILE_BEGIN);
	SetEndOfFile(input_file->h_file);
	CloseHandle(input_file->h_file);
}