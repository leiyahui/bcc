#include "bcc.h"

#define LOG_BUFFERSIZE 100

void error_message(char *log_format, ...)
{
	int size;
	char message[LOG_BUFFERSIZE];
	char *to_write;
	va_list args;

	va_start(args, log_format);
	size = vsnprintf(message, LOG_BUFFERSIZE - 1, log_format, args);
	va_end(args);

	to_write = bcc_malloc(LOG_BUFFERSIZE);

	snprintf(to_write, LOG_BUFFERSIZE, "%s:%s", "Error", message);
	printf(to_write);
}

void warn_message(char *log_format, ...)
{
	int size;
	char message[LOG_BUFFERSIZE];
	char *to_write;
	va_list args;

	va_start(args, log_format);
	size = vsnprintf(message, LOG_BUFFERSIZE - 1, log_format, args);
	va_end(args);

	to_write = bcc_malloc(LOG_BUFFERSIZE);

	snprintf(to_write, LOG_BUFFERSIZE, "%s:%s", "Warn", message);
	printf(to_write);
}
