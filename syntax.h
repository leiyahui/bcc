#ifndef _SYNTAX_H
#define _SYNTAX_H
#include "bcc.h"

#define NODE_TOKEN_KIND(node)	node->tk_val.tk_kind
#define NODE_TOKEN_VALUE(node)	node->tk_val.token_value

struct _assign_expr_t;
struct _cond_expr_t;
struct _declarator_t;
struct _declaration_t;

enum expr_kind {
	AST_VAR, AST_STR, OP_EXPR, AST_CONST,		//primary expr
	AST_FUNC, AST_ARRAY, AST_POINTER_TO, AST_CONTAIN, AST_POST_INC, AST_POST_DEC,	//postfix expr
	AST_PRFIX_INC, AST_PRIFX_DEC, AST_ADDR, AST_DREF, AST_UNARY_PLUS, AST_UNARY_MINUS, AST_BITREVERT, AST_NOT, AST_SIZEOF, //unary_expr
	AST_CAST,		//cast expr
	//binary expr
	AST_MULTI, AST_DIVIDE, AST_MOD, AST_ADD, AST_SUB, AST_LEFT, AST_RIGHT, AST_LESS, AST_LESS_EQUAL, AST_GREAT, AST_GREAT_EQUAL,
	AST_EQUAL, AST_NEQUAL, AST_BIT_AND, AST_XOR, AST_BIT_OR, AST_AND, AST_OR,
	AST_COND_EXPR,	//condition expr
	AST_ASSIGN_EXPR,	//assign expr
	AST_COMMA_EXPR,		//comma expr
	AST_INIT, AST_DECL,
	AST_LABELED, AST_COMPOUND, AST_EXPR, AST_SELECT, AST_ITERAT, AST_JUMP,
};

typedef union _ast_node_t {
	token_t tk_val;
}ast_node_t;

typedef struct _store_cls_spec_t {
	int kind;
}store_cls_spec_t;

#define TYPE_SPEC_BASIC_TYPE	1
#define TYPE_SPEC_STRUCT_UNION	2
#define TYPE_SPEC_ENUM_SPEC		3
#define TYPE_SPEC_TYPEDEF		4

typedef struct _type_spec_t {
	int kind;
	int sign;
	ast_node_t *value;
}type_spec_t;


#define WITH_CONST		0x1 << 1
#define WITH_VOLATILE	0x1 << 2

typedef struct _type_qual_t {
	int qual;
}type_qual_t;

typedef struct _decl_spec_t {
	store_cls_spec_t	*store_cls;
	type_spec_t			*type_spec;
	type_qual_t			*type_qual;
}decl_spec_t;


typedef struct _pointer_t {
	type_qual_t *type_qual_ptr;
	struct _pointer_t *next;
}pointer_t;

typedef struct _initializer_t {
	struct _assign_expr_t *assign_expr;
	struct _initializer_t *initialier_list;
	struct _initializer_t *next;
}initializer_t;


typedef struct _expr_t {
	int kind;
	double value;
	type_t *type;
	struct _expr_t *child_1;
	struct _expr_t *child_2;
}expr_t;

typedef struct _cond_expr_t {
	expr_t expr;
	expr_t *logical_or;
}cond_expr_t;

typedef struct _init_ast_node_t {
	int kind;
	type_t *type;
	int offset;
	expr_t *expr;
}init_ast_node_t;

typedef struct _decl_node_t {
	int kind;
	char *name;
	init_ast_node_t *init_node;
}decl_node_t;




#define AST_LABELED_STATEMENT	0
#define AST_COMPOUND_STATEMENT	1
#define AST_EXPR_STATEMENT		2
#define AST_SELECT_STATEMENT	3
#define AST_ITERAT_STATEMENT	4
#define AST_JUMP_STATMENT		5

typedef struct _statement_t {
	int node_kind;
	ast_node_t *statement;
	struct _expr_statement_t *next;
}statement_t;

typedef struct _labeled_statement_t {
	ast_node_t *token;
	const_expr_t *const_expr;
	statement_t *statement;
}labeled_statement_t;

#define IF_SELECTION		1
#define IF_ELSE_SELECTION	2
#define SWITCH_SELECTION	3

typedef struct _select_statement_t {
	int kind;
	expr_t *fir_expr;
	expr_t *sec_expr;
	statement_t *statement;
}select_statement_t;

typedef struct _iteration_statement_t {
	int kind;
	expr_t *expr;
	statement_t *statement;
	expr_statement_t *expr_statement1;
	expr_statement_t *expr_statement2;
}iteration_statement_t;

typedef struct _jump_statement_t {
	int kind;
	ast_node_t *value;
}jump_statement_t;


typedef struct _comp_state_t {
	statement_t *statement_list;
	declaration_t *decl_list;
}comp_state_t;

typedef struct _func_def_t {
	decl_spec_t *decl_spec;
	declarator_t *decl;
	declaration_t *decl_list;
	comp_state_t *comp_state_ptr;
}func_def_t;

typedef struct _coordinate_t {
	char *g_cursor;
	int line;
	int colum;
}coordinate_t;

#endif