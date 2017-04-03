#include "../bcc.h"

int lex_suite_init()
{
	return 0;
}

int lex_suite_clean()
{
	return 0;
}

void test_is_dec_num(void)
{
	CU_ASSERT(is_dec_num('0'));
	CU_ASSERT(is_dec_num('9'));
	CU_ASSERT_FALSE(is_dec_num('/'));
	CU_ASSERT_FALSE(is_dec_num(':'));
}

void 

CU_TestInfo lex_test_arrray[] = {
	{"is_dec_num:", test_is_dec_num},
	CU_TEST_INFO_NULL,
};

