#ifndef _TYPE_H
#define _TYPE_H
#include "bcc.h"

enum {
	TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_LONG,
	TYPE_FLOAT,TYPE_DOUBLE, TYPE_POINTER, TYPE_ARRAY, 
	TYPE_STRUCT, TYPE_UNION, TYPE_ENUM, TYPE_PARAMETER, TYPE_FUNCTION
	};

#define EXTERN_STORE_CLS	1
#define STATIC_STORE_CLS	2
#define AUTO_STORE_CLS		3
#define REGISTER_STORE_CLS	4

#define WITH_CONST		0x1 << 1
#define WITH_VOLATILE	0x1 << 2

typedef struct _type_t {
	int qual;
	int store_cls;
	int kind;
	int size;
	int align;
	union {
		field_type_t *field_type;
		tag_type_t *tag_type;
		td_type_t *td_type;
		param_type_t *param_type;
		function_type_t *func_type;
	};
	struct _type_t *base_type;
}type_t;

typedef struct _field_type_t {
	type_t* type;
	char *name;
	int bits;
	struct _field_type_t *next;
}field_type_t;

typedef struct _tag_type_t {
	int type;
	char *name;
	field_type_t *head;
	field_type_t *tail;
}tag_type_t;

typedef struct _enum_t {
	type_t type;
	char *name;
}enum_t;

typedef struct _td_type_t {
	type_t *type;
	type_t *name;
}td_type_t;

typedef struct _param_type_t {
	type_t *type;
	char *name;
	struct _param_type_t *next;
}param_type_t;

typedef struct _function_type_t {
	type_t *ret;
	char *name;
	field_type_t *head;
	field_type_t *tail;
}function_type_t;

/*func*/

void init_base_type();

tag_type_t* create_tag(char *name, int struct_or_union);

void add_field_to_tag(tag_type_t *tag, type_t *type, char *name, int bits);

function_type_t* create_func_type(char *name, type_t *ret_type);

void add_param_to_func(function_type_t *func, type_t *type, char *name);

td_type_t *create_td_type(char *name, type_t *type);



/*get_get*/
type_t *get_postfix_declarator_type(decl_postfix_t *decl_postfix);

type_t *get_declarator_type(declarator_t *decl);

type_t *get_struct_declarator_type(struct_declarator_t *decl);

type_t *get_struct_union_type(type_spec_t *type_spec);

type_t *get_decl_spec_type(decl_spec_t *spec);

#endif