#include "bcc.h"

input_file_t g_input_file;

#ifdef _WIN32

void load_file(char *file_name)
{
	HANDLE file_handle, file_mapping_handle;
	int file_size;


	file_handle = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
		OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	
	file_size = GetFileSize(file_handle, NULL);

	file_mapping_handle = CreateFileMapping(file_handle, NULL, PAGE_READWRITE, 0, file_size + 1, NULL);

	g_input_file.cursor = g_input_file.base = (char*)MapViewOfFile(file_mapping_handle, FILE_MAP_WRITE, 0, 0, 0);
	g_input_file.file_name = file_name;
	g_input_file.h_file = file_handle;
	g_input_file.h_filemapping = file_mapping_handle;
	g_input_file.line = 1;
	g_input_file.base[file_size] = END_OF_FILE;
}

void unload_file()
{
	UnmapViewOfFile(g_input_file.base);
	CloseHandle(g_input_file.h_filemapping);
	SetFilePointer(g_input_file.h_file, g_input_file.size, NULL, FILE_BEGIN);
	SetEndOfFile(g_input_file.h_file);
	CloseHandle(g_input_file.h_file);
}
#else

void load_file(char* file_name)
{
	int fd, file_size;
	struct stat f_stat;
	char *s_addr;

	fd = open(file_name, O_RDONLY);
	file_size = fstat(fd, &f_stat);
	s_addr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
	g_input_file.cursor = g_input_file.base = s_addr;
	g_input_file.file_name = file_name;
	g_input_file.fd = fd;
	g_input_file.line = 1;
	g_input_file.base[file_size] = END_OF_FILE;
}
#endif