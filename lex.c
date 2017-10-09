#include "bcc.h"

lex_scan_func g_scanner[255];
keyword_ele_t g_keywords_hashtable[100];
hashtable_t g_identify_hashtable;		//this hash table include identifier and string constant
token_t g_current_token;

BOOL is_dec_num(int ascii_value)
{
	if (ascii_value >= '0' && ascii_value <= '9') {
		return TRUE;
	}
	return FALSE;
}

BOOL is_letter(int ascii_value)
{
	if (ascii_value >= 'a' && ascii_value <= 'z'
		|| ascii_value >= 'A' && ascii_value <= 'Z') {
			return TRUE;
	}
	return FALSE;
}

BOOL is_underline(int ascii_value)
{
	if (ascii_value == '_') {
		return TRUE;
	}
	return FALSE;
}

BOOL is_valid_nondigit(int ascii_value)
{
	if (is_letter(ascii_value) || is_underline(ascii_value)) {
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
		if (is_valid_nondigit(ascii_value)) {
			g_scanner[ascii_value] = scan_identifier;
		} else if (is_dec_num(ascii_value)) {
			g_scanner[ascii_value] = scan_number;
		} else if(is_file_end(ascii_value)) {
			g_scanner[ascii_value] = scan_file_end;
		} else {
			g_scanner[ascii_value] = scan_bad_letter;
		}
	}

	g_scanner['"'] = scan_string_literal;
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

BOOL is_scan_end(char letter)
{
	if (letter == ' ' || letter == '\r' || letter == '\t') {
		return TRUE;
	}
	return FALSE;
}

/*identity contains keywords and varible name or function name that user declared*/
void scan_identifier()
{
	char *base_ptr;
	char *identify_ptr;
	int str_len, tk_kind;

	base_ptr = G_CURSOR;

	if (*G_CURSOR == 'L' || *(G_CURSOR + 1) == '\'') {
		scan_w_character();
		return;
	}

	if (*G_CURSOR == 'L' || *(G_CURSOR + 1) == '\"') {
		scan_w_str_literal();
		return;
	}

	while (is_valid_nondigit(*G_CURSOR) || is_dec_num(*G_CURSOR)) {
		G_CURSOR++;
	}

	str_len = G_CURSOR - base_ptr;
	tk_kind = lookup_keywords(base_ptr, str_len);

	if (!tk_kind) {				//not keywords
		identify_ptr = lookup_hash(&g_identify_hashtable, base_ptr, str_len);
		if (!identify_ptr) {
			identify_ptr = insert_hash(&g_identify_hashtable, base_ptr, str_len);
		}
		tk_kind = TK_IDENTIFIER;
		g_current_token.token_value.ptr = identify_ptr; 
	}
	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
	
}

/*contain octal decimal hex*/ 

#define SIGNED_CHAR_MAX				127
#define SIGNED_CHAR_MIN				-128

#define SIGNED_SHORT_MAX			32767
#define SIGNED_SHORT_MIN			-32768
#define UNSIGNED_SHORT_MAX			65535
#define UNSIGNED_SHORT_MIN			0

#define SIGNED_INT_MAX				2147483647
#define SIGNED_INT_MIN				-2147483648
#define UNSIGNED_INT_MAX			4294967295
#define UNSIGNED_INT_MIN			0

#define SIGNED_LONG_MAX				9223372036854775807
#define SIGNED_LONG_MIN				-9223372036854775807
#define UNSIGNED_LONG_MAX			18446744073709551615
#define UNSIGNED_LONG_MIN			0


BOOL is_dot(int ascii)
{
	if (ascii == '.') {
		return TRUE;
	}
	return FALSE;
}

BOOL is_e_or_E(int ascii)
{
	if (ascii == 'e' || ascii == 'E') {
		return TRUE;
	}
	return FALSE;
}

BOOL is_sign(int ascii)
{
	if (ascii == '+' || ascii == '-') {
		return TRUE;
	}
	return FALSE;
}

BOOL is_octal_number(int ascii)
{
	if (ascii >= '0' && ascii <= '8') {
		return TRUE;
	}
	return FALSE;
}

BOOL is_hex_number(int ascii)
{
	if (ascii >= '0' && ascii <= '9' ||
		ascii >= 'A' && ascii <= 'F' ||
		ascii >= 'a' && ascii <= 'f') {
			return TRUE;
	}
	return FALSE;
}

BOOL is_floating(char *str)
{
	char *curr_ptr;

	curr_ptr = str;
	if (curr_ptr[0] == '0' && curr_ptr[1] == 'x' || curr_ptr[1] == 'X') {		//hex number
		return FALSE;
	}

	while (is_dec_num(*curr_ptr)) {
		curr_ptr++;
	}
	if (is_dot(*curr_ptr) || is_e_or_E(*curr_ptr)) {				//floating
		return TRUE;
	}
	return FALSE;
}

/*
floating-constant:
fractional-constant exponent-part<opt> floating-suffix<opt>
digit-sequence exponent-part floating-suffix<opt>

fractional-constant:
digit-sequence<opt>.digit-sequence
digit-sequence.

exponent-part:
e  sign<opt> digit-sequence
E  sign<opt> digit-sequence

sign: one of
+  -

digit-sequence:
digit
digit-sequence digit

floating-suffix: one of
f  l  F  L
*/

void scan_exponent_part()
{
	G_CURSOR++;
	if (is_sign(*G_CURSOR)){
		G_CURSOR++;
	}
	while (is_dec_num(*G_CURSOR)) {
		G_CURSOR++;
	}
}

/* An unsuffixed floating constant has type double.  If suffixed by
the letter f or F, it has type float.  If suffixed by the letter l
or L, it has type long double*/

int scan_floating_suffix()
{
	int tk_kind;

	switch (*G_CURSOR) {
	case 'f':
	case 'F':
		G_CURSOR++;
		tk_kind = TK_FLOATCONST;
		break;
	case 'l':
	case 'L':
		G_CURSOR++;
		tk_kind = TK_LDOUBLECONST;
		break;
	default:
		tk_kind = TK_DOUBLECONST;
		break;
	}
	return tk_kind;
}

void scan_floating()
{
	char *base_ptr;
	int tk_kind;
	double floating_value;

	base_ptr = G_CURSOR;
	while (is_dec_num(*G_CURSOR)) {
		G_CURSOR++;
	}

	if (is_dot(*G_CURSOR)) {		//with fraction
		G_CURSOR++;
		while (is_dec_num(*G_CURSOR)) {
			G_CURSOR++;
		}
		if (is_e_or_E(*G_CURSOR)) {
			scan_exponent_part();
		}
	} else if(is_e_or_E(*G_CURSOR)) {		//without fraction
		scan_exponent_part();
	} else {
		ERROR("invalid character in floating constant");
	}
	
	tk_kind = scan_floating_suffix();
	
	floating_value = strtod(base_ptr, NULL);
	if (errno == ERANGE) {
		WARN("overflow in implicit constant conversion");
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
	g_current_token.token_value.double_num = floating_value;
}

#define WITH_UNSIGNED_SUFFIX	0x0001
#define WITH_LONG_SUFFIX		0x0002

int scan_integer_suffix()
{
	int suffix_flag;

	suffix_flag = 0;

	switch (*G_CURSOR) {
	case 'u':
	case 'U':
		suffix_flag |= WITH_UNSIGNED_SUFFIX;
		G_CURSOR++;
		break;
	case 'l':
	case 'L':
		suffix_flag |= WITH_LONG_SUFFIX;
		G_CURSOR++;
		break;
	default:
		return suffix_flag;
	}

	switch (*G_CURSOR) {
	case 'u':
	case 'U':
		if (suffix_flag & WITH_UNSIGNED_SUFFIX) {
			ERROR("invalid suffix in integer");
		}
		suffix_flag |= WITH_UNSIGNED_SUFFIX;
		G_CURSOR++;
		break;
	case 'l':
	case 'L':
		if (suffix_flag & WITH_LONG_SUFFIX) {
			ERROR("invalid suffix in integer");
		}
		suffix_flag |= WITH_LONG_SUFFIX;
		G_CURSOR++;
		break;
	default:
		return suffix_flag;
	}

	return suffix_flag;
}

void scan_decimal_number()
{
	while (is_dec_num(*G_CURSOR)) {
		G_CURSOR++;
	}
}

void scan_octal_number()
{
	G_CURSOR++;
	while (is_octal_number(*G_CURSOR)) {
		G_CURSOR++;
	}
}

void scan_hex_number()
{
	G_CURSOR += 2;
	while (is_hex_number(*G_CURSOR)) {
		G_CURSOR++;
	}
}

/*
The type of an integer constant is the first of the corresponding list in which its value can be represented.  
Unsuffixed decimal: int, long int, unsigned long int; 
unsuffixed octal or hexadecimal: int, unsigned int, long int, unsigned long int; 
suffixed by the letter u or U: unsigned int, unsigned long int; 
suffixed by the letter l or L: long int, unsigned long int; 
suffixed by both the letters u or U and l or L: unsigned long int
*/
int get_integer_tk_kind(int suffix_flag,int base, long int integer_value)
{
	int tk_kind;
	if (suffix_flag == 0) {							//without suffix
		if (base == 10) {
			if (integer_value <= SIGNED_INT_MAX) {
				tk_kind = TK_INTCONST;
			} else if (integer_value <= SIGNED_LONG_MAX) {
				tk_kind = TK_LONGCONST;
			} else {
				tk_kind = TK_UNSIGNED_LONGCONST;
			}
		} else {
			if (integer_value <= SIGNED_INT_MAX) {
				tk_kind = TK_INTCONST;
			} else if (integer_value <= UNSIGNED_INT_MAX) {
				tk_kind = TK_UNSIGNED_INTCONST;
			} else if (integer_value <= SIGNED_LONG_MAX) {
				tk_kind = TK_LONGCONST;
			} else {
				tk_kind = TK_UNSIGNED_LONGCONST;
			}
		}
	} else if (suffix_flag & WITH_UNSIGNED_SUFFIX 
		&& !(suffix_flag & WITH_LONG_SUFFIX)) {
			if (integer_value <= UNSIGNED_INT_MAX) {
				tk_kind = TK_UNSIGNED_INTCONST;
			} else {
				tk_kind = TK_UNSIGNED_LONGCONST;
			}
	} else if (!(suffix_flag & WITH_UNSIGNED_SUFFIX)
		&& suffix_flag & WITH_LONG_SUFFIX) {
			if (integer_value <= SIGNED_LONG_MAX) {
				tk_kind = TK_LONGCONST;
			} else {
				tk_kind = TK_UNSIGNED_LONGCONST;
			}
	} else {
		tk_kind = TK_UNSIGNED_LONGCONST;
	}
	return tk_kind;
}

void scan_integer()
{
	char *base_ptr;
	long int integer_value;
	int base, suffix_flag;
	int tk_kind;

	base_ptr = G_CURSOR;
	if (base_ptr[0] != '0') {										/*10 binary*/
		scan_decimal_number();		
		base = 10;
	} else if (base_ptr[1] == 'x' || base_ptr[1] == 'X') {			/*8 binary*/
		scan_hex_number();
		base = 16;
	} else {														/*16 binary*/
		scan_octal_number();
		base = 8;
	}
	
	suffix_flag = 0;
	suffix_flag = scan_integer_suffix();

 	integer_value = strtol(base_ptr, NULL, base);
	if (errno == ERANGE) {
		WARN("overflow in implicit constant conversion");
	}

	tk_kind = get_integer_tk_kind(suffix_flag, base, integer_value);

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
	g_current_token.token_value.long_num = integer_value;
}

void scan_number()
{
	if (is_floating(G_CURSOR)) {
		scan_floating();
	} else {
		scan_integer();
	}
}

/*string or character*/

/*For example, 'ab' for a target with an 8-bit char would be interpreted as ¡®(int)((unsigned char) 'a' * 256 + (unsigned char) 'b')¡¯, and '\234a' as ¡®(int) ((unsigned char) '\234' * 256 + (unsigned char) 'a')¡¯.*/

int turn_ascii_to_num(int ascii_value)
{
	if (ascii_value >= '0' && ascii_value <= '9') {
		return ascii_value - 48;	
	} else if (ascii_value >= 'A' && ascii_value <= 'F') {
		return ascii_value - 55;
	} else {
		return ascii_value - 87;
	}
}

static unsigned char scan_character_hex()			//most two Bit hex
{
	unsigned char hex_value;

	if (!is_hex_number(*G_CURSOR)) {
		ERROR("\\x used hex without hex digits");
	}
	hex_value = turn_ascii_to_num(*G_CURSOR);

	G_CURSOR++;
	while (is_hex_number(*G_CURSOR)) {
		hex_value <<= 4;
		hex_value += turn_ascii_to_num(*G_CURSOR);
		G_CURSOR++;
	}

	return hex_value;
}

static unsigned char scan_character_oct()		//most three Bit oct
{
	unsigned char oct_value;

	oct_value = turn_ascii_to_num(*G_CURSOR);

	G_CURSOR++;
	if (is_octal_number(*G_CURSOR)) {
		oct_value <<= 3;
		oct_value += turn_ascii_to_num(*G_CURSOR);
	}

	G_CURSOR++;
	if (is_octal_number(*G_CURSOR)) {
		oct_value <<= 3;
		oct_value += turn_ascii_to_num(*G_CURSOR);
		G_CURSOR++;
	}

	return oct_value;
}

static BOOL is_hex_escape_sequce(unsigned char character) {
	if (character == 'x') {
		return TRUE;
	}
	return FALSE;
}

static BOOL is_simple_escape_sequence(unsigned char character) {
	if (character == '\''
		|| character == '"'
		|| character == '?'
		|| character == '\\'
		|| character == 'a'
		|| character == 'b'
		|| character == 'f'
		|| character == 'n'
		|| character == 'r'
		|| character == 't'
		|| character == 'v') {
		return TRUE;
	}
	return FALSE;
}

char trans_simple_escape_sequence_to_ascii(unsigned char character)
{
	unsigned char ret_char;

	if (character == '\'' 
		|| character == '\\'
		|| character == '"'
		|| character == '?') {
		ret_char = character;
	} else if (character == 'a') {
		ret_char = '\a';
	} else if (character == 'b') {
		ret_char = '\b';
	} else if (character == 'f') {
		ret_char = '\f';
	} else if (character == 'n') {
		ret_char = '\n';
	} else if (character == 'r') {
		ret_char = '\r';
	} else if (character == 't') {
		ret_char = '\t';
	} else if (character == 'v') {
		ret_char = '\v';
	} else {
		ERROR("unknown escape sequence: %c");
	}
	return ret_char;
}

static char scan_one_character()
{
	char ret_char;

	if (*G_CURSOR == '\\') {			//escape sequence
		G_CURSOR++;
		if (is_hex_escape_sequce(*G_CURSOR)) {			//hex escape sequence
			G_CURSOR++;						//jump 'x'
			ret_char = scan_character_hex();
		} else if (is_octal_number(*G_CURSOR)) {  //oct escape sequence
			ret_char = scan_character_oct();
		} else {
			ret_char = trans_simple_escape_sequence_to_ascii(*G_CURSOR);  //simple escape sequence
			G_CURSOR++;
		}
	} else if (*G_CURSOR >= '!' && *G_CURSOR <= '~') {
		ret_char = *G_CURSOR;
		if (ret_char == '\'') {
			ERROR("empty character constant");
		}
		G_CURSOR++;
	} else {
		ERROR("invalid character constant");
	}
	return ret_char;
}

void scan_character()
{
	char temp_character;
	int character_value;

	G_CURSOR++;

	temp_character = scan_one_character();
	character_value = temp_character;

	while (*G_CURSOR != '\'') {
		temp_character = scan_one_character();
		character_value <<= 8;
		character_value += temp_character;
	}

	G_CURSOR++;
	g_current_token.tk_kind = TK_INTCONST;
	g_current_token.token_value.int_num = character_value;
	g_current_token.line = G_LINE;
}


char scan_one_str_character()
{
	char ret_char;

	if (*G_CURSOR == '\\') {
		G_CURSOR++;
		ret_char = trans_simple_escape_sequence_to_ascii(*G_CURSOR);  //simple escape sequence
		G_CURSOR++;
	} else if (*G_CURSOR >= '!' && *G_CURSOR <= '~') {
		ret_char = *G_CURSOR;
		G_CURSOR++;
	} else {
		ERROR("invalid character constant");
	}
	return ret_char;
}

#define DEFAULT_STR_BUFFER_SIZE 10
void scan_string_literal()
{
	unsigned char temp_character;
	char* str_ptr, *ptr_in_hash;
	int curr_buffer_size, str_len;

	curr_buffer_size = DEFAULT_STR_BUFFER_SIZE;
	str_ptr = (char*)bcc_malloc(curr_buffer_size);
	G_CURSOR++;

	str_len = 0;
	while (*G_CURSOR != '"') {
		temp_character = scan_one_str_character();
		str_len++;
		if (str_len >= curr_buffer_size) {
			curr_buffer_size *= 2;
			str_ptr = bcc_realloc(str_ptr, curr_buffer_size);
		}
		str_ptr[str_len - 1] = temp_character;
	}
	G_CURSOR++;
	str_ptr[str_len] = '\0';

	ptr_in_hash = lookup_hash(&g_identify_hashtable, str_ptr, str_len);
	if (!ptr_in_hash) {
		ptr_in_hash = insert_hash(&g_identify_hashtable, str_ptr, str_len);
	}
	bcc_free(str_ptr);
	
	g_current_token.line = G_LINE;
	g_current_token.tk_kind = TK_STRING;
	g_current_token.token_value.ptr = ptr_in_hash;
}

void scan_w_character()
{

}

void scan_w_str_literal()
{

}

void scan_comma()
{
	G_CURSOR++;
	g_current_token.line = G_LINE;
	g_current_token.tk_kind = TK_COMMA;
}

void scan_question_mark()
{
	G_CURSOR++;
	g_current_token.line = G_LINE;
	g_current_token.tk_kind = TK_QUESTION;
}

void scan_colon()
{
	G_CURSOR++;
	g_current_token.line = G_LINE;
	g_current_token.tk_kind = TK_COLON;
}

void scan_equal_sign()
{
	int tk_kind;

	tk_kind = TK_ASSIGN;

	G_CURSOR++;
	if (*G_CURSOR == '=') {
		tk_kind = TK_EQUAL;
		G_CURSOR++;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_and()
{
	int tk_kind;

	G_CURSOR++;
	if (*G_CURSOR == '=') {
		tk_kind = TK_BITAND_ASSIGN;
		G_CURSOR++;
	} else if (*G_CURSOR == '&') {
		tk_kind = TK_AND;
		G_CURSOR++;
	} else {
		tk_kind = TK_BITAND;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_or()
{
	int tk_kind;

	G_CURSOR++;
	if (*G_CURSOR == '=') {
		tk_kind = TK_BITOR_ASSIGN;
		G_CURSOR++;
	} else if (*G_CURSOR == '|') {
		tk_kind = TK_OR;
		G_CURSOR++;
	} else {
		tk_kind = TK_BITOR;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_less()
{
	int tk_kind;

	G_CURSOR++;
	if (*G_CURSOR == '<') {
		tk_kind = TK_LSHIFT;
		G_CURSOR++;
		if (*G_CURSOR == '=') {
			tk_kind = TK_LSHIFT_ASSIGN;
			G_CURSOR++;
		}
	} else if (*G_CURSOR == '=') {
		tk_kind = TK_LESS_EQUAL;
		G_CURSOR++;
	} else {
		tk_kind = TK_LESS;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_great()
{
	int tk_kind;

	G_CURSOR++;
	if (*G_CURSOR == '>') {
		tk_kind = TK_RSHIFT;
		G_CURSOR++;
		if (*G_CURSOR == '=') {
			tk_kind = TK_RSHIFT_ASSIGN;
			G_CURSOR++;
		}
	} else if (*G_CURSOR == '=') {
		tk_kind = TK_GREAT_EQUAL;
		G_CURSOR++;
	} else {
		tk_kind = TK_GREAT;
	}
	
	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_add()
{
	int tk_kind;

	G_CURSOR++;
	if (*G_CURSOR == '+') {
		tk_kind = TK_INC;
		G_CURSOR++;
	} else if (*G_CURSOR == '=') {
		tk_kind = TK_ADD_ASSING;
		G_CURSOR++;
	} else {
		tk_kind = TK_ADD;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_minus()
{
	int tk_kind;

	G_CURSOR++;
	if (*G_CURSOR == '-') {
		tk_kind = TK_DEC;
		G_CURSOR++;
	} else if (*G_CURSOR == '=') {
		tk_kind = TK_SUB_ASSIGN;
		G_CURSOR++;
	} else if (*G_CURSOR == '>') {
		tk_kind = TK_POINTER;
		G_CURSOR++;
	} else {
		tk_kind = TK_SUB;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_multi()
{
	int tk_kind;

	tk_kind = TK_MULTIPLY;

	G_CURSOR++;
	if (*G_CURSOR == '=') {
		tk_kind = TK_MULTI_ASSIGN;
		G_CURSOR++;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_divide()
{
	int tk_kind;

	tk_kind = TK_DIVIDE;

	G_CURSOR++;
	if (*G_CURSOR == '=') {
		tk_kind = TK_DIVIDE_ASSIGN;
		G_CURSOR++;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_percent()
{
	int tk_kind;

	tk_kind = TK_MOD;

	G_CURSOR++;
	if (*G_CURSOR == '=') {
		tk_kind = TK_MOD_ASSIGN;
		G_CURSOR++;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_caret()
{
	int tk_kind;

	tk_kind = TK_BITXOR;

	G_CURSOR++;
	if (*G_CURSOR == '=') {
		tk_kind = TK_BITXOR_ASSIGN;
		G_CURSOR++;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_comp()
{
	G_CURSOR++;
	g_current_token.tk_kind = TK_BITREVERT;
	g_current_token.line = G_LINE;
}

void scan_dot()
{
	int tk_kind;

	if (!bcc_strnequal(G_CURSOR, "...", 3)) {
		tk_kind = TK_DOT;
		G_CURSOR += 1;
	} else {
		tk_kind = TK_ELLIPSE;
		G_CURSOR += 3;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_lparen()
{
	G_CURSOR++;
	g_current_token.tk_kind = TK_LPAREN;
	g_current_token.line = G_LINE;
}

void scan_rparen()
{
	G_CURSOR++;
	g_current_token.tk_kind = TK_RPAREN;
	g_current_token.line = G_LINE;
}

void scan_lbrace()
{
	G_CURSOR++;
	g_current_token.tk_kind = TK_LBRACE;
	g_current_token.line = G_LINE;
}

void scan_rbrace()
{
	G_CURSOR++;
	g_current_token.tk_kind = TK_RBRACE;
	g_current_token.line = G_LINE;
}

void scan_lbracket()
{
	G_CURSOR++;
	g_current_token.tk_kind = TK_LBRACKET;
	g_current_token.line = G_LINE;
}

void scan_rbracket()
{
	G_CURSOR++;
	g_current_token.tk_kind = TK_RBRACKET;
	g_current_token.line = G_LINE;
}

void scan_semicolon()
{
	G_CURSOR++;
	g_current_token.tk_kind = TK_SEMICOLON;
	g_current_token.line = G_LINE;
}

void scan_exclamation()
{
	int tk_kind;

	tk_kind = TK_NOT;

	G_CURSOR++;
	if (*G_CURSOR == '=') {
		tk_kind = TK_NEQUAL;
		G_CURSOR++;
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
}

void scan_file_end()
{
	g_current_token.tk_kind = TK_END;
	g_current_token.line = G_LINE;
}

void scan_bad_letter()
{
	ERROR("illegal character");
}

void get_next_token()
{
	g_scanner[*G_CURSOR]();
}
