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
	AST_COMMA_EXPR		//comma expr
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

typedef struct _assign_op_t {
	ast_node_t *value;
}assign_op_t;

typedef struct _assign_expr_t {
	struct _cond_expr_t *cond_expr;
	assign_op_t *assign_op;
	struct _assign_expr_t *assign_expr;
	struct _assign_expr_t *next;
	double value;
}assign_expr_t;

#define PRI_TOKEN	0
#define PRI_EXPR	1

typedef struct _primary_expr_t {
	int kind;
	expr_t *expr;
	double value;
	BOOL compile_evaluated;
}primary_expr_t;

typedef struct _argu_expr_list_t {
	assign_expr_t *assign_expr;
	struct _argu_expr_list_t *argu_expr_list;
}argu_expr_list_t;


typedef struct _expr_t {
	int kind;
	double value;
	type_t *type;
	struct _expr_t *child_1;
	struct _expr_t *child_2;
}expr_t;

typedef struct _init_ast_node_t {
	int kind;
	type_t *type;
	int offset;
	expr_t *expr;
}init_ast_node_t;

#define ARRAY_POSTFIX		0
#define FUNC_POSTFIX		1
#define STRCTURE_POSTFIX	2
#define INC_DEC_POSTFIX		3


typedef struct _postfix_t {
	int kind;
	int op;
	expr_t *expr;
	ast_node_t *ident;
	struct _postfix_t *next;
	double value;
}postfix_t;

typedef struct _postfix_expr_t {
	primary_expr_t *primary_expr;
	postfix_t *postfix;
	double value;
	BOOL compile_evaluted;
}postfix_expr_t;

#define POSTFIX_EXPR	0
#define UNARY_EXPR		1
#define CAST_EXPR		2
#define TYPE_NAME		3

typedef struct _type_name_t {
	type_t *type;
	decl_spec_t *spec_qual_list;
	struct _declarator_t *abs_decl;
}type_name_t;

typedef struct _unary_expr_t {
	int expr_kind;
	expr_t *expr;
	type_name_t *cast_type;
	int op;
	BOOL compile_evaluated;
	double value;			
}unary_expr_t;

typedef struct _cast_expr_t {
	type_name_t *type_name;
	unary_expr_t *unary_expr;
	BOOL compile_evaluated;
	double value;
}cast_expr_t;

typedef struct _binary_expr_t {
	struct _binary_expr_t *op1;
	struct _binary_expr_t *op2;
	int op;
	BOOL compile_evaluated;
	double value;			
}binary_expr_t;

typedef struct _cond_expr_t {
	binary_expr_t *logic_or_expr;
	expr_t *expr;
	struct _cond_expr_t *cond_expr;
	BOOL compile_evaluated;
	double value;			//if constant expr is all number, it's calculated when parse
}cond_expr_t;

typedef struct _const_expr_t {
	cond_expr_t *cond_expr;
	BOOL compile_evaluated;
	double value;			//if constant expr is all number, it's calculated when parse
}const_expr_t;

typedef struct _param_type_list_t {
	struct _declaration_t *param_list;
	BOOL with_ellipse;
}param_type_list_t;

typedef struct _ident_list_t {
	ast_node_t *ident;
	struct _ident_list_t *next;
}ident_list_t;

#define PAREN		1
#define BRACKET		2
typedef struct _decl_postfix_t {
	int paren_or_barcket;
	const_expr_t *const_expr;
	param_type_list_t *param_list;
	ident_list_t *ident_list;
	struct _decl_postfix_t *next;
}decl_postfix_t;

typedef struct _direct_declarator_t {
	BOOL is_abs_decl;
	ast_node_t *ident;
	struct _declarator_t *decl;
	decl_postfix_t *post;
}direct_declarator_t;


#define DECLARATOR			1 << 1
#define INIT_DECLARATOR		1 << 2
#define STRUCT_DECLARATOR	1 << 3
#define ABS_DECLARATOR		1 << 4

typedef struct _declarator_t {
	int kind;
	char *name;
	initializer_t *initializer;
}declarator_t;

#define REGULAR_DECLARATION			1 << 1
#define STRUCT_DECLARATION		1 << 2
#define PARAM_DECLARATION		1 << 3

typedef struct _declaration_t {
	int kind;
	decl_spec_t *decl_spec;
	declarator_t *declarator_list;
}declaration_t;


typedef struct _struct_or_union_spec_t {
	int s_or_u;
	ast_node_t *ident;
	declaration_t *struct_decl;
}struct_or_union_spec_t;

#define CONST_EXPR	0
#define PARAM_LIST	1
#define IDENT_LIST	2

typedef struct _enumerator_t {
	ast_node_t *ident;
	cond_expr_t *const_expr;
	struct _enumerator_t *next;
}enumerator_t;

typedef struct _enum_spec_t {
	ast_node_t *ident;
	enumerator_t *enum_decl;
}enum_spec_t;

typedef struct _expr_statement_t {
	expr_t *expr;
}expr_statement_t;


#define LABELED_STATEMENT	0
#define COMPOUND_STATEMENT	1
#define EXPR_STATEMENT		2
#define SELECT_STATEMENT	3
#define ITERAT_STATEMENT	4
#define JUMP_STATMENT		5

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

#define DECLARATION			1
#define FUNC_DEFINATION		2

typedef struct _external_declaration_t {
	int kind;
	func_def_t *func;
	declaration_t *decl;
}external_declaration_t;



typedef struct _tdname_t {
	char *name;
	int level;
}tdname_t;

typedef struct _coordinate_t {
	char *g_cursor;
	int line;
	int colum;
}coordinate_t;

ast_node_t *parse_expr();
ast_node_t *parse_cast_expr();
ast_node_t *parse_decl_spec(int with_store_cls);
ast_node_t *parse_abs_declarator();
ast_node_t *parse_decl_postfix();
ast_node_t *parse_direct_declarator();
ast_node_t *parse_argu_expr_list();
ast_node_t *parse_pointer();
ast_node_t *parse_declarator(type_t *base_type);
ast_node_t *parse_initializer_list();
ast_node_t *parse_statement();
ast_node_t *parse_compound_statement();
#endif