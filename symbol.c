#include "bcc.h"

scope_t *g_curr_scope;

void init_g_scope()
{
	g_curr_scope->lables_tail = &g_curr_scope->lables_head;
	g_curr_scope->tags_tail = &g_curr_scope->tags_head;
	g_curr_scope->tdname_tail = &g_curr_scope->tdname_head;
	g_curr_scope->sym_tail = &g_curr_scope->sym_head;
	g_curr_scope->parent = NULL;
}

void insert_to_user_define_type(user_define_type_t *space_tail, char *name, type_t *type, BOOL has_declarator)
{
	user_define_type_t *new_type;

	new_type = (user_define_type_t *)bcc_malloc(sizeof(user_define_type_t));

	new_type->name = name;
	new_type->type = type;
	new_type->has_declarator = has_declarator;
	new_type->next = NULL;

	space_tail->next = new_type;
}

void insert_to_sym_table(symbol_t *sym_tail, char *name, type_t *type)
{
	symbol_t *new_sym;

	new_sym = (symbol_t *)bcc_malloc(sizeof(symbol_t));

	new_sym->name = name;
	new_sym->type = type;
	new_sym->next = NULL;
	
	sym_tail->next = new_sym;
}

BOOL in_curr_user_define_type(user_define_type_t *type_head, char* name)
{
	user_define_type_t *iter_type;

	iter_type = type_head->next;
	while (iter_type != NULL) {
		if (iter_type->name == name) {
			return TRUE;
		}
		iter_type = iter_type->next;
	}

	return FALSE;
}

BOOL in_symbol_table(symbol_t *sym_head, char *name)
{
	symbol_t *iter_sym;

	iter_sym = sym_head->next;
	while (iter_sym != NULL) {
		if (iter_sym->name == name) {
			return TRUE;
		}
	}

	return FALSE;
}

type_t *get_user_def_type(user_define_type_t *type_head, char *name)
{
	user_define_type_t *iter_type;

	iter_type = type_head->next;
	while (iter_type != NULL) {
		if (iter_type == name) {
			return iter_type->type;
		}
		iter_type = iter_type->next;
	}
	return NULL;
}

type_t *get_symbol_type(symbol_t *sym_head, char *name)
{
	symbol_t *iter_sym;

	iter_sym = sym_head->next;
	while (iter_sym != NULL) {
		if (iter_sym->name == name) {
			return iter_sym->type;
		}
	}
	return NULL;
}