#ifndef _LEX_H
#define _LEX_H

#include "bcc.h"

typedef union _token_value_t {
	short short_num;
	int int_num;
	long long_num;
	float float_num;
	double double_num;
	char *ptr;
}token_value_t;

typedef struct token_t{
	int tk_kind;
	int line;		//the line of this token;
	token_value_t token_value;  
}token_t;

typedef void* (*lex_scan_func)();

void init_scanner(lex_scan_func *scanner_fun);

BOOL is_letter(char *ptr);
BOOL is_digit(char *ptr);
BOOL is_underline(char *ptr);

void scan_identity();
void scan_number();
void scan_string();
void scan_letter();
void scan_comma();
void scan_question_mark();
void scan_colon();
void scan_equal_sign();
void scan_bar();
void ScanAmpersand();
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
void get_next_token(input_file_t *input_file);

char* 

#endif