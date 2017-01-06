#include "bcc.h"

lex_scan_func g_scanner_func[255];
token_t current_token;

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
			g_scanner_func[ascii_value] = scan_identity;
		} else if (is_digit(ascii_value)) {
			g_scanner_func[ascii_value] = scan_number;
		} else if(is_file_end(ascii_value)) {
			g_scanner_func[ascii_value] == scan_file_end;
		} else {
			g_scanner_func[ascii_value] = scan_bad_letter;
		}
	}

	g_scanner_func['"'] = scan_string;
	g_scanner_func['\''] = scan_character;
	g_scanner_func[','] = scan_comma;
	g_scanner_func['?'] = scan_question_mark;
	g_scanner_func[':'] = scan_colon;
	g_scanner_func['='] = scan_equal_sign;
	g_scanner_func['|'] = scan_or;
	g_scanner_func['&'] = scan_and;
	g_scanner_func['<'] = scan_less;
	g_scanner_func['>'] = scan_great;
	g_scanner_func['+'] = scan_add;
	g_scanner_func['-'] = scan_minus;
	g_scanner_func['*'] = scan_multi;
	g_scanner_func['/'] = scan_divide;
	g_scanner_func['%'] = scan_percent;
	g_scanner_func['^'] = scan_caret;
	g_scanner_func['~'] = scan_comp;
	g_scanner_func['.'] = scan_dot;
	g_scanner_func['('] = scan_lparen;
	g_scanner_func[')'] = scan_rparen;
	g_scanner_func['{'] = scan_lbrace;
	g_scanner_func['}'] = scan_rbrace;
	g_scanner_func['['] = scan_lbracket;
	g_scanner_func[']'] = scan_rbracket;
	g_scanner_func[';'] = scan_semicolon;
	g_scanner_func['!'] = scan_exclamation;		
}