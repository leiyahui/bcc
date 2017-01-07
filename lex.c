#include "bcc.h"

lex_scan_func g_scanner[255];
keyword_ele_t g_keywords_hashtable[100];
identify_hashtable_t g_identify_hashtable;
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

BOOL is_scan_end(char letter)
{
	if (letter == ' ' || letter == '\r' || letter == '\t') {
		return TRUE;
	}
	return FALSE;
}

void scan_identity()
{
	char *base_ptr, *curr_ptr;
	char *identify_ptr;
	char curr_letter;
	int str_len, token;
	token_value_t token_value;


	base_ptr = curr_ptr = g_input_file.base;
	curr_letter = *curr_ptr;
	while (!is_scan_end(curr_letter)) {
		curr_ptr++;
	}
	g_input_file.cursor = curr_ptr;
	str_len = curr_ptr - base_ptr;

	token = lookup_keywords(base_ptr, str_len);
	if (token) {								//identify is keyword
		g_current_token.tk_kind = token;
		g_current_token.line = g_input_file.line;
		return ;
	}
	identify_ptr = lookup_identify_ptr(base_ptr, str_len);
	if (!identify_ptr) {
		identify_ptr = insert_identify_hash(base_ptr, str_len);
	}
	token = TK_IDENTIFIER;
	g_current_token.tk_kind = token;
	g_current_token.line = g_input_file.line;
	g_current_token.token_value.ptr = identify_ptr; 
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

char* lookup_identify_ptr(char *identify, int len)
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

