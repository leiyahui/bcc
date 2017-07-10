#include "bcc.h"
void insert_to_namespace(namespace_t *curr_namespace, char *str, int type, BOOL defined)
{
	symbol_t *new_sym;

	new_sym = (symbol_t *)bcc_malloc(sizeof(symbol_t));
	new_sym->str = str;
	new_sym->type = type;
	new_sym->defined = defined;
	new_sym->next = curr_namespace->sym_list;

	curr_namespace->sym_list = new_sym;
}

BOOL is_in_namespace(namespace_t *curr_namespace, char *str)
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

void insert_to_scope(scope_t *curr_scope, char *str, int kind, BOOL defined)
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
	insert_to_namespace(curr_namespace, str, kind, defined);
}

