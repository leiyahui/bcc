#ifndef _TYPE_H
#define _TYPE_H
#include "bcc.h"

enum {
	TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_LONG,
	TYPE_FLOAT,TYPE_DOUBLE, TYPE_POINTER, TYPE_ARRAY, 
	TYPE_STRUCT, TYPE_UNION,TYPE_ENUM,TYPE_FUNCTION
	};


typedef struct _type_t {
	BOOL is_const;
	int kind;
	int size;
	int align;
	struct _type_t *base_type;
}type_t;

typedef struct _base_data_type_t {
	type_t type;
	char *name;
}base_data_type;

typedef struct _field_t {
	type_t* type;
	char *name;
	int bits;
	struct _field_t *next;
}field_t;

typedef struct _tag_t {
	int type;
	char *name;
	field_t *head;
	field_t *tail;
}tag_t;

typedef struct _enum_t {
	type_t type;
	char *name;
}enum_t;

typedef struct _param_type_t {
	type_t *type;
	char *name;
	struct _param_type_t *next;
}param_type_t;

typedef struct _function_type_t {
	type_t *ret;
	char *name;
	field_t *head;
	field_t *tail;
}function_type_t;

/*func*/

void init_base_type();

tag_t* create_tag(char *name, int struct_or_union);

void add_field_to_tag(tag_t *tag, type_t *type, char *name, int bits);

function_type_t* create_func_type(char *name, type_t *ret);

void add_param_to_func(function_type_t *func, type_t *type, char *name);

#endif