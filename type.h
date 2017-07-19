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
	char *id;
};
typedef struct _field_t {
	type_t type;
	char *name;
	int bits;
	struct _field_t *next;
}field_t;

typedef struct _struct_enum_type_t {
	int type;
	char *name;
	field_t *head;
	field_t *tail;
};

typedef struct _enum_t {
	type_t type;
	char *name;
};


#endif