#ifndef _SYMBOL_H
#define _SYMBOL_H

#include "bcc.h"

#define STRUCT_SYM			1
#define UNION_SYM			2
#define ENUM_SYM			3
#define STATE_LABLES_SYM	4
#define TDNAME_SYM			5
#define OTHER_IDET			6


typedef struct _symbol_t {
	int type;
	char *str;
	int defined;
	symbol_t *next;
}symbol_t;

typedef struct _namespace_t {
	symbol_t *sym_list;
}namespace_t;

#define STATE_LABLES	1
#define TAGS			2
#define OTHER_IDENT		3
#define TDNAME			4

typedef struct _scope_t {
	namespace_t statement_lables;
	namespace_t tags;
	namespace_t other_ident;
	namespace_t tdname;
	struct _scope_t *parent;
}scope_t;

void insert_to_namespace(namespace_t *curr_namespace, char *str, int type, BOOL defined);

BOOL is_in_namespace(namespace_t *curr_namespace, char *str);

void insert_to_scope(scope_t *curr_scope, char *str, int kind, BOOL defined);

#endif
