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
	int kind;
	int size;
	int align;
	int sign;		//unsigned is false, signed is true;
	int qual;
	int store_cls;
	struct _type_t *base_type;
}type_t;

typedef struct _field_t {
	type_t *type;
	char *name;
	int bits;
	struct _field_t *next;
}field_t;

typedef struct _tag_type_t {
	type_t type;
	char *name;
	field_t *head;
	field_t *tail;
}tag_type_t;

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
	type_t type;
	type_t *ret;
	char *name;
	param_type_t *head;
	param_type_t *tail;
	BOOL with_ellipse;
}function_type_t;

/*func*/

extern type_t *g_ty_void;
extern type_t *g_ty_char;
extern type_t *g_ty_short;
extern type_t *g_ty_int;
extern type_t *g_ty_long;
extern type_t *g_ty_float;
extern type_t *g_ty_double;
extern type_t *g_ty_uchar;
extern type_t *g_ty_ushort;
extern type_t *g_ty_uint;
extern type_t *g_ty_ulong;
extern type_t *g_ty_ufloat;
extern type_t *g_ty_udouble;

tag_type_t* create_tag_type(char *name, int struct_or_union);

void add_field_to_tag(tag_type_t *tag, type_t *type, char *name, int bits);

type_t *get_tag_member_type(tag_type_t *tag, char *name);

function_type_t* create_func_type(char *name, type_t *ret_type);

void add_param_to_func(function_type_t *func, type_t *type, char *name);


/*get_get*/
type_t *get_declaration_base_type(declaration_t *decl);

type_t *get_declarator_type(type_t *base_type, declarator_t *decl);

type_t *get_struct_union_type(type_spec_t *type_spec);

type_t *get_decl_spec_type(decl_spec_t *spec);

void add_declaration_to_sym_table(declaration_t *declaration);

type_t *type_conv(type_t *type);
#endif