#include "bcc.h"

#define LOG_BUFFERSIZE 100



const char *loglevel_to_string(LogLevel l)
{
	switch (l) {
	case LL_DEBUG:
		return "DEBUG";
	case LL_TRACE:
		return "TRACE";
	case LL_INFO:
		return "INFO";
	case LL_WARNING:
		return "WARN";
	case LL_ERROR:
		return "ERROR";
	default:
		return "UNKNOWN";
	}
}

void output(LogLevel level, char *log_format, ...)
{
	int size;
	char message[LOG_BUFFERSIZE];
	char *error_str;
	va_list args;

	error_str = loglevel_to_string(level);

	va_start(args, log_format);
	size = vsnprintf(message, LOG_BUFFERSIZE - 1, log_format, args);
	va_end(args);

	printf("%s:line: %d, colum :%d :%s", error_str, G_LINE, G_COLUM, message);
}

