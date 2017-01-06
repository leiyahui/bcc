#include "bcc.h"

lex_scan_func g_scanner[255];
int g_keywords_hashtable[100];
token_t g_current_token;

BOOL is_digit(int ascii_value)
{
	if (ascii_value >= 48 && ascii_value <= 57) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_letter(int ascii_value)
{
	if (ascii_value >= 65 && ascii_value <= 90
		|| ascii_value >= 97 && ascii_value <= 122) {
			return TRUE;
	}
	return FALSE;
}

BOOL is_underline(int ascii_value)
{
	if (ascii_value == 95) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_file_end(int ascii_value)
{
	if (ascii_value == 255) {
		return TRUE;
	}
	return FALSE;
}

void init_scanner()
{
	int ascii_value;

	for (ascii_value = 0; ascii_value <= 255; ascii_value++) {
		if (is_letter(ascii_value) || is_underline(ascii_value)) {
			g_scanner[ascii_value] = scan_identity;
		} else if (is_digit(ascii_value)) {
			g_scanner[ascii_value] = scan_number;
		} else if(is_file_end(ascii_value)) {
			g_scanner[ascii_value] == scan_file_end;
		} else {
			g_scanner[ascii_value] = scan_bad_letter;
		}
	}

	g_scanner['"'] = scan_string;
	g_scanner['\''] = scan_character;
	g_scanner[','] = scan_comma;
	g_scanner['?'] = scan_question_mark;
	g_scanner[':'] = scan_colon;
	g_scanner['='] = scan_equal_sign;
	g_scanner['|'] = scan_or;
	g_scanner['&'] = scan_and;
	g_scanner['<'] = scan_less;
	g_scanner['>'] = scan_great;
	g_scanner['+'] = scan_add;
	g_scanner['-'] = scan_minus;
	g_scanner['*'] = scan_multi;
	g_scanner['/'] = scan_divide;
	g_scanner['%'] = scan_percent;
	g_scanner['^'] = scan_caret;
	g_scanner['~'] = scan_comp;
	g_scanner['.'] = scan_dot;
	g_scanner['('] = scan_lparen;
	g_scanner[')'] = scan_rparen;
	g_scanner['{'] = scan_lbrace;
	g_scanner['}'] = scan_rbrace;
	g_scanner['['] = scan_lbracket;
	g_scanner[']'] = scan_rbracket;
	g_scanner[';'] = scan_semicolon;
	g_scanner['!'] = scan_exclamation;		
}

BOOL is_scan_over(char letter)
{
	if (letter == ' ' || letter == '\r' || letter == '\t') {
		return TRUE;
	}
	return FALSE;
}

BOOL find_keywords(char *ptr, int len)
{
	
}

void scan_identity()
{
	unsigned char *base_ptr, *curr_ptr;
	unsigned char curr_letter;

	base_ptr = curr_ptr = g_input_file.base;
	curr_letter = *curr_ptr;
	while (is_scan_over(curr_letter)) {
		curr_ptr++;
	}
	g_input_file.cursor = curr_ptr;
}
