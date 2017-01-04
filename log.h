#ifndef _LOG_H
#define _LOG_H

#define LOG_FILE_SIZE			30 * 1024 * 1024
#define LOG_BUFFSIZE			1024 * 100

typedef enum _LogLevel {
	LL_DEBUG = 1,
	LL_TRACE = 2,
	LL_INFO = 3,
	LL_WARNING = 4,
	LL_ERROR = 5,
	LL_OFF = 6
} LogLevel;

const char *loglevel_to_string(LogLevel l);
int string_to_loglevel(char *str);

void init_logfile_fd(int log_limit);
void close_logfile_fd();
void out_put_log(LogLevel l, char *file, int lineno, const char *func_name, char *logformat, ...);

#define log_error(log_fmt, ...) \
	do {out_put_log(LL_ERROR, __FILE__, __LINE__, __FUNCTION__, log_fmt, __VA_ARGS__);} while (0)

#define log_warn(log_fmt, ...) \
	do {out_put_log(LL_WARNING, __FILE__, __LINE__, __FUNCTION__, log_fmt, __VA_ARGS__);} while (0)

#define log_info(log_fmt, ...) \
	do {out_put_log(LL_INFO, __FILE__, __LINE__, __FUNCTION__, log_fmt, __VA_ARGS__);} while (0)

#define log_trace(log_fmt, ...) \
	do {out_put_log(LL_TRACE, __FILE__, __LINE__, __FUNCTION__, log_fmt, __VA_ARGS__);} while (0)

#define log_debug(log_fmt, ...) \
	do {out_put_log(LL_DEBUG, __FILE__, __LINE__, __FUNCTION__, log_fmt, __VA_ARGS__);} while (0)

void apm_fatal_signal_handler();


#endif
