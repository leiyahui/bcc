#include "bcc.h"

#define BASE_TYPE_NUM 8

type_t g_base_type[BASE_TYPE_NUM];

#define INIT_ONE_BASE_TYPE(type, size_a)						\
			g_base_type[type].kind = type;						\
			g_base_type[type].align = size_a;					\
			g_base_type[type].size = size_a;					\
			g_base_type[type].store_cls = AUTO_STORE_CLS;	\
			g_base_type[type].qual = 0;						\
			g_base_type[type].base_type = NULL;

void init_base_type()
{
	INIT_ONE_BASE_TYPE(TYPE_VOID, 0);
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

void add_field_list_to_tag(tag_type_t *tag, field_list_t *list)
{
	if (tag->head = NULL) {
		tag->head = tag->tail = list;
	}
	else {
		tag->tail->next = list;
		tag->tail = list;
	}
}

field_list_t *create_field_list(type_t *type)
{
	field_list_t *field;
	field = (field_list_t *)bcc_malloc(sizeof(field_list_t));
	field->type = type;
	field->head = field->tail = NULL;
}

void add_field_to_same_type_list(field_list_t *field_list, type_t *type, char *name, int bits)
{
	field_t *field;

	field = (field_t*)bcc_malloc(sizeof(field_t));
	field->name = name;
	field->bits = bits;

	if (field_list->head == NULL) {
		field_list->head = field_list->tail = field;
	}
	else {
		field_list->tail->next = field;
		field_list->tail = field;
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
	param_type_t * param;
	param = (param_type_t **)bcc_malloc(sizeof(param_type_t));

	param->name = name;
	param->type = type;
	param->next = NULL;

	if (func->head = NULL) {
		func->head = func->tail = param;
	}
	else {
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


#define POINTER_LENGTH 4

type_t *derive_pointer_type(type_t *base_type, int qual)
{
	type_t *new_type;
	new_type = (type_t *)bcc_malloc(sizeof(type_t));

	new_type->qual = qual;
	new_type->store_cls = base_type->store_cls;
	new_type->kind = TYPE_POINTER;
	new_type->size = new_type->align = 4;
	new_type->base_type = base_type;
	return new_type;
}

type_t *derive_array_type(type_t *base_type, int len)
{
	type_t *new_type;

	new_type = (type_t *)bcc_malloc(sizeof(type_t));

	new_type->qual = base_type->qual;
	new_type->store_cls = base_type->store_cls;
	new_type->kind = TYPE_ARRAY;
	new_type->size = base_type->size * len;
	new_type->align = base_type->align;
	new_type->base_type = base_type;
	return new_type;
}

type_t *derive_decl_spec_type(type_t *base_type, int qual, int sign, int store_cls)
{
	type_t *new_type;

	new_type->qual = qual;
	new_type->sign = sign;
	new_type->store_cls = store_cls;
	new_type->align = base_type->align;
	new_type->sign = base_type->sign;
	new_type->kind = base_type->kind;

	return new_type;
}

type_t *get_declaration_base_type(declaration_t *decl)
{
	type_t *type;
	pointer_t *pointer;

	if (decl->decl_spec) {
		type = get_decl_spec_type(decl->decl_spec);
	}

	return type;
}

void add_param_list_to_func(function_type_t *func_type, param_type_list_t *param_decl)
{
	type_t *base_type, *type;
	declaration_t *param_declaration;
	char *name;

	param_declaration = param_decl->param_list;

	while (param_declaration) {
		base_type = get_declaration_base_type(param_declaration);
		switch (param_declaration->declarator_list->kind) {
		case DECLARATOR:
			type = get_declarator_type(base_type, param_declaration->declarator_list);
			add_param_to_func(func_type, type, param_declaration->declarator_list->direct_declarator->ident->tk_val.token_value.ptr);
		case ABS_DECLARATOR:
			type = get_declarator_type(base_type, param_declaration->declarator_list);
			add_param_to_func(func_type, type, NULL);
		default:
			break;
		}
		param_declaration = param_declaration->next;
	}
	func_type->with_ellipse = param_decl->with_ellipse;
}

type_t *get_declarator_type(type_t *base_type, declarator_t *decl)
{
	type_t *type;
	pointer_t *pointer;
	decl_postfix_t *post;

	pointer = decl->pointer;
	type = base_type;
	while (pointer) {
		type = derive_pointer_type(type, pointer->type_qual_ptr->qual);
		pointer = pointer->next;
	}
	post = decl->direct_declarator->post;

	while (post) {
		if (post->paren_or_barcket == BRACKET) {
			type = derive_array_type(type, post->const_expr->value);
		}
		if (post->paren_or_barcket == PAREN) {
			type = create_func_type(decl->direct_declarator->ident->tk_val.token_value.ptr, type);
			add_param_list_to_func(type, post->param_list);
		}
	}
	return type;
}

type_t * get_struct_union_type(type_spec_t *type_spec)
{
	struct_or_union_spec_t *tag;
	declaration_t *declaration;
	declarator_t *declarator;
	type_t* decl_base_type, *decl_type;
	type_t *tag_type, *field_list;

	tag = (struct_or_union_spec_t *)(type_spec->value);
	tag_type = create_tag(tag->ident->tk_val.token_value.ptr, tag->s_or_u);

	declaration = tag->struct_decl;
	while (declaration) {
		decl_base_type = get_declaration_base_type(declaration);
		declarator = declaration->declarator_list;
		field_list = create_field_list(decl_base_type);
		while (declarator) {
			decl_type = get_declarator_type(decl_base_type, declarator);
			add_field_to_same_type_list(field_list, decl_type, declarator->direct_declarator->ident->tk_val.token_value.ptr, declarator->const_expr->value);
			declarator = declarator->next;
		}
		add_field_list_to_tag(tag, field_list);
		declaration = declaration->next;
	}
	return tag;
}

int trans_tk_kind_to_type_kind(int tk_kind)
{
	switch (tk_kind) {
	case TK_VOID:
		return TYPE_VOID;
	case TK_CHAR:
		return TYPE_CHAR;
	case TK_SHORT:
		return TYPE_SHORT;
	case TK_INT:
		return TYPE_INT;
	case TK_LONG:
		return TYPE_LONG;
	case TK_FLOAT:
		return TYPE_LONG;
	case TK_DOUBLE:
		return TK_DOUBLE;
	default:
		ERROR("not base type");
	}
}

type_t *get_decl_spec_type(decl_spec_t *spec)
{
	int store_cls_tk;
	int type_spec_tk;

	type_t *base_type, *type;
	type = base_type = NULL;

	switch (spec->type_spec->kind) {
	case TK_VOID:
	case TK_CHAR:
	case TK_SHORT:
	case TK_INT:
	case TK_LONG:
	case TK_FLOAT:
	case TK_DOUBLE:
		base_type = &g_base_type[trans_tk_kind_to_type_kind(spec->type_spec->kind)];
		break;
	case TK_STRUCT:
	case TK_UNION:
		base_type = get_struct_union_type(spec->type_spec);
		break;
	case TK_IDENTIFIER:
		base_type = get_user_def_type(&(g_curr_scope->tdname_head), spec->type_spec->value->tk_val.token_value.ptr);
		break;
	default:
		break;
	}

	if (spec->type_spec->sign) {
		if (base_type != NULL) {
			base_type = &g_base_type[TYPE_INT];
		}
	}

	type = derive_decl_spec_type(base_type, spec->type_qual->qual, spec->type_spec->sign, spec->store_cls->kind);
	return type;
}