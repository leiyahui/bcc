#ifndef _SYNTAX_H
#define _SYNTAX_H
#include "bcc.h"

#define NODE_TOKEN_KIND(node)	node->tk_val.tk_kind
#define NODE_TOKEN_VALUE(node)	node->tk_val.token_value

typedef union _ast_node_t {
	token_t tk_val;
}ast_node_t;

typedef struct _store_cls_spec_t {
	ast_node_t *value;
}store_cls_spec_t;

#define TYPE_SPEC_BASIC_TYPE	1
#define TYPE_SPEC_STRUCT_UNION	2
#define TYPE_SPEC_ENUM_SPEC		3
#define TYPE_SPEC_TYPEDEF		4

typedef struct _type_spec_t {
	int kind;
	ast_node_t *value;
}type_spec_t;

typedef struct _type_qual_t {
	ast_node_t *const_tk;
	ast_node_t *volatile_tk;
}type_qual_t;

typedef struct _decl_spec_t {
	ast_node_t	*store_cls;
	ast_node_t	*type_spec;
	ast_node_t	*type_qual;
}decl_spec_t;


typedef struct _pointer_t {
	ast_node_t *value;
	ast_node_t *type_qual_ptr;
	ast_node_t *next;
}pointer_t;

typedef struct _initializer_t {
	ast_node_t *assign_expr;
	ast_node_t *initialier_list;
	ast_node_t *next;
}initializer_t;

typedef struct _assign_op_t {
	ast_node_t *value;
}assign_op_t;

typedef struct _assign_expr_t {
	ast_node_t *cond_expr;
	ast_node_t *assign_op;
	ast_node_t *assign_expr;
	ast_node_t *next;
	double value;
}assign_expr_t;

typedef struct _expr_t {
	ast_node_t *assign_expr;
	ast_node_t *op;
	ast_node_t *expr;
	double value;
}expr_t;

#define PRI_EXPR_TOKEN	0
#define PRI_EXPR_EXPR	1

typedef struct _primary_expr_t {
	int kind;
	ast_node_t *expr;
	double value;
}primary_expr_t;

typedef struct _argu_expr_list_t {
	ast_node_t *assign_expr;
	ast_node_t *argu_expr_list;
}argu_expr_list_t;


#define ARRAY_POSTFIX		0
#define FUNC_POSTFIX		1
#define STRCTURE_POSTFIX	2
#define INC_DEC_POSTFIX		3


typedef struct _postfix_t {
	int kind;
	ast_node_t *expr;
	ast_node_t *op;
	ast_node_t *ident;
	ast_node_t *next;
	double value;
}postfix_t;

typedef struct _postfix_expr_t {
	ast_node_t *primary_expr;
	ast_node_t *postfix;
}postfix_expr_t;

#define POSTFIX_EXPR	0
#define UNARY_EXPR		1
#define CAST_EXPR		2
#define TYPE_NAME		3

typedef struct _type_name_t {
	ast_node_t *spec_qual_list;
	ast_node_t *abs_decl;
}type_name_t;

typedef struct _unary_expr_t {
	int expr_kind;
	ast_node_t *expr;
	ast_node_t *op;
}unary_expr_t;

typedef struct _cast_expr_t {
	BOOL is_unary_expr;
	ast_node_t *expr;
	ast_node_t *type_name;
}cast_expr_t;

typedef struct _binary_expr_t {
	ast_node_t *op1;
	ast_node_t *next;
	ast_node_t *op;
}binary_expr_t;

typedef struct _cond_expr_t {
	ast_node_t *logic_or_expr;
	ast_node_t *expr;
	ast_node_t *cond_expr;
	ast_node_t *que_op;
	ast_node_t *colon_op;
}cond_expr_t;

typedef struct _const_expr_t {
	ast_node_t *cond_expr;
	double value;			//if constant expr is all number, it's calculated when parse
}const_expr_t;

typedef struct _param_type_list_t {
	ast_node_t *param_list;
	ast_node_t *ellipsis;
}param_type_list_t;

typedef struct _ident_list_t {
	ast_node_t *ident;
	ast_node_t *next;
}ident_list_t;

#define PAREN		1
#define BRACKET		2
typedef struct _decl_postfix_t {
	int paren_or_barcket;
	ast_node_t *const_expr;
	ast_node_t *param_list;
	ast_node_t *ident_list;
	ast_node_t *next;
}decl_postfix_t;

typedef struct _direct_declarator_t {
	BOOL is_abs_decl;
	ast_node_t *ident;
	ast_node_t *decl;
	decl_postfix_t *post;
}direct_declarator_t;


#define DECLARATOR			1 << 1
#define INIT_DECLARATOR		1 << 2
#define STRUCT_DECLARATOR	1 << 3
#define ABS_DECLARATOIR		1 << 4

typedef struct _declarator_t {
	int kind;
	ast_node_t *pointer;
	ast_node_t *direct_declarator;
	ast_node_t *const_expr;
	ast_node_t *initializer;
	ast_node_t *next;
}declarator_t;

#define DECLARATION			1 << 1
#define STRUCT_DECLARATION	1 << 2
#define PARAM_DECLARATION	1 << 3

typedef struct _declaration_t {
	int kind;
	ast_node_t *decl_spec;
	ast_node_t *declarator_list;
	ast_node_t *next;
}declaration_t;


typedef struct _struct_or_union_spec_t {
	ast_node_t *s_or_u;
	ast_node_t *ident;
	ast_node_t *struct_decl;
}struct_or_union_spec_t;

#define CONST_EXPR	0
#define PARAM_LIST	1
#define IDENT_LIST	2

typedef struct _enumerator_t {
	ast_node_t *ident;
	ast_node_t *const_expr;
	ast_node_t *op;
	ast_node_t *next;
}enumerator_t;

typedef struct _enum_spec_t {
	ast_node_t *enum_keyword;
	ast_node_t *ident;
	ast_node_t *enum_decl;
}enum_spec_t;

typedef struct _expr_statement_t {
	ast_node_t *expr;
}expr_statement_t;

typedef struct _labeled_statement_t {
	ast_node_t *token;
	ast_node_t *const_expr_ptr;
	ast_node_t *statement;
}labeled_statement_t;

typedef struct _select_statement_t {
	ast_node_t *fir_token;
	ast_node_t *sec_token;
	ast_node_t *fir_expr;
	ast_node_t *sec_expr;
	ast_node_t *statement;
}select_statement_t;

typedef struct _iteration_statement_t {
	ast_node_t *fir_token;
	ast_node_t *sec_token;
	ast_node_t *expr;
	ast_node_t *statement;
	ast_node_t *expr_statement1;
	ast_node_t *expr_statement2;
}iteration_statement_t;

typedef struct _jump_statement_t {
	ast_node_t *jmp_cmd;
	ast_node_t *value;
}jump_statement_t;


#define LABELED_STATEMENT	0
#define COMPOUND_STATEMENT	1
#define EXPR_STATEMENT		2
#define SELECT_STATEMENT	3
#define ITERAT_STATEMENT	4
#define JUMP_STATMENT		5

typedef struct _statement_t {
	int node_kind;
	ast_node_t *statement;
	ast_node_t *next;
}statement_t;

typedef struct _statement_list_t {
	ast_node_t *statement;
	ast_node_t *next_statement;
}statement_list_t;

typedef struct _comp_state_t {
	ast_node_t *statement_list;
	ast_node_t *decl_list;
}comp_state_t;

typedef struct _func_def_t {
	ast_node_t *decl_spec_ptr;
	ast_node_t *decl_ptr;
	ast_node_t *decl_list_ptr;
	ast_node_t *comp_state_ptr;
}func_def_t;


ast_node_t *parse_enum();
ast_node_t *parse_struct_union();


typedef struct _tdname_t {
	char *name;
	int level;
}tdname_t;

typedef struct _coordinate_t {
	char *g_cursor;
	int line;
	int colum;
}coordinate_t;

#endif