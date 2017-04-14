#ifndef _SYNTAX_H
#define _SYNTAX_H
#include "bcc.h"

#define TOKEN	1
#define NODE	2

typedef union _ast_node_t {
	token_t tk_val;
}ast_node_t;

typedef struct _store_cls_spec_t {
	ast_node_t *value;
}store_cls_spec_t;

#define TYPE_SPEC_BASIC_TYPE	1
#define TYPE_SPEC_STRUCT_UNION	2
#define TYPE_SPEC_ENUM_SPEC		3

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

typedef struct _spec_qual_list_t {
	ast_node_t *type_sepc;
	ast_node_t *type_qual;
}spec_qual_list_t;

typedef struct _pointer_t {
	ast_node_t *type_qual_ptr;
	ast_node_t *point_next;
}pointer_t;

#define ASSIGN_EXPR			0
#define INITIALIZER_LIST	1

typedef struct _initializer_t {
	int kind;
	ast_node_t *value;
}initializer_t;

typedef struct _initializer_list_t {
	ast_node_t *initializer;
	ast_node_t *next_init;
}initializer_list_t;

typedef struct _assign_op_t {
	ast_node_t *value;
}assign_op_t;

typedef struct _assign_expr_t{
	int is_cond_expr;
	ast_node_t *cond_expr;
	ast_node_t *unary_expr;
	ast_node_t *assign_op;
	ast_node_t *assign_expr;
}assign_expr_t;

typedef struct _expr_t {
	ast_node_t *assign_expr;
	ast_node_t *expr;
}expr_t;

#define PRI_EXPR_TOKEN	0
#define PRI_EXPR_EXPR	1

typedef struct _primary_expr_t {
	int kind;
	ast_node_t *expr;
}primary_expr_t;

typedef struct _argu_expr_list_t{
	ast_node_t *assign_expr;
	ast_node_t *argu_expr_list;
}argu_expr_list_t;

#define POSTFIX_WITH_EXPR		0
#define POSTFIX_WITH_ARGUMNET	1
#define POSTFIX_WITH_OP			2
#define POSTFIX_WITH_IDENT		3

typedef struct _postfix_expr_t {
	BOOL		is_primary_expr;
	int			postfix_with;
	ast_node_t	*primary_expr;
	ast_node_t	*postfix_expr;
	ast_node_t	*op;
	ast_node_t	*ident;
}postfix_expr_t;

#define POSTFIX_EXPR	0
#define UNARY_EXPR		1
#define CAST_EXPR		2
#define TYPE_NAME		3


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

typedef struct _multi_expr_t {
	ast_node_t *cast_expr;
	ast_node_t *multi_expr;
	ast_node_t *op;
}multi_expr_t;

typedef struct _addit_expr_t {
	ast_node_t *multi_expr;
	ast_node_t *addit_expr;
	ast_node_t *op
}addit_expr_t;

typedef struct _shift_expr_t {
	ast_node_t *addit_expr;
	ast_node_t *shift_expr;
	ast_node_t *op;
}shift_expr_t;

typedef struct _rela_expr_t {
	ast_node_t *shift_expr;
	ast_node_t *rela_expr;
	ast_node_t *op;
}rela_expr_t;

typedef struct _equal_expr_t {
	ast_node_t *rela_expr;
	ast_node_t *euqal_expr;
	ast_node_t *op
}equal_expr_t;

typedef struct _and_expr_t {
	ast_node_t *equal_expr;
	ast_node_t *and_expr;
	ast_node_t *op;
}and_expr_t;

typedef struct _exclusive_or_expr_t {
	ast_node_t *and_expr;
	ast_node_t *exclu_or_expr;
	ast_node_t *op;
}exclusive_or_expr_t;

typedef struct _inclusive_or_expr_t{
	ast_node_t *exclu_or_expr;
	ast_node_t *inclu_or_expr;
	ast_node_t *op;
}inclusive_or_expr_t;

typedef struct _logic_and_expr_t {
	ast_node_t *inclusive_or_expr;
	ast_node_t *logic_and_expr;
	ast_node_t *op;
}logic_and_expr_t;

typedef struct _logic_or_expr_t {
	ast_node_t *logic_or_expr;
	ast_node_t *logic_and_expr;
	ast_node_t *op;
}logic_or_expr_t;

typedef struct _cond_expr_t {
	ast_node_t *logic_or_expr;
	ast_node_t *expr;
	ast_node_t *cond_expr;
	ast_node_t *que_op;
	ast_node_t *colon_op;
}cond_expr_t;

typedef struct _const_expr_t {
	ast_node_t *cond_expr;
}const_expr_t;

typedef struct _type_name_t {
	ast_node_t *spec_qual_list;
	ast_node_t *abstract_declarator;
}type_name_t;

#define WITHOUT_DECL		0
#define WITH_ABSTRACT_DECL	1
#define WITH_DIRECT_DECL	2

typedef struct _direct_abstract_declarator_t {
	int decl_kind;
	BOOL	with_const_expr;
	BOOL	with_param_list;
	ast_node_t *abs_decl;
	ast_node_t *const_expr;
	ast_node_t *param_list;
}direct_abstract_declarator_t;

typedef struct _abstract_declarator_t{
	pointer_t *poninter;
	direct_abstract_declarator_t *direct_abstract_declarator;
}abstract_declarator_t;


#define WITHOUT_DECL		0
#define WITH_DECL			1
#define WITH_ABSTRACT_DECL	2

typedef struct _param_declaration_t {
	int decl_kind;
	ast_node_t *decl_spec;
	ast_node_t *decl;
}param_declaration_t;

typedef struct _param_list_t {
	ast_node_t *param_declaration;
	ast_node_t *param_list;
}param_list_t;

typedef struct _param_type_list_t {
	ast_node_t *param_list;
	ast_node_t *ellipsis;
}param_type_list_t;

typedef struct _ident_list_t {
	ast_node_t *ident;
	ast_node_t *ident_list;
}ident_list_t;

#define CONST_EXPR		0
#define PARAM_TYPE_LIST	1
#define IDENT_LIST		2

typedef struct _direct_declarator_t {
	BOOL with_ident;
	BOOL is_direct_decl;
	int	sec_node_kind;
	ast_node_t *decl;
	ast_node_t *sec_node;
}direct_declarator_t;

typedef struct _declarator_t {
	ast_node_t *pointer;
	ast_node_t *direct_declarator;
}declarator_t;

typedef struct _struct_declarator_list_t {
	ast_node_t *declarator;
	ast_node_t *const_expr;
	ast_node_t *next;
}struct_declarator_list_t;

typedef struct _struct_declaration_t {
	ast_node_t *spec_qual_list;
	ast_node_t *struct_decl_list;
	ast_node_t *next;
}struct_declaration_t;

typedef struct _struct_or_union_spec_t {
	ast_node_t *s_or_u;
	ast_node_t *ident;
	ast_node_t *struct_decl;
}struct_or_union_spec_t;

typedef struct _enumerator_t {
	ast_node_t *ident;
	ast_node_t *const_expr;
	ast_node_t *next;
}enumerator_t;

typedef struct _enum_spec_t {
	ast_node_t *enum_keyword;
	ast_node_t *ident;
	ast_node_t *enum_decl;
}enum_spec_t;

typedef struct _init_declarator_t
{
	ast_node_t *declarator;
	ast_node_t *initializer;
	ast_node_t *next_decl;
}init_declarator_t;

typedef struct _declaration_t {
	ast_node_t *decl_spec;
	ast_node_t *init_decl;
}declaration_t;

typedef struct _declaration_list_t {
	ast_node_t *declaration;
	ast_node_t *next_declaration;
}declaration_list_t;

typedef struct _expr_statement_t {
	ast_node_t *expr;
}expr_statement_t;


typedef struct _labeled_statement_t {
	int label_kind;
	ast_node_t *token;
	ast_node_t *const_expr_ptr;
	ast_node_t *statement;
}labeled_statement_t;


typedef struct _select_statement_t {
	ast_node_t *fir_token;
	ast_node_t *sec_token;
	ast_node_t *expr_ptr;
	ast_node_t *statement_ptr;
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
};

#endif