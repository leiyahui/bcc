#ifndef _TYPE_H
#define _TYPE_H
#include "bcc.h"

enum {
	TYPE_VOID,
	TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_LONG,
	TYPE_FLOAT,TYPE_DOUBLE, TYPE_POINTER, TYPE_ARRAY, 
	TYPE_STRUCT, TYPE_UNION, TYPE_ENUM, TYPE_PARAMETER, TYPE_FUNCTION
};

#define EXTERN_STORE_CLS	1
#define STATIC_STORE_CLS	2
#define AUTO_STORE_CLS		3
#define REGISTER_STORE_CLS	4

typedef struct _type_t {
	int qual;
	int store_cls;
	int kind;
	int size;
	int align;
	int sign;
	struct _type_t *base_type;
}type_t;

typedef struct _field_t {
	type_t *type;
	char *name;
	int bits;
	struct _field_t *next;
}field_t;

typedef struct _field_list_t {
	type_t* type;
	field_t *head;
	field_t *tail;
	struct _field_list_t *next;
}field_list_t;

typedef struct _tag_type_t {
	type_t *type;
	char *name;
	field_list_t *head;
	field_list_t *tail;
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
	param_type_t *head;
	param_type_t *tail;
	BOOL with_ellipse;
}function_type_t;

/*func*/

void init_base_type();

tag_type_t* create_tag(char *name, int struct_or_union);

field_list_t *create_field_list(type_t *type);

void add_field_to_same_type_list(field_list_t *field, type_t *type, char *name, int bits);

void add_field_list_to_tag(tag_type_t *tag, field_list_t *list);

function_type_t* create_func_type(char *name, type_t *ret_type);

void add_param_to_func(function_type_t *func, type_t *type, char *name);

td_type_t *create_td_type(char *name, type_t *type);



/*get_get*/
type_t *get_declaration_base_type(declaration_t *decl);

type_t *get_declarator_type(type_t *base_type, declarator_t *decl);

type_t *get_struct_union_type(type_spec_t *type_spec);

type_t *get_decl_spec_type(decl_spec_t *spec);

#endif