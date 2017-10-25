#include "bcc.h"

type_t *g_ty_void = &(type_t){ TYPE_VOID, 0, 0, TRUE };
type_t *g_ty_char = &(type_t) { TYPE_CHAR, 1, 1, TRUE };
type_t *g_ty_short = &(type_t) { TYPE_SHORT, 2, 2, TRUE };
type_t *g_ty_int = &(type_t) { TYPE_INT, 4, 4, TRUE };
type_t *g_ty_long = &(type_t) { TYPE_LONG, 8, 8, TRUE };
type_t *g_ty_float = &(type_t) { TYPE_FLOAT, 4, 4, TRUE };
type_t *g_ty_double = &(type_t) { TYPE_VOID, 8, 8, TRUE };
type_t *g_ty_uchar = &(type_t) { TYPE_CHAR, 1, 1, FALSE };
type_t *g_ty_ushort = &(type_t) { TYPE_SHORT, 2, 2, FALSE };
type_t *g_ty_uint = &(type_t) { TYPE_INT, 4, 4, FALSE };
type_t *g_ty_ulong = &(type_t) { TYPE_LONG, 8, 8, FALSE };

tag_type_t * create_tag_type(char * name, int struct_or_union)
{
	tag_type_t *type;
	type = (tag_type_t *)bcc_malloc(sizeof(tag_type_t));
	type->name = name;
	((type_t *)type)->kind = struct_or_union;
	type->head = type->tail = NULL;
	return type;
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
	return field;
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
	
	((type_t *)type)->kind = TYPE_FUNCTION;
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


type_t *type_conv(type_t *type)
{
	type_t *new_type;
	switch (type->kind)
	{
	case TYPE_ARRAY:
		new_type = derive_pointer_type(type, 0);
	case TYPE_FUNCTION:
		new_type = derive_pointer_type(type, 0);
	default:
		break;
	}
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
			if (type->kind = TYPE_FUNCTION) {
				ERROR("declaration a array of function");
			}
			type = derive_array_type(type, post->const_expr->value);
		}
		if (post->paren_or_barcket == PAREN) {
			if (type->kind == TYPE_FUNCTION) {
				ERROR("declared as function returning a function");
			}
			if (type->kind == TYPE_ARRAY) {
				ERROR("declared as function returning an array");
			}
			type = create_func_type(decl->direct_declarator->ident->tk_val.token_value.ptr, type);
			add_param_list_to_func(type, post->param_list);
		}
	}
	if (decl->direct_declarator->decl != NULL) {
		type = get_declarator_type(type, decl->direct_declarator->decl);
	}

	return type;
}

void add_declaration_to_sym_table(declaration_t *declaration)
{
	decl_spec_t *decl_spec;
	declarator_t *declarator;
	type_t *base_type, *type;
	char *name;

	decl_spec = declaration->decl_spec;
	base_type = get_declaration_base_type(declaration);
	declarator = declaration->declarator_list;
	while (declarator) {
		type = get_declarator_type(base_type, declarator);
		if (decl_spec->store_cls->kind == TK_TYPEDEF) {
			insert_to_user_define_type(g_curr_scope->tdname_tail, declarator->direct_declarator->ident->tk_val.token_value.ptr, type, TRUE);
		} else {
			insert_to_sym_table(g_curr_scope->sym_tail, declarator->direct_declarator->ident->tk_val.token_value.ptr, type);
		}
	}
}

type_t * get_struct_union_type(type_spec_t *type_spec)
{
	struct_or_union_spec_t *tag;
	declaration_t *declaration;
	declarator_t *declarator;
	type_t* decl_base_type, *decl_type;
	type_t *tag_type, *field_list;
	char *tag_name;

	tag = (struct_or_union_spec_t *)(type_spec->value);
	if (tag->ident == NULL) {
		tag_type = create_tag_type(NULL, tag->s_or_u);
	} else {
		tag_type = create_tag_type(tag->ident->tk_val.token_value.ptr, tag->s_or_u);
		if (tag->struct_decl != NULL) {
			insert_to_user_define_type(g_curr_scope->tags_tail, tag->ident->tk_val.token_value.ptr, tag_type, TRUE);
		} else {
			insert_to_user_define_type(g_curr_scope->tags_tail, tag->ident->tk_val.token_value.ptr, tag_type, FALSE);
		}
	}
	
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
		add_field_list_to_tag(tag_type, field_list);
		declaration = declaration->next;
	}

	return tag_type;
}

type_t *get_decl_spec_type(decl_spec_t *spec)
{
	int store_cls_tk;
	int type_spec_tk;

	type_t *base_type, *type;
	type = base_type = NULL;

	switch (spec->type_spec->kind) {
	case TK_VOID:
		base_type = g_ty_void;
		break;
	case TK_CHAR:
		base_type = g_ty_char;
		break;
	case TK_SHORT:
		base_type = g_ty_short;
		break;
	case TK_INT:
		base_type = g_ty_int;
		break;
	case TK_LONG:
		base_type = g_ty_long;
		break;
	case TK_FLOAT:
		base_type = g_ty_float;
		break;
	case TK_DOUBLE:
		base_type = g_ty_double;
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
			base_type = g_ty_int;
		}
	}

	type = derive_decl_spec_type(base_type, spec->type_qual->qual, spec->type_spec->sign, spec->store_cls->kind);
	return type;
}