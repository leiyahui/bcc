#include "bcc.h"

lex_scan_func g_scanner[255];
keyword_ele_t g_keywords_hashtable[100];
identify_hashtable_t g_identify_hashtable;
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
	if (is_letter(ascii_value) || is_valid_nondigit(ascii_value)) {
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



	while (!is_scan_end(*G_CURSOR)) {
		G_CURSOR++;
		if (!is_valid_nondigit(*G_CURSOR) && !is_dec_num(*G_CURSOR)) {
			log_error("invalid character in identifier: %s", base_ptr);
		}
	}

	str_len = G_CURSOR - base_ptr;

	tk_kind = lookup_keywords(base_ptr, str_len);

	if (!tk_kind) {				//not keywords
		identify_ptr = lookup_identify_hash(base_ptr, str_len);
		if (!identify_ptr) {
			identify_ptr = insert_identify_hash(base_ptr, str_len);
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

#define SIGNED_LONG_MAX				2147483647
#define SIGNED_LONG_MIN				-2147483648
#define UNSIGNED_LONG_MAX			4294967295
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
	if (ascii >= '0' && ascii <= '9') {
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
	if (curr_ptr[0] = '0' && curr_ptr[1] == 'x' || curr_ptr[1] == 'X') {		//hex number
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

int scan_floating_suffix()
{
	if (!is_scan_end(*G_CURSOR)) {
		if (*G_CURSOR == 'f' || *G_CURSOR == 'F') {
			return TK_FLOATCONST;
		} else if (*G_CURSOR == 'l' || *G_CURSOR == 'L') {
			return TK_LDOUBLECONST;
		} else {
			error_message("invalid suffix in floating");
		}
		G_CURSOR++;
	}
	return TK_DOUBLECONST;
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
		error_message("invalid character in floating constant");
	}
	
	tk_kind = scan_floating_suffix();
	
	floating_value = strtol(base_ptr, NULL, 0);
	if (errno = ERANGE) {
		warn_message("overflow in implicit constant conversion");
	}

	g_current_token.tk_kind = tk_kind;
	g_current_token.line = G_LINE;
	g_current_token.token_value.double_num = floating_value;
}

/*
The type of an integer constant is the first of the corresponding
list in which its value can be represented.  Unsuffixed decimal: int,
long int, unsigned long int; unsuffixed octal or hexadecimal: int,
unsigned int, long int, unsigned long int; suffixed by the letter u
or U: unsigned int, unsigned long int; suffixed by the letter l or
L: long int, unsigned long int; suffixed by both the letters u or U
and l or L: unsigned long int .s
*/

#define WITH_UNSIGNED_SUFFIX	0x0001
#define WITH_LONG_SUFFIX		0x0002

int scan_integer_suffix()
{
	int suffix_flag;

	suffix_flag = 0;

	if (!is_scan_end(*G_CURSOR)) {
		if (*G_CURSOR == 'u' || *G_CURSOR == 'U') {
			suffix_flag |= WITH_UNSIGNED_SUFFIX;
		} else if (*G_CURSOR == 'l' || *G_CURSOR == 'L') {
			suffix_flag |= WITH_LONG_SUFFIX;
		} else {
			error_message("invalid suffix in integer");
		}
		G_CURSOR++;
	}

	if (!is_scan_end(*G_CURSOR)) {
		if (*G_CURSOR == 'u' || *G_CURSOR == 'U') {
			if (suffix_flag & WITH_UNSIGNED_SUFFIX) {
				error_message("invalid suffix in integer");
			}
			suffix_flag |= WITH_UNSIGNED_SUFFIX;
		} else if (*G_CURSOR == 'l' || *G_CURSOR == 'L') {
			if (suffix_flag & WITH_LONG_SUFFIX) {
				error_message("invalid suffix in integer");
			}
			suffix_flag |= WITH_LONG_SUFFIX;
		}
		G_CURSOR++;
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
			if (integer_value < UNSIGNED_INT_MAX) {
				tk_kind = TK_UNSIGNED_INTCONST;
			} else {
				tk_kind = TK_UNSIGNED_LONGCONST;
			}
	} else if (!(suffix_flag & WITH_UNSIGNED_SUFFIX)
		&& suffix_flag & WITH_LONG_SUFFIX) {
			if (integer_value < SIGNED_LONG_MAX) {
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

	integer_value = strtol(base_ptr, NULL, 0);
	if (errno == ERANGE) {
		warn_message("overflow in implicit constant conversion");
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

/*For example, 'ab' for a target with an 8-bit char would be interpreted as ¡®(int) ((unsigned char) 'a' * 256 + (unsigned char) 'b')¡¯, and '\234a' as ¡®(int) ((unsigned char) '\234' * 256 + (unsigned char) 'a')¡¯.*/

static unsigned char scan_character_hex()			//most two Bit hex
{
	unsigned char hex_value;

	if (!is_hex_number(*G_CURSOR)) {
		log_error("\\x used hex without hex digits");
	}
	hex_value = *G_CURSOR;

	G_CURSOR++;
	if (is_hex_number(*G_CURSOR)) {
		hex_value <<= 4;
		hex_value += *G_CURSOR;
	}

	return hex_value;
}

static unsigned char scan_character_oct()		//most three Bit oct
{
	unsigned char oct_value;

	oct_value = *G_CURSOR;

	G_CURSOR++;
	if (is_octal_number(*G_CURSOR)) {
		oct_value <<= 3;
		oct_value += *G_CURSOR;
	}

	G_CURSOR++;
	if (is_octal_number(*G_CURSOR)) {
		oct_value <<= 3;
		oct_value += *G_CURSOR;
	} else {
		G_CURSOR--;
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

static unsigned char trans_simple_escape_sequence_to_ascii(unsigned char character)
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
		log_error("unknow escape sequence: %c", character);
	}
	return ret_char;
}

static unsigned char scan_one_character()
{
	unsigned char ret_char;

	if (*G_CURSOR == '\\') {			//escape sequence
		G_CURSOR++;
		if (is_hex_escape_sequce(*G_CURSOR)) {			//hex escape sequence
			G_CURSOR++;						//jump 'x'
			ret_char = scan_character_hex();
		} else if (is_octal_number(*G_CURSOR)) {  //oct escape sequence
			ret_char = scan_character_oct();
		} else {
			ret_char = trans_simple_escape_sequence_to_ascii(*G_CURSOR);  //simple escape sequence
		}
	} else if (*G_CURSOR >= '!' && *G_CURSOR <= '~') {
		ret_char = *G_CURSOR;
		if (ret_char == '\'') {
			log_error("empty character constant");
		}
	} else {
		log_error("invalid character constant");
	}

	return ret_char;
}

void scan_character()
{
	unsigned char temp_character;
	int character_value;

	G_CURSOR++;

	temp_character = scan_one_character();
	character_value = temp_character;

	while (*G_CURSOR != '\'') {
		temp_character = scan_one_character();
		character_value <<= 8;
		character_value += temp_character;
	}
	g_current_token.tk_kind = TK_INTCONST;
	g_current_token.token_value.int_num = character_value;
	g_current_token.line = G_LINE;

}

void scan_string_literal()
{
	
}

void scan_w_character()
{

}

void scan_w_str_literal()
{

}

/*keywords hash*/

int get_string_key(char *str, int len)
{
	unsigned int hash_value;
	int i;

	hash_value = 0;
	for (i = 0; i < len; i++) {
		hash_value = (*str) + (hash_value << 6) + (hash_value << 16) - hash_value;
		str++;
	}

	return (hash_value & 0x7FFFFFFF);
}

int hash_key(int key_value)
{
	return key_value % KEYWORD_HASHTABLE_NUM;
}

void insert_keyword_hash(char *keyword, int token)
{
	int key_value;
	int hash_value;

	key_value = get_string_key(keyword, bcc_strlen(keyword));
	hash_value = hash_key(key_value);
	
	while (g_keywords_hashtable[hash_value].key != 0) {
		hash_value++;
	}

	g_keywords_hashtable[hash_value].key = key_value;
	g_keywords_hashtable[hash_value].key = token;
}

int lookup_keywords(char *str, int len)
{
	int key_value, hash_value;

	key_value = get_string_key(str, len);
	hash_value = hash_key(key_value);

	while (g_keywords_hashtable[hash_value].key != 0) {
		if (g_keywords_hashtable[hash_value].key == key_value) {
			return g_keywords_hashtable[hash_value].token;
		}
	}
	return 0;
}

void init_keywords_hash()
{
	int i;
	
	for (i = 0; i < KEYWORD_HASHTABLE_NUM; i++) {
		g_keywords_hashtable[i].key = 0;
		g_keywords_hashtable[i].token = 0;
	}

	insert_keyword_hash("auto", TK_AUTO);
	insert_keyword_hash("extern", TK_EXTERN);
	insert_keyword_hash("register", TK_REGISTER);
	insert_keyword_hash("static", TK_STATIC);
	insert_keyword_hash("typedef", TK_TYPEDEF);
	insert_keyword_hash("const", TK_CONST);
	insert_keyword_hash("volatile", TK_VOLATILE);
	insert_keyword_hash("void", TK_VOID);
	insert_keyword_hash("char", TK_CHAR);
	insert_keyword_hash("short", TK_SHORT);
	insert_keyword_hash("int", TK_INT);
	insert_keyword_hash("long", TK_LONG);
	insert_keyword_hash("float", TK_FLOAT);
	insert_keyword_hash("double", TK_DOUBLE);
	insert_keyword_hash("signed", TK_SIGNED);
	insert_keyword_hash("unsigned", TK_UNSIGNED);
	insert_keyword_hash("struct", TK_STRUCT);
	insert_keyword_hash("union", TK_UNION);
	insert_keyword_hash("enum", TK_ENUM);
	insert_keyword_hash("break", TK_BREAK);
	insert_keyword_hash("case", TK_CASE);
	insert_keyword_hash("continue", TK_CONTIUE);
	insert_keyword_hash("default", TK_DEFAULT);
	insert_keyword_hash("if", TK_IF);
	insert_keyword_hash("else", TK_ELSE);
	insert_keyword_hash("do", TK_DO);
	insert_keyword_hash("for", TK_FOR);
	insert_keyword_hash("while", TK_WHILE);
	insert_keyword_hash("goto", TK_GOTO);
	insert_keyword_hash("return", TK_RETURN);
	insert_keyword_hash("switch", TK_SWITCH);
}


/*identify hash*/
int hash_identify(int key_value)
{
	return key_value % g_identify_hashtable.size;
}
void init_identify_hashtable()
{
	g_identify_hashtable.ele_num = 0;
	g_identify_hashtable.size = DEFAULT_IDENTIFY_HASHTABLE_NUM;
	g_identify_hashtable.iden_ele = (identify_ele_t **) bcc_calloc(DEFAULT_IDENTIFY_HASHTABLE_NUM, sizeof(identify_ele_t*));
}

void insert_iden_key_hash(int key_value, char* p_ident)
{
	int hash_value;
	identify_ele_t *new_ident_ele;

	hash_value = hash_identify(key_value);

	new_ident_ele = (identify_ele_t *)bcc_malloc(sizeof(identify_ele_t));

	new_ident_ele->key = key_value;
	new_ident_ele->p_ident = p_ident;
	new_ident_ele->next = g_identify_hashtable.iden_ele[hash_value];
}

void resize_identify_hashtable()
{
	int i;
	identify_ele_t *curr_ident_element;
	int old_table_size, new_table_size;
	identify_ele_t **old_iden_ele;

	old_table_size = g_identify_hashtable.size;
	old_iden_ele = g_identify_hashtable.iden_ele;

	new_table_size = old_table_size * 2;

	g_identify_hashtable.size = new_table_size;
	g_identify_hashtable.iden_ele = (identify_ele_t **)bcc_calloc(new_table_size, sizeof(identify_ele_t*));

	for (i = 0; i <= old_table_size; i++) {
		curr_ident_element = old_iden_ele[i];
		while (curr_ident_element) {
			insert_iden_key_hash(curr_ident_element->key, curr_ident_element->p_ident);
			curr_ident_element = curr_ident_element->next;
		}
	}
	bcc_free(old_iden_ele);
}

char* insert_identify_hash(char *identify, int len)
{
	int key_value;
	int hash_value;
	identify_ele_t *new_ident_ele;


	if (g_identify_hashtable.ele_num >= g_identify_hashtable.size) {		//load factor over 1
		resize_identify_hashtable();
	}

	key_value = get_string_key(identify, len);
	hash_value = hash_identify(key_value);

	new_ident_ele = (identify_ele_t *)bcc_malloc(sizeof(identify_ele_t));

	new_ident_ele->key = key_value;
	new_ident_ele->p_ident = (char*)bcc_malloc(len + 1);
	bcc_strncpy(new_ident_ele->p_ident, identify, len);
	new_ident_ele->p_ident[len] = '\0';
	new_ident_ele->next = g_identify_hashtable.iden_ele[hash_value];
	
	return new_ident_ele->p_ident;
}

char* lookup_identify_hash(char *identify, int len)
{
	int key_value;
	int hash_value;
	identify_ele_t *curr_ident_ele;

	key_value = get_string_key(identify, len);
	hash_value = hash_identify(key_value);
	
	curr_ident_ele = g_identify_hashtable.iden_ele[hash_value];
	while (curr_ident_ele != NULL) {
		if (curr_ident_ele->key == key_value) {
			return curr_ident_ele->p_ident;
		}
	}
	return NULL;
}

