#include "../bcc.h"

extern int lex_suite_init(void);
extern int lex_suite_clean(void);
extern CU_TestInfo lex_test_arrray[];

CU_SuiteInfo suites[] = {
	{ "lexical", lex_suite_init, lex_suite_clean, NULL, NULL, lex_test_arrray },
	CU_SUITE_INFO_NULL,
};

void main()
{
	if (CU_initialize_registry()) {
		printf("Initialization of Test Registry failed! \n");
		exit(EXIT_FAILURE);
	}
	CU_register_suites(suites);
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
}