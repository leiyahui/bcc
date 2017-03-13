#ifndef _LEX_H
#define _LEX_H

#include "bcc.h"

enum TOKEN {
	/*storage_class_specifier*/
	TK_AUTO, TK_EXTERN, TK_REGISTER, TK_STATIC, TK_TYPEDEF,
	/*type qualifier*/
	TK_CONST, TK_VOLATILE,
	/*type specifier*/
	TK_VOID, TK_CHAR, TK_SHORT, TK_INT, TK_LONG, TK_FLOAT, TK_DOUBLE, TK_SIGNED, TK_UNSIGNED, TK_STRUCT, TK_UNION, TK_ENUM,
	/*rest key words*/
	TK_BREAK, TK_CASE, TK_CONTIUE, TK_DEFAULT, TK_SIZEOF,
	TK_IF, TK_ELSE, TK_DO, TK_FOR, TK_WHILE, TK_GOTO, TK_RETURN, TK_SWITCH,
	/*constant*/
	TK_STRING, TK_LETERAL,
	TK_INTCONST, TK_UNSIGNED_INTCONST, TK_LONGCONST, TK_UNSIGNED_LONGCONST, 
	TK_LLONGCONST, TK_UNSIGNED_LLONGCONST, TK_FLOATCONST, TK_DOUBLECONST, TK_LDOUBLECONST,
	/*identifier*/
	TK_IDENTIFIER,
	/*OPERATOR*/
	TK_ADD, TK_SUB, TK_MULTIPLY, TK_DIVIDE, TK_MOD, 
	TK_BITOR, TK_BITXOR, TK_AND, TK_LSHIFT, TK_RSHIFT, 
	TK_PTR, TK_INC, TK_DEC,
	/*assign operator*/
	TK_ASSIGN,
	TK_ADD_ASSING, TK_SUB_ASSIGN, TK_MULTI_ASSIGN, TK_DIVIDE_ASSIGN, TK_MOD_ASSIGN, 
	TK_BITOR_ASSIGN, TK_BITXOR_ASSIGN, TK_BITAND_ASSIGN, TK_LSHIFT_ASSIGN, TK_RSHIFT_ASSIGN, 
	/*special character*/
	TK_COMMA, TK_QUESTION, TK_COLON, TK_NOT, TK_COMP, TK_DOT, TK_POINTER,
	TK_SEMICOLON, TK_ELLIPSE,
	/*bracket*/
	TK_LPAREN, TK_PAREN, TK_LBRACKET, TK_RBRACKET, TK_LBRACE, TK_RBRACE, 
	
	/*condition operator*/
	TK_EQUAL, TK_NEQUAL, TK_LESS_EQUAL, TK_GREAT_EQUAL, TK_LESS, TK_GREAT, 
};

typedef union _token_value_t {
	short short_num;
	int int_num;
	long long_num;
	float float_num;
	double double_num;
	char *ptr;
}token_value_t;

typedef struct _token_t{
	int tk_kind;
	int line;		//the line of this token;
	token_value_t token_value;
}token_t;

typedef void (*lex_scan_func)();

/*lexcial scanner*/
extern lex_scan_func g_scanner[255];

BOOL is_letter(int ascii);
BOOL is_dec_num(int ascii);
BOOL is_underline(int ascii);

void init_scanner();

void scan_identifier();
void scan_number();
void scan_string_literal();
void scan_character();
void scan_w_character();
void scan_w_str_literal();
void scan_comma();
void scan_question_mark();
void scan_colon();
void scan_equal_sign();
void scan_or();
void scan_and();
void scan_less();
void scan_great();
void scan_add();
void scan_minus();
void scan_multi();
void scan_divide();
void scan_percent();
void scan_caret();
void scan_comp();
void scan_dot();
void scan_lparen();
void scan_rparen();
void scan_lbrace();
void scan_rbrace();
void scan_lbracket();
void scan_rbracket();
void scan_semicolon();
void scan_exclamation();
void scan_file_end();
void scan_bad_letter();


void get_next_token(input_file_t *input_file);

/*keywords and identify hash*/

int get_string_key(char *str, int len);

/*keywords*/
#define KEYWORD_HASHTABLE_NUM	100

typedef struct _keyword_ele_t {
	int key;
	int token;
}keyword_ele_t;


extern keyword_ele_t g_keywords_hashtable[KEYWORD_HASHTABLE_NUM];
int hash_keywords(int key_value);
void init_keywords_hash();
void insert_keyword_hash(char *keyword, int token);
int lookup_keywords(char *str, int len);

/*identify hash*/
typedef struct _identify_ele_t {
	int key;
	char *p_ident;
	struct _identify_ele_t *next;
}identify_ele_t;

typedef struct _identify_hashtable_t {
	int size;
	int ele_num;
	identify_ele_t **iden_ele;
}identify_hashtable_t;


#define DEFAULT_IDENTIFY_HASHTABLE_NUM 200

extern identify_hashtable_t g_identify_hashtable;
int hash_identify(int key_value);
void init_identify_hashtable();
char* insert_identify_hash(char *identify, int len);
char* lookup_identify_hash(char *identify, int len);




#endif
