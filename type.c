#include "bcc.h"

#define BASE_TYPE_NUM 8

type_t g_base_type[BASE_TYPE_NUM];

#define INIT_ONE_BASE_TYPE(type, size)						\
			g_base_type[type].kind = type;					\
			g_base_type[type].align = size;					\
			g_base_type[type].size = size;					\
			g_base_type[type].store_cls = AUTO_STORE_CLS	\
			g_base_type[type].qual = 0;				\
			g_base_type[type].base_type = NULL

void init_base_type()
{
	INIT_ONE_BASE_TYPE(TYPE_CHAR, 1);
	INIT_ONE_BASE_TYPE(TYPE_SHORT, 2);
	INIT_ONE_BASE_TYPE(TYPE_INT, 4);
	INIT_ONE_BASE_TYPE(TYPE_LONG, 4);
	INIT_ONE_BASE_TYPE(TYPE_FLOAT, 4);
	INIT_ONE_BASE_TYPE(TYPE_DOUBLE, 8);
	INIT_ONE_BASE_TYPE(TYPE_POINTER, 4);
	INIT_ONE_BASE_TYPE(TYPE_ARRAY, 0);
}

tag_type_t * create_tag(char * name, int struct_or_union)
{
	tag_type_t *type;
	type = (tag_type_t *)bcc_malloc(sizeof(tag_type_t));
	type->name = name;
	type->type = struct_or_union;
	type->head = type->tail = NULL;
}

void add_field_to_tag(tag_type_t *tag, type_t *type, char *name, int bits)
{
	field_type_t *field;
	field = (field_type_t *)bcc_malloc(sizeof(field_type_t));
	
	field->type = type;
	field->name = name;
	field->bits = bits;
	field->next = NULL;

	if (tag->head = NULL) {
		tag->head = tag->tail = field;
	} else {
		tag->tail->next = field;
		tag->tail = field;
	}
}

function_type_t* create_func_type(char * name, type_t * ret)
{
	function_type_t *type;

	type->name = name;
	type->ret = ret;
	type->head = type->tail = NULL;
	return type;
}

void add_param_to_func(function_type_t *func, type_t *type, char *name)
{
	field_type_t * param;
	param = (field_type_t * *)bcc_malloc(sizeof(field_type_t));
	
	param->name = name;
	param->type = type;
	param->next = NULL;

	if (func->head = NULL) {
		func->head = func->tail = param;
	} else {
		func->tail->next = param;
		func->tail = param;
	}
}

td_type_t * create_td_type(char * name, type_t * type)
{
	td_type_t *td;
	
	td->name = name;
	td->type = type;

	return td;
}



type_t * get_postfix_declarator_type(decl_postfix_t * decl_postfix)
{
	type_t *type;
	int array_len;
	type = (type_t *)bcc_malloc(sizeof(type_t));
	
	if (decl_postfix->paren_or_barcket == BRACKET) {
		type->kind = TYPE_ARRAY;
		type->align = type->size = ((const_expr_t *)decl_postfix->const_expr)->value;
	} else if (decl_postfix->paren_or_barcket == PAREN) {
		type->kind = TYPE_PARAMETER;
		type->param_type = 
	}
	return NULL;
}

type_t * get_declarator_type(declarator_t * decl)
{
	return NULL;
}

type_t * get_struct_declarator_type(struct_declarator_t * decl)
{
	return NULL;
}

type_t * get_struct_union_type(type_spec_t *type_spec)
{
	struct_or_union_spec_t *tag;
	struct_declaration_t *decl;

	tag_type_t *tag_type;
	
	tag = (struct_or_union_spec_t *)(type_spec->value);
	tag_type = create_tag(tag->ident, type_spec->kind);
	
	decl = tag->struct_decl;
	decl

}

type_t *get_decl_spec_type(decl_spec_t *spec)
{
	type_qual_t *type_qual;
	type_spec_t *type_spec;
	store_cls_spec_t *store_cls;
	int store_cls_tk;
	int type_spec_tk;

	type_t *type;
	type = (type_t *)bcc_malloc(sizeof(type_t));

	type_qual = spec->type_qual;
	type_spec = spec->type_spec;
	store_cls = spec->store_cls;

	//if (store_cls) {
	//	type->store_cls = NODE_TOKEN_KIND(store_cls->value);
	//}
	//if (type_qual) {
	//	if (type_qual->const_tk != NULL) {
	//		type->qual |= WITH_CONST;
	//	}
	//	if (type_qual->volatile_tk != NULL) {
	//		type->qual |= WITH_VOLATILE;
	//	}
	//}
	if (type_spec) {
		switch (NODE_TOKEN_KIND(type_spec->value)) {
		case TK_STRUCT:
			tag_type_t *tag = create_tag()
		}
	}
}