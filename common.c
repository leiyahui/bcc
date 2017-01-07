#include "bcc.h"

BOOL bcc_strequal(const char *str1, const char *str2)
{
	if(str1 == NULL || str2 == NULL) {
		return FALSE;
	}
	if (_stricmp(str1, str2)) {
		return FALSE;
	}
	return TRUE;
}

BOOL bcc_strnequal(const char *str1, const char *str2, unsigned int ch_count)
{
	if(str1 == NULL || str2 == NULL) {
		return FALSE;
	}
	if (_strnicmp(str1, str2, ch_count)) {
		return FALSE;
	}
	return TRUE;
}

int bcc_strlen(const char *str)
{
	if (str == NULL) {
		return 0;
	}
	return strlen(str);
}