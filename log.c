
#include "bcc.h"
static int g_logfile_fd = -1;
#define LOG_FILE "C:/bcc.log"
#define LOG_LEVLE 1

//========================================= log =========================================//
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

int string_to_loglevel(char *str)
{
	if (bcc_strequal("DEBUG", str))
		return LL_DEBUG;
	else if (bcc_strequal("TRACE", str))
		return LL_TRACE;
	else if (bcc_strequal("INFO", str))
		return LL_INFO;
	else if (bcc_strequal("WARN", str))
		return LL_WARNING;
	else if (bcc_strequal("ERROR", str))
		return LL_ERROR;
	else
		return LL_OFF;
}

static int get_logfilefd(const char *filename)
{
    int fd = -1;
	if (filename && (*filename)) {
        fd = _open(filename, O_CREAT | O_WRONLY | O_APPEND, _S_IREAD | _S_IWRITE);
	}
	return fd;
}

void init_logfile_fd(int log_limit)
{
#define LCC_LOG	"/lcc.log"
	//char* p_prefix = LOG_PREFIX;
	/*char* p_log_file = NULL;
	int prefix_len, agent_log_len, log_file_len;*/

	struct stat file_stat;

	if (g_logfile_fd == -1) {
		//prefix_len = l_strlen(p_prefix);
		//agent_log_len = l_strlen(LCC_LOG);
		//log_file_len = prefix_len + agent_log_len + 1;
		//p_log_file = (char*)l_malloc(log_file_len);
		//strncpy(p_log_file, p_prefix, prefix_len);
		//strncat(p_log_file, LCC_LOG, agent_log_len);
		//p_log_file[log_file_len - 1] = '\0';
		g_logfile_fd = get_logfilefd(LOG_FILE);
		//l_free(p_log_file);
	}

	if (!log_limit) {
		return;
	}

	if (fstat(g_logfile_fd, &file_stat) != 0) {
		return;
	}

}

void close_logfile_fd()
{
	if (g_logfile_fd != -1)
	_close(g_logfile_fd);
}

void out_put_log(LogLevel l, char *file, int lineno, const char *func_name, char *logformat, ...)
{
	va_list args;
	int _size;
	time_t cur_time;
	struct tm local_tm;

	char *agentlog = NULL;
	int agentlog_len;
	if (l >= LOG_LEVLE) {
		char message[LOG_BUFFSIZE] = {0};
		va_start(args, logformat);
		_size = vsnprintf(message, LOG_BUFFSIZE - 1, logformat, args);
		va_end(args);
		message[LOG_BUFFSIZE - 1] = 0;
		if (_size > LOG_BUFFSIZE - 1 || _size < 0) {
			_size = LOG_BUFFSIZE - 1;
		}
		cur_time = time(NULL);
#ifdef _WIN32
		if (g_logfile_fd != -1 && cur_time > 0 && localtime_s(&local_tm, &cur_time) == NULL) {
#else
		if (g_logfile_fd != -1 && cur_time > 0 && localtime_r(&cur_time, &local_tm) != NULL) {
#endif
			agentlog = malloc(LOG_BUFFSIZE + 128);
			agentlog_len = _snprintf(agentlog, LOG_BUFFSIZE + 128, "%s: %04d-%02d-%02d %02d:%02d:%02d [%s:%d][%s] %s\n", loglevel_to_string(l),
				(1900 + local_tm.tm_year), local_tm.tm_mon + 1, local_tm.tm_mday,
				local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, file, lineno, func_name, message);
			if (agentlog_len > 0) {
				_write(g_logfile_fd, agentlog, agentlog_len);
            }
            if (agentlog) {
                free(agentlog);
                agentlog = NULL;
            }
		}
	}
}

