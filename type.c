#include "bcc.h"

#define BASE_TYPE_NUM 8

type_t g_base_type[BASE_TYPE_NUM];

#define INIT_ONE_BASE_TYPE(type, size)				\
			g_base_type[type].kind = type;			\
			g_base_type[type].align = size;			\
			g_base_type[type].size = size;			\
			g_base_type[type].base_type = NULL;

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

tag_t * create_tag(char * name, int struct_or_union)
{
	tag_t *type;
	type = (tag_t *)bcc_malloc(sizeof(tag_t));
	type->name = name;
	type->type = struct_or_union;
	type->head = type->tail = NULL;
}

void add_field_to_tag(tag_t *tag, type_t *type, char *name, int bits)
{
	field_t *field;
	field = (field_t *)bcc_malloc(sizeof(field_t));
	
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
	field_t * param;
	param = (field_t * *)bcc_malloc(sizeof(field_t));
	
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
