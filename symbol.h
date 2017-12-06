#ifndef _SYMBOL_H
#define _SYMBOL_H

#include "bcc.h"

typedef struct _symbol_t {
	char *name;
	type_t *type;
	BOOL is_typedef;
	BOOL defined;
	BOOL is_enum_const;
	double init_value;
	struct _symbol_t *next;
}symbol_t;

typedef struct _symbol_table_t {
	symbol_t *list_head;
	symbol_t *list_tail;
	struct _symbol_table_t *parent;
}symbol_table_t;

typedef struct _user_define_type_t {				// including tags, statement_labels, typedef name
	char *name;
	type_t *type;
	BOOL has_declaration;				//user defined type may only define a type name,but declarator
	struct _user_define_type_t *next;
}user_define_type_t;

typedef struct _user_df_ty_table_t {
	user_define_type_t *list_head;
	user_define_type_t *list_tail;
	BOOL has_declarator;
	struct _user_df_ty_table_t *parent;
}user_df_ty_table_t;

extern symbol_table_t *g_sym_tb;		//variable and typedef name
extern user_df_ty_table_t *g_lables_tb;
extern user_df_ty_table_t *g_tag_tb;

void insert_to_user_define_type(user_df_ty_table_t *ty_table, char *name, type_t *type, BOOL has_declaration);

void insert_to_sym_table(char *name, type_t *type, BOOL is_typedef, BOOL defined, BOOL is_enum_const, int init_val);

BOOL is_curr_scope_define_type(user_df_ty_table_t *ty_table, char *name);

BOOL is_ances_scope_define_type(user_df_ty_table_t *ty_table, char *name);

BOOL is_user_define_type(user_df_ty_table_t *ty_table, char *name);

BOOL in_symbol_table(symbol_table_t*sym_tb, char *name);

user_df_ty_table_t *get_user_def(user_df_ty_table_t *ty_table, char *name);

type_t *get_user_def_type(user_df_ty_table_t *ty_table, char *name);

BOOL in_curr_scope_sym_tb(symbol_table_t *sym_table, char *name);

symbol_t *get_symbol(symbol_table_t *sym_table, char *name);

type_t *get_symbol_type(symbol_table_t* sym_tb, char *name);
#endif
