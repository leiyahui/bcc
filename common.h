#ifndef _COMMON_H
#define _COMMON_H

typedef int BOOL;
#define TRUE	1
#define FALSE	0

BOOL bcc_strequal(const char *str1, const char *str2);

BOOL bcc_strnequal(const char *str1, const char *str2, unsigned int ch_count);



#endif
