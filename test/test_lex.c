#include "../bcc.h"

int lex_suite_init()
{
	init_keywords_hash();
	init_hashtable(&g_identify_hashtable, NULL);
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
	new_str[old_len - 1] = '\0';
	
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
static void test_character(char *str)
{
	G_CURSOR = str;
	scan_character();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_INTCONST);
	CU_ASSERT_EQUAL(g_current_token.token_value.int_num, *(str +  1));
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_escape_character(char *str)
{
	G_CURSOR = str;
	scan_character();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_INTCONST);
	CU_ASSERT_EQUAL(g_current_token.token_value.int_num, trans_simple_escape_sequence_to_ascii(*(str + 2)));
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_complicated_character(char *str, int num)
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
	test_escape_character("'\\'' ");

	test_complicated_character("'ab' ", 24930);
	test_complicated_character("'abcd' ", 1633837924);
	test_complicated_character("'abcde' ", 1650680933);
	test_complicated_character("'\\123' ", 83);
	test_complicated_character("'\\123\\123' ", 21331);
	test_complicated_character("'\\123\\123a' ", 5460833);
	test_complicated_character("'\\255' ", -83);
	test_complicated_character("'\\xab' ", -85);
	test_complicated_character("'\\xabc' ", -68);
	//test_complicated_character("'\\xabg' ", 43879);			//the action is different from gcc
}

/*test scan string*/
static char *turn_str_to_ident(char *str)
{
	int old_len, new_len, i;
	char *ident;
	char tmp_value;

	old_len = bcc_strlen(str);
	ident = (char*)bcc_malloc(old_len - 2);
	new_len = 0;
	for (i = 0; i < old_len - 3; i++) {
		tmp_value = *(str + i + 1);
		if (tmp_value == '\\') {
			tmp_value = trans_simple_escape_sequence_to_ascii(*(str + i + 2));
			i++;
		}
		ident[new_len] = tmp_value;
		new_len++;
	}
	ident[new_len] = '\0';
	
	return ident;
}

static void test_str_literal(char *str)
{
	char *ident;

	G_CURSOR = str;
	scan_string_literal();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_STRING);
	ident = turn_str_to_ident(str);
	CU_ASSERT_STRING_EQUAL(g_current_token.token_value.ptr, ident);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
	bcc_free(ident);
}

void test_scan_string_literal()
{
	test_str_literal("\"\" ");
	test_str_literal("\"a\" ");
	test_str_literal("\"abcdfe\" ");
	test_str_literal("\"\\n\" ");
	test_str_literal("\"\\t\" ");
	test_str_literal("\"\\\"\" ");
	test_str_literal("\"\\'\" ");
}

/*test scan special character*/

static void test_scan_comma()
{
	G_CURSOR = ", ";
	scan_comma();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_COMMA);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_scan_question_mark()
{
	G_CURSOR = "?  ";
	scan_question_mark();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_QUESTION);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_scan_colon()
{
	G_CURSOR = ": ";
	scan_colon();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_COLON);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_scan_comp()
{
	G_CURSOR = "~ ";
	scan_comp();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_BITREVERT);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_scan_paren()
{
	G_CURSOR = "( ";
	scan_lparen();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_LPAREN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = ") ";
	scan_rparen();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_RPAREN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_scan_brace()
{
	G_CURSOR = "{ ";
	scan_lbrace();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_LBRACE);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "} ";
	scan_rbrace();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_RBRACE);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_scan_bracket()
{
	G_CURSOR = "[ ";
	scan_lbracket();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_LBRACKET);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "] ";
	scan_rbracket();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_RBRACKET);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

static void test_scan_semicolon()
{
	G_CURSOR = "; ";
	scan_semicolon();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_SEMICOLON);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

void test_scan_special_character()
{
	test_scan_comma();
	test_scan_question_mark();
	test_scan_colon();
	test_scan_comp();
	test_scan_paren();
	test_scan_brace();
	test_scan_bracket();
	test_scan_semicolon();
}

/*test scan equal sign*/
void test_scan_euqal_sign()
{
	G_CURSOR = "= ";
	scan_equal_sign();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "== ";
	scan_equal_sign();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_EQUAL);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan and sign*/

void test_scan_and()
{
	G_CURSOR = "& ";
	scan_and();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_BITAND);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "&& ";
	scan_and();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_AND);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "&= ";
	scan_and();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_BITAND_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan or sign*/
void test_scan_or()
{
	G_CURSOR = "| ";
	scan_or();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_BITOR);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "|| ";
	scan_or();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_OR);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "|= ";
	scan_or();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_BITOR_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan less sign*/
void test_scan_less()
{
	G_CURSOR = "< ";
	scan_less();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_LESS);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "<< ";
	scan_less();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_LSHIFT);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "<<= ";
	scan_less();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_LSHIFT_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "<= ";
	scan_less();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_LESS_EQUAL);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan great sign*/
void test_scan_great()
{
	G_CURSOR = "> ";
	scan_great();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_GREAT);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = ">> ";
	scan_great();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_RSHIFT);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = ">>= ";
	scan_great();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_RSHIFT_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = ">= ";
	scan_great();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_GREAT_EQUAL);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan add sign*/
void test_scan_add()
{
	G_CURSOR = "+ ";
	scan_add();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_ADD);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "++ ";
	scan_add();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_INC);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "+= ";
	scan_add();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_ADD_ASSING);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan minus sign*/
void test_scan_minus()
{
	G_CURSOR = "- ";
	scan_minus();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_SUB);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "-= ";
	scan_minus();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_SUB_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "-- ";
	scan_minus();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_DEC);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "-> ";
	scan_minus();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_POINTER);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan multi sign*/
void test_scan_multi()
{
	G_CURSOR = "* ";
	scan_multi();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_MULTIPLY);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "*= ";
	scan_multi();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_MULTI_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan divide muliti*/
void test_scan_divide()
{
	G_CURSOR = "/ ";
	scan_divide();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_DIVIDE);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "/= ";
	scan_divide();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_DIVIDE_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan percent sign*/
void test_scan_percent()
{
	G_CURSOR = "% ";
	scan_percent();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_MOD);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "%= ";
	scan_percent();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_MOD_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan caret sign*/
void test_scan_caret()
{
	G_CURSOR = "^ ";
	scan_caret();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_BITXOR);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = "^= ";
	scan_caret();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_BITXOR_ASSIGN);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

/*test scan dot sign*/
void test_scan_dot()
{
	G_CURSOR = ". ";
	scan_dot();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_DOT);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');

	G_CURSOR = ".. ";
	scan_dot();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_DOT);
	CU_ASSERT_EQUAL(*G_CURSOR, '.');

	G_CURSOR = "... ";
	scan_dot();
	CU_ASSERT_EQUAL(g_current_token.tk_kind, TK_ELLIPSE);
	CU_ASSERT_EQUAL(*G_CURSOR, ' ');
}

CU_TestInfo lex_test_arrray[] = {
	{"is_dec_num:", test_is_dec_num},
	{"scan_identifier", test_scan_identifier},
	{"scan_number", test_scan_number},
	{"scan_character", test_scan_character},
	{"scan_string", test_scan_string_literal},
	{"scan_special_character", test_scan_special_character},
	{"scan_equal_sign", test_scan_euqal_sign},
	{"scan_and", test_scan_and},
	{"scan_or", test_scan_or},
	{"scan_less", test_scan_less},
	{"scan_great", test_scan_great},
	{"scan_add", test_scan_add},
	{"scan_minus", test_scan_minus},
	{"scan_multi", test_scan_multi},
	{"scan_divide", test_scan_divide},
	{"scan_percent", test_scan_percent},
	{"scan_caret", test_scan_caret},
	{"scan_dot", test_scan_dot},
	CU_TEST_INFO_NULL,
};

