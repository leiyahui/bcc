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
static char *trim_str_end(char* ident)
{
	int old_len;
	char *new_str;

	old_len = strlen(ident);
	new_str = (char*)bcc_malloc(old_len);
	bcc_strncpy(new_str, ident, old_len - 1);
	new_str[old_len] = '\0';
	
	return new_str;
}

static void test_identifier(char *ident)
{
	char *new_str;

	G_CURSOR = ident;
	scan_identifier();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_IDENTIFIER);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
	new_str = trim_str_end(ident);
	CU_ASSERT_STRING_EQUAL(g_current_token.token_value.ptr, new_str);
	bcc_free(new_str);
}

static void test_keywords(char *ident, int tk_kind)
{
	G_CURSOR = ident;
	scan_identifier();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, tk_kind);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

void test_scan_identifier(void)
{
	test_identifier("test_ident ");
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

/*test scan number*/
static void test_scan_floating(char *f_str, int tk_kind)
{
	G_CURSOR = f_str;
	scan_number();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, tk_kind);
	CU_ASSERT_DOUBLE_EQUAL(g_current_token.token_value.double_num, strtod(f_str, NULL), 0.001);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_scan_integer(char *i_str, int tk_kind, int base)
{
	G_CURSOR = i_str;
	scan_number();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, tk_kind);
	CU_ASSERT_EQUAL(g_current_token.token_value.long_num, strtol(i_str, NULL, base));
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

void test_scan_number(void)
{
	/*unsuffixed floating number*/
	test_scan_floating("129.0 ", TK_DOUBLECONST);
	test_scan_floating("1234.e2 ", TK_DOUBLECONST);
	test_scan_floating("1234.e+5 ", TK_DOUBLECONST);
	test_scan_floating("1234.5e+5 ", TK_DOUBLECONST);
	test_scan_floating("1234e5 ", TK_DOUBLECONST);
	test_scan_floating("1234e-5 ", TK_DOUBLECONST);

	/*suffixed floating number*/
	test_scan_floating("1234.0e5f ", TK_FLOATCONST);
	test_scan_floating("1234.5F ", TK_FLOATCONST);
	test_scan_floating("1234.6l ", TK_LDOUBLECONST);
	test_scan_floating("1234e5L ", TK_LDOUBLECONST);


	/*unsuffixed integer number*/
	test_scan_integer("123 ", TK_INTCONST, 10);
	test_scan_integer("2147483647 ", TK_INTCONST, 10);
	test_scan_integer("2147483648 ", TK_LONGCONST, 10);
	//test_scan_integer("9223372036854775807 ", TK_UNSIGNED_LONGCONST, 10);
	test_scan_integer("017777777777 ", TK_INTCONST, 8);
	test_scan_integer("0x7FFFFFFF ", TK_INTCONST, 16);
	test_scan_integer("037777777777 ", TK_UNSIGNED_INTCONST, 8);
	test_scan_integer("0xFFFFFFFF ", TK_UNSIGNED_INTCONST, 16);
	test_scan_integer("0777777777777777777777 ", TK_LONGCONST, 8);
	test_scan_integer("0x7FFFFFFFFFFFFFFF ", TK_LONGCONST, 16);
	//test_scan_integer("01777777777777777777777 ", TK_UNSIGNED_LONGCONST, 8);
	//test_scan_integer("0x8FFFFFFFFFFFFFFF ", TK_UNSIGNED_LONGCONST, 16);

	/*suffixed integer number*/
	test_scan_integer("123u ", TK_UNSIGNED_INTCONST, 10);
	test_scan_integer("4294967295u ", TK_UNSIGNED_INTCONST, 10);
	test_scan_integer("037777777777u ", TK_UNSIGNED_INTCONST, 8);
	test_scan_integer("0xFFFFFFFFu ", TK_UNSIGNED_INTCONST, 16);
	test_scan_integer("4294967296u ", TK_UNSIGNED_LONGCONST, 10);
	test_scan_integer("047777777777u ", TK_UNSIGNED_LONGCONST, 8);
	test_scan_integer("0x1FFFFFFFFu ", TK_UNSIGNED_LONGCONST, 16);

	test_scan_integer("123l ", TK_LONGCONST, 10);
	test_scan_integer("4294967296l ", TK_LONGCONST, 10);
	test_scan_integer("047777777777l ", TK_LONGCONST, 8);
	test_scan_integer("0x1FFFFFFFFu ", TK_UNSIGNED_LONGCONST, 16);
	
	test_scan_integer("4294967296lu ", TK_UNSIGNED_LONGCONST, 10);
	test_scan_integer("047777777777ul ", TK_UNSIGNED_LONGCONST, 8);
	test_scan_integer("0x1FFFFFFFFul ", TK_UNSIGNED_LONGCONST, 16);
}

/*test scan character*/

void test_character(char *str)
{
	G_CURSOR = str;
	scan_character();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_INTCONST);
	CU_ASSERT_EQUAL(g_current_token.token_value.int_num, *(str +  1));
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

void test_escape_character(char *str)
{
	G_CURSOR = str;
	scan_character();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_INTCONST);
	CU_ASSERT_EQUAL(g_current_token.token_value.int_num, trans_simple_escape_sequence_to_ascii(*(str + 2)));
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

void test_complicated_character(char *str, int num)
{
	G_CURSOR = str;
	scan_character();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_INTCONST);
	CU_ASSERT_EQUAL(g_current_token.token_value.int_num, num);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

void test_scan_character()
{
	test_character("'a' ");
	test_escape_character("'\\a' ");
	test_escape_character("'\\\\' ");
	test_escape_character("'\\\'' ");
	test_escape_character("'\\n' ");

	test_complicated_character("'ab' ", 24930);
	test_complicated_character("'abcd' ", 1633837924);
	test_complicated_character("'\\123' ", 83);
	test_complicated_character("'\\123\\123' ", 21331);
	test_complicated_character("'\\123\\123a' ", 5460833);
	test_complicated_character("'\\255' ", -83);
	test_complicated_character("'\\xab' ", -85);
	test_complicated_character("'\\xabc' ", -68);
}

CU_TestInfo lex_test_arrray[] = {
	{"is_dec_num:", test_is_dec_num},
	{"scan_identifier", test_scan_identifier},
	{"scan_number", test_scan_number},
	{"scan_character", test_scan_character},
	CU_TEST_INFO_NULL,
};

