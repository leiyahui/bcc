#include "bcc.h"

type_t *g_ty_void = &(type_t){ TYPE_VOID, 0, 0, TRUE };
type_t *g_ty_char = &(type_t) { TYPE_CHAR, 1, 1, TRUE };
type_t *g_ty_short = &(type_t) { TYPE_SHORT, 2, 2, TRUE };
type_t *g_ty_int = &(type_t) { TYPE_INT, 4, 4, TRUE };
type_t *g_ty_long = &(type_t) { TYPE_LONG, 8, 8, TRUE };
type_t *g_ty_float = &(type_t) { TYPE_FLOAT, 4, 4, TRUE };
type_t *g_ty_double = &(type_t) { TYPE_DOUBLE, 8, 8, TRUE };
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

void add_field_to_tag(tag_type_t *tag, type_t *type, char *name ,int bits)
{
	field_t *field;

	field = (field_t*)bcc_malloc(sizeof(field_t));
	field->name = name;
	field->bits = bits;

	if (tag->head == NULL) {
		tag->head = tag->tail = field;
	}
	else {
		tag->tail->next = field;
		tag->tail = field;
	}
}

type_t *get_tag_member_type(tag_type_t *tag, char *name)
{
	field_t *tmp_field;

	tmp_field = tag->head;
	while (tmp_field != NULL) {
		if (tmp_field->name == name) {
			return tmp_field->type;
		}
		tmp_field = tmp_field->next;
	}
	return NULL;
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

type_t *do_integer_promotion(type_t *type)
{
	if (type->size <= 4) {		//int size
		return g_ty_int;
	}
	return type;
}

type_t *type_conv(type_t *type)
{
	type_t *new_type;
	switch (type->kind)
	{
	case TYPE_ARRAY:
		return derive_pointer_type(type->base_type, 0);
	case TYPE_FUNCTION:
		return derive_pointer_type(type, 0);
	default:
		return do_integer_promotion(type);
	}
}

