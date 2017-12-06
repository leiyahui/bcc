#include "bcc.h"

void insert_to_user_define_type(user_df_ty_table_t *ty_table, char *name, type_t *type, BOOL has_declarator)
{
	user_define_type_t *new_type;

	new_type = (user_define_type_t *)bcc_malloc(sizeof(user_define_type_t));

	new_type->name = name;
	new_type->type = type;
	new_type->has_declaration = has_declarator;
	new_type->next = NULL;

	ty_table->list_tail->next = new_type;
	ty_table->list_tail = new_type;
}

void insert_to_sym_table(char *name, type_t *type, BOOL is_typedef, BOOL defined, BOOL is_enum_const, int init_val)
{
	symbol_t *new_sym;

	new_sym = (symbol_t *)bcc_malloc(sizeof(symbol_t));

	new_sym->name = name;
	new_sym->type = type;
	new_sym->is_typedef = is_typedef;
	new_sym->defined = defined;		//used for function body
	new_sym->is_enum_const = is_enum_const;
	new_sym->init_value = init_val;
	new_sym->next = NULL;

	g_sym_tb->list_tail->next = new_sym;
	g_sym_tb->list_tail = new_sym;
}

BOOL is_curr_scope_define_type(user_df_ty_table_t *ty_table, char *name)
{
	user_define_type_t *iter_type;

	iter_type = ty_table->list_head;
	while (iter_type != NULL) {
		if (iter_type->name == name) {
			return TRUE;
		}
		iter_type = iter_type->next;
	}
	return FALSE;
}

BOOL is_user_define_type(user_df_ty_table_t *ty_table, char* name)
{
	if (is_curr_scope_define_type(ty_table, name)) {
		return TRUE;
	}
	if (ty_table->parent) {
		return is_user_define_type(ty_table->parent, name);
	}
	return FALSE;
}

BOOL is_ances_scope_define_type(user_df_ty_table_t *ty_table, char *name)
{
	if (ty_table->parent) {
		return is_user_define_type(ty_table->parent, name);
	}
	return FALSE;
}

BOOL in_symbol_table(symbol_table_t *sym_tb, char *name)
{
	symbol_t *iter_sym;

	iter_sym = sym_tb->list_head;
	while (iter_sym != NULL) {
		if (iter_sym->name == name) {
			return TRUE;
		}
	}
	if (sym_tb->parent) {
		return in_symbol_table(sym_tb->parent, name);
	}
	
	return FALSE;
}

user_df_ty_table_t *get_user_def(user_df_ty_table_t *ty_table, char *name)
{
	user_define_type_t *iter_type;

	iter_type = ty_table->list_head;
	while (iter_type != NULL) {
		if (iter_type->name == name) {
			return iter_type;
		}
		iter_type = iter_type->next;
	}
	if (ty_table->parent) {
		return get_user_def_type(ty_table->parent, name);
	}
	return NULL;
}

type_t *get_user_def_type(user_df_ty_table_t *ty_table, char *name)
{
	user_define_type_t *iter_type;

	iter_type = ty_table->list_head;
	while (iter_type != NULL) {
		if (iter_type->name == name) {
			return iter_type->type;
		}
		iter_type = iter_type->next;
	}
	if (ty_table->parent) {
		return get_user_def_type(ty_table->parent, name);
	}
	return NULL;
}

BOOL in_curr_scope_sym_tb(symbol_table_t *sym_table, char *name)
{
	symbol_t *iter_sym;

	iter_sym = sym_table->list_head;
	while (iter_sym != NULL) {
		if (iter_sym->name == name) {
			return TRUE;
		}
	}
	return FALSE;
}

symbol_t *get_symbol(symbol_table_t *sym_table, char *name)
{
	symbol_t *iter_sym;

	iter_sym = sym_table->list_head;
	while (iter_sym != NULL) {
		if (iter_sym->name == name) {
			return iter_sym;
		}
	}
	if (sym_table->parent) {
		return get_symbol(sym_table->parent, name);
	}
	return NULL;
}

type_t *get_symbol_type(symbol_table_t *sym_table, char *name)
{
	symbol_t *iter_sym;

	iter_sym = sym_table->list_head;
	while (iter_sym != NULL) {
		if (iter_sym->name == name) {
			return iter_sym->type;
		}
	}
	if (sym_table->parent) {
		return get_symbol_type(sym_table->parent, name);
	}
	return NULL;
}