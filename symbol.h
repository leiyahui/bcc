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
	symbol_t *next;
}symbol_t;

typedef struct _namespace_t {
	symbol_t *sym_list;
}namespace_t;

typedef struct _scope_t {
	namespace_t statement_lables;
	namespace_t tags;
	namespace_t other_ident;
	namespace_t tdname;
	struct _scope_t *parent;
}scope_t;

#define STATE_LABLES	1
#define TAGS			2
#define OTHER_IDENT		3
#define TDNAME			4

void insert_to_namespace(namespace_t *curr_namespace, char *str, int type)
{
	symbol_t *new_sym;

	new_sym = (symbol_t *)bcc_malloc(sizeof(symbol_t));
	new_sym->str = str;
	new_sym->type = type;
	new_sym->next = curr_namespace->sym_list;

	curr_namespace->sym_list = new_sym;
}

BOOL in_namespace(namespace_t *curr_namespace, char *str)
{
	symbol_t *sys_iter;

	sys_iter = curr_namespace->sym_list;
	
	while (sys_iter != NULL) {
		if (sys_iter->str == str) {
			return TRUE;
		}
		sys_iter = sys_iter->next;
	}
	return FALSE;
}

void insert_to_scope(scope_t *curr_scope, char *str, int kind)
{
	namespace_t *curr_namespace;
	switch (kind) {
	case STATE_LABLES:
		curr_namespace = &(curr_scope->statement_lables);
		break;
	case TAGS:
		curr_namespace = &(curr_scope->tags);
		break;
	case OTHER_IDENT:
		curr_namespace = &(curr_scope->other_ident);
		break;
	case TDNAME:
		curr_namespace = &(curr_scope->tdname);
		break;
	}
	insert_to_namespace(curr_namespace, str, kind);
}





