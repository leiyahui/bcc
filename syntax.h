#ifndef _SYNTAX_H
#define _SYNTAX_H
#include "bcc.h"

typedef struct _ast_node_t {
	int kind;
}ast_node_t;

typedef struct _store_cls_spec_t {
	int node_kind;
	token_t token;
}store_cls_spec_t;

typedef struct _type_spec_t {
	int node_kind;
	union {
		token_t token;
		ast_node_t *s_or_u_spec;
		ast_node_t *enum_spec;
	}type;
}type_spec_t;

typedef struct _type_qual_t {
	int node_kind;
	token_t token;
}type_qual_t;

typedef struct _decl_spec_t {
	int node_kind;
	ast_node_t	*store_cls_ptr;
	ast_node_t		*type_spec_ptr;
	ast_node_t		*type_qual_ptr;
}decl_spec_t;

typedef struct _spec_qual_list_t {
	int node_kind;
	ast_node_t *type_sepc_ptr;
	ast_node_t *type_qual_ptr;
}spec_qual_list_t;

typedef struct _pointer_t {
	int node_kind;
	ast_node_t *type_qual_ptr;
	ast_node_t *point_next;
}pointer_t;

typedef struct _initializer_t {
	int node_kind;
	union {
		ast_node_t *assign_expr_ptr;
		ast_node_t *initializer_list_ptr;
	};
}initializer_t;

typedef struct _initializer_list_t {
	int node_kind;
	ast_node_t *initializer_ptr;
	ast_node_t *next_initial;
}initializer_list_t;

typedef struct _assign_op_t {
	int node_kind;
	token_t token;
}assign_op_t;

typedef struct _assign_expr_t{
	int node_kind;
	ast_node_t *cond_exp_ptr;
	ast_node_t *unary_expr_ptr;
	ast_node_t *assign_op_ptr;
	ast_node_t *assign_expr_ptr;
}assign_expr_t;

typedef struct _expr_t {
	int node_kind;
	ast_node_t *assign_expr_ptr;
	ast_node_t *expr_ptr;
}expr_t;

typedef struct _primary_expr_t {
	int node_kind;
	union {
		token_t token;
		ast_node_t *expr_ptr;
	};
}primary_expr_t;

typedef struct _argu_expr_list_t{
	ast_node_t *assign_expr_ptr;
	ast_node_t *argu_expr_list_ptr;
}argu_expr_list_t;

typedef struct _postfix_expr_t {
	int node_kind;
	union {
		ast_node_t *primary_expr_ptr;
		ast_node_t *postfix_expr_ptr;
	};
	union {
		ast_node_t *expr_ptr;
		ast_node_t* argu_expr_list_ptr;
		token_t token;
	};
}postfix_expr_t;

typedef struct _unary_expr_t {
	union {
		ast_node_t *postfix_expr_ptr;
		ast_node_t *unary_expr_ptr;
		struct {
			ast_node_t *unary_op_ptr;
			ast_node_t *cast_expr_ptr;
		};
		ast_node_t *type_name_ptr;
	};
}unary_expr_t;

typedef struct _cast_expr_t {
	union {
		ast_node_t *unary_expr_ptr;
		struct {
			ast_node_t *type_name_ptr;
			ast_node_t *cast_expr_ptr;
		};
	};
}cast_expr_t;

typedef struct _multi_expr_t {
	ast_node_t *cast_expr_ptr;
	ast_node_t *multi_expr_ptr;
}multi_expr_t;

typedef struct _addit_expr_t {
	ast_node_t *multi_expr_ptr;
	ast_node_t *addit_expr_ptr;
}addit_expr_t;

typedef struct _shift_expr_t {
	ast_node_t *addit_expr_ptr;
	ast_node_t *shift_expr_ptr;
}shift_expr_t;

typedef struct _rela_expr_t {
	ast_node_t *shift_expr_ptr;
	ast_node_t *rela_expr_ptr;
}rela_expr_t;

typedef struct _equal_expr_t {
	ast_node_t *rela_expr_ptr;
	ast_node_t *euqal_expr_ptr;
}equal_expr_t;

typedef struct _and_expr_t {
	ast_node_t *equal_expr_ptr;
	ast_node_t *and_expr_ptr;
}and_expr_t;

typedef struct _exclusive_or_expr_t {
	ast_node_t *and_expr_ptr;
	ast_node_t *exclu_or_expr_ptr;
}exclusive_or_expr_t;

typedef struct _inclusive_or_expr_t{
	ast_node_t *exclu_or_expr_ptr;
	ast_node_t *inclu_or_expr_ptr;
}inclusive_or_expr_t;

typedef struct _logic_and_expr_t {
	ast_node_t *inclusive_or_expr_ptr;
	ast_node_t *logic_and_expr_ptr;
}logic_and_expr_t;

typedef struct _logic_or_expr_t {
	ast_node_t *logic_or_expr_ptr;
	ast_node_t *logic_and_expr_ptr;
}logic_or_expr_t;

typedef struct _cond_expr_t {
	ast_node_t *logic_or_expr_ptr;
	ast_node_t *expr_ptr;
	ast_node_t *cond_expr_ptr;
}cond_expr_t;

typedef struct _const_expr_t {
	ast_node_t *cond_expr_ptr;
}const_expr_t;

typedef struct _type_name_t {
	ast_node_t *spec_qual_list_ptr;
	ast_node_t *abstract_declarator_ptr;
}type_name_t;

typedef struct _direct_abstract_declarator_t {
	ast_node_t *abstract_declarator_ptr;
	ast_node_t *direct_abstract_declarator_ptr;
	ast_node_t *const_expr_ptr;
	ast_node_t *param_type_list_ptr;
}direct_abstract_declarator;

typedef struct _abstract_declarator_t{
	pointer_t *poninter_ptr;
	direct_abstract_declarator *direct_abstract_declarator_ptr;
}abstract_declarator_t;

typedef struct _param_declaration_t {
	ast_node_t *decl_spec_ptr;
	union {
		ast_node_t *declarator_ptr;
		ast_node_t *abstract_declarator_ptr;
	};
}param_declaration_t;

typedef struct _param_list_t {
	ast_node_t *param_declaration_ptr;
	ast_node_t *param_list_ptr;
}param_list_t;

typedef struct _param_type_list_t {
	ast_node_t *param_list_ptr;
	token_t token;
}param_type_list_t;

typedef struct _ident_list_t {
	int node_kind;
	token_t token;
	ast_node_t *ident_list_ptr;
}ident_list_t;

typedef struct _direct_declarator_t {
	int node_kind;
	union {
		token_t token;
		ast_node_t *declarator_ptr;
		struct {
			ast_node_t *direct_declarator_ptr;
			union {
				ast_node_t *const_expr_ptr;
				ast_node_t *param_list_ptr;
				ast_node_t *ident_list;
			};
		};
	};
}direct_declarator_t;

typedef struct _declarator_t {
	int node_kind;
	ast_node_t *pointer_ptr;
	ast_node_t *direct_declarator_ptr;
}declarator_t;

typedef struct _struct_declarator_list_t {
	int node_kind;
	ast_node_t *declarator_ptr;
	ast_node_t *const_expr_ptr;
	ast_node_t *next;
}struct_declarator_list_t;

typedef struct _struct_declaration_t {
	int node_kind;
	ast_node_t *spec_qual_list_ptr;
	ast_node_t *struct_decl_list_ptr;
	ast_node_t *next;
}struct_declaration_t;

typedef struct _struct_or_union_spec_t {
	int node_kind;
	token_t s_or_u;
	token_t identifier;
	ast_node_t *struct_decl_ptr;
}struct_or_union_spec_t;

typedef struct _enumerator_t {
	int node_kind;
	token_t identifier;
	ast_node_t *const_expr_ptr;
	ast_node_t *next;
}enumerator_t;

typedef struct _enum_spec_t {
	int node_kind;
	token_t identifier;
	ast_node_t *enum_decl_ptr;
}enum_spec_t;

typedef struct _init_declarator_t
{
	int node_kind;
	ast_node_t *declarator_ptr;
	ast_node_t *initializer_ptr;
}init_declarator_t;

typedef struct _declaration_t {
	int node_kind;
	ast_node_t *decl_spec_ptr;
	ast_node_t *init_decl_ptr;
}declaration_t;

typedef struct _declaration_list_t {
	int node_kind;
	ast_node_t *declaration_ptr;
	ast_node_t *next_declaration;
}declaration_list_t;







typedef struct _expr_statement_t {
	ast_node_t *expr_ptr;
}expr_statement_t;

typedef struct _labeled_statement_t {
	int node_kind;
	token_t token;
	ast_node_t *const_expr_ptr;
	ast_node_t *statement_ptr;
}labeled_statement_t;

typedef struct _select_statement_t {
	ast_node_t *expr_ptr;
	ast_node_t *statement_ptr;
}select_statement_t;

typedef struct _iteration_statement_t {
	ast_node_t *expr_ptr;
	ast_node_t *statement_ptr;
	ast_node_t *expr_statement_ptr1;
	ast_node_t *expr_statement_ptr2;
}iteration_statement_t;

typedef struct _jump_statement_t {
	int node_kind;
	token_t jmp_cmd;
	token_t token;
	ast_node_t *expr_ptr;
}jump_statement_t;

typedef struct _statement_t {
	int node_kind;
	union {
		ast_node_t *labled_ptr;
		ast_node_t *comp_ptr;
		ast_node_t *expre_ptr;
		ast_node_t *select_ptr;
		ast_node_t *iter_ptr;
		ast_node_t *jump_ptr;
	};
}statement_t;

typedef struct _statement_list_t {
	int node_kind;
	ast_node_t *statement_ptr;
	ast_node_t *next_statement;
}statement_list_t;

typedef struct _comp_state_t {
	int node_kind;
	ast_node_t *statement_list_ptr;
	ast_node_t *decl_list_ptr;
}comp_state_t;

typedef struct _func_def_t {
	int node_kind;
	ast_node_t *decl_spec_ptr;
	ast_node_t *decl_ptr;
	ast_node_t *decl_list_ptr;
	ast_node_t *comp_state_ptr;
};

#endif