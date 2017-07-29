#ifndef _OUTPUT_MESSAGE_H
#define _OUTPUT_MESSAGE_H


typedef enum _LogLevel {
	LL_DEBUG = 1,
	LL_TRACE = 2,
	LL_INFO = 3,
	LL_WARNING = 4,
	LL_ERROR = 5,
} LogLevel;


void output(LogLevel level, char *log_format, ...)

#ifdef _WIN32
#define ERROR(log_fmt, ...) \
	do {output(LL_ERROR, log_fmt, __VA_ARGS__);} while (0)

#define WARN(log_fmt, ...) \
	do {output(LL_WARNING, log_fmt, __VA_ARGS__);} while (0)
#else
#define ERROR(log_fmt, ...) \
	do {output(LL_ERROR, log_fmt, ##log_arg);} while (0)

#define WARN(log_fmt, ...) \
	do {output(LL_WARNING, log_fmt, ##log_arg);} while (0)
#endif

#endif