#include "../bcc.h"

int lex_suite_init()
{
	init_keywords_hash();
	init_identify_hashtable();
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

/*test scan identifier*/

char *trim_str_end(char* ident)
{
	int old_len;
	char *new_str;

	old_len = strlen(ident);
	new_str = (char*)bcc_malloc(old_len);
	bcc_strncpy(new_str, ident, old_len - 1);
	new_str[old_len] = '\0';
	
	return new_str;
}

void test_identifier(char *ident)
{
	char *new_str;

	G_CURSOR = ident;
	scan_identifier();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_IDENTIFIER);

	new_str = trim_str_end(ident);
	CU_ASSERT_STRING_EQUAL(g_current_token.token_value.ptr, new_str);
	bcc_free(new_str);
}

void test_keywords(char *ident, int tk_kind)
{
	G_CURSOR = ident;
	scan_identifier();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, tk_kind);
}

void test_scan_identifier(void)
{
	test_identifier("test_ident ");
	test_identifier("test_ident\r");
	test_identifier("test_ident\t");
	test_identifier("tnedi_tset ");
	test_identifier("hello ");


	test_keywords("auto ", TK_AUTO);
	test_keywords("extern ", TK_EXTERN);
	test_keywords("register ", TK_REGISTER);
	test_keywords("static ", TK_STATIC);
	test_keywords("typedef ", TK_TYPEDEF);
	test_keywords("const ", TK_CONST);
	test_keywords("volatile ", TK_VOLATILE);
	test_keywords("void ", TK_VOID);
	test_keywords("char ", TK_CHAR);
	test_keywords("short ", TK_SHORT);
	test_keywords("int ", TK_INT);
	test_keywords("long ", TK_LONG);
	test_keywords("float ", TK_FLOAT);
	test_keywords("double ", TK_DOUBLE);
	test_keywords("signed ", TK_SIGNED);
	test_keywords("unsigned ", TK_UNSIGNED);
	test_keywords("struct ", TK_STRUCT);
	test_keywords("union ", TK_UNION);
	test_keywords("enum ", TK_ENUM);
	test_keywords("break ", TK_BREAK);
	test_keywords("case ", TK_CASE);
	test_keywords("continue ", TK_CONTIUE);
	test_keywords("default ", TK_DEFAULT);
	test_keywords("if ", TK_IF);
	test_keywords("else ", TK_ELSE);
	test_keywords("do ", TK_DO);
	test_keywords("for ", TK_FOR);
	test_keywords("while ", TK_WHILE);
	test_keywords("goto ", TK_GOTO);
	test_keywords("return ", TK_RETURN);
	test_keywords("switch ", TK_SWITCH);
}





CU_TestInfo lex_test_arrray[] = {
	{"is_dec_num:", test_is_dec_num},
	{"scan_identifier", test_scan_identifier},
	CU_TEST_INFO_NULL,
};

