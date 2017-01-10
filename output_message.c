#include "bcc.h"

void error_message(char *message)
{
	printf("%s(%d): error: %s", g_input_file.file_name, g_input_file.line, message);
}

void warn_message(char *message)
{
	printf("%s(%d): warnning: %s", g_input_file.file_name, g_input_file.line, message);
}
