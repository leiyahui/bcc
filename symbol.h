#ifndef _SYMBOL_H
#define _SYMBOL_H

#include "bcc.h"

extern scope_t *g_curr_scope;

#define G_SCOPE(v) = (g_curr_scope->v)

#define STRUCT_SYM			1
#define UNION_SYM			2
#define ENUM_SYM			3
#define STATE_LABLES_SYM	4
#define TDNAME_SYM			5
#define OTHER_IDET			6


typedef struct _symbol_t {
	char *name;
	type_t *type;
	symbol_t *next;
}symbol_t;

typedef struct _user_define_type_t {				// includeing tags, statement_labels, typedef name
	char *name;
	type_t *type;
	BOOL has_declarator;				//user defined type may only define a type name,but declarator
	struct _user_define_type_t *next;
}user_define_type_t;


#define STATE_LABLES	1
#define TAGS			2
#define SYMBOL			3
#define TDNAME			4

typedef struct _scope_t {
	user_define_type_t lables_head;
	user_define_type_t *lables_tail;
	user_define_type_t tags_head;
	user_define_type_t *tags_tail;
	user_define_type_t tdname_head;
	user_define_type_t *tdname_tail;
	symbol_t *sym_head;
	symbol_t *sym_tail;
	struct _scope_t *parent;
}scope_t;

void init_g_scope();

void insert_to_user_define_type(user_define_type_t *space_tail, char *name, type_t *type, BOOL has_declarator);

void insert_to_sym_table(symbol_t *sym_tail, char *name, type_t *type);

BOOL in_curr_user_define_type(user_define_type_t *type_head, char *name);

BOOL in_symbol_table(symbol_t *sym_head, char *name);

type_t *get_user_def_type(user_define_type_t *type_head, char *name);
#endif
