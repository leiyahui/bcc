#include "bcc.h"


hashtable_t  g_external_symbol_table;
coordinate_t g_recorded_coord;

scope_t g_file_scope;
scope_t *g_current_scope;

//int G_LEVEL = 0;
//vector_t *g_td_table;					//this table is used for typedef name
//vector_t *g_td_overload_table;			//this table is used for overloaded name of typedef name

//tdname_t *create_tdname(char *name)
//{
//	tdname_t *td_name = (tdname_t *)bcc_malloc(sizeof(tdname_t));
//	td_name->level = G_LEVEL;
//	td_name->name = name;
//	
//	return td_name;
//}
//
//void insert_td_table(char *name)
//{
//	tdname_t *td_name = create_tdname(name);
//	insert_vector(g_td_table, td_name);
//}
//
//void insert_td_overload_table(char *name)
//{
//	tdname_t *td_name = create_tdname(name);
//	insert_vector(g_td_overload_table, td_name);
//}
//
//int get_td_level(char *name)
//{
//	int i;
//	tdname_t *tmp_td;
//
//	for (i = g_td_table->len - 1; i >= 0; i--) {
//		tmp_td = g_td_table->data[i];
//		if (tmp_td->name == name) {
//			return tmp_td->level;
//		}
//	}
//	return -1;
//}
//
//int get_td_overload_level(char *name)
//{
//	int i;
//	tdname_t *tmp_td;
//
//	for (i = 0; i < g_td_table->len; i++) {
//		tmp_td = g_td_overload_table->data[i];
//		if (tmp_td->name == name) {
//			return tmp_td->level;
//		}
//	}
//	return -1;
//}
//
//void pop_higher_level_td_table()
//{
//	int i;
//	tdname_t *tmp_td;
//
//	for (i = g_td_table->len - 1; i >= 0; i--) {
//		tmp_td = g_td_table->data;
//		if (tmp_td->level > G_LEVEL) {
//			out_vector(g_td_table);
//		} else {
//			break;
//		}
//	}
//}
//
//void pop_higher_level_td_overload_table()
//{
//	int i;
//	tdname_t *tmp_td;
//
//	for (i = g_td_overload_table->len - 1; i >= 0; i--) {
//		tmp_td = g_td_overload_table->data;
//		if (tmp_td->level > G_LEVEL) {
//			out_vector(g_td_overload_table);
//		} else {
//			break;
//		}
//	}
//}
//
//BOOL is_typedef_name(char *name)
//{
//	int td_level;
//	int td_overload_level;
//	
//	td_level = get_td_level(name);
//	td_overload_level = get_td_overload_level(name);
//
//	if (td_level > td_overload_level) {
//		return TRUE;
//	}
//	return FALSE;
//}

BOOL is_typedef_name(char *name)
{
	scope_t *scope;
	BOOL typedefed;
	int level;

	level = 0;
	typedefed = FALSE;
	scope = g_current_scope;
	while (scope != NULL) {
		level++;
		if (in_namespace(&scope->tdname, name)) {
			typedefed = TRUE;
			break;
		}
		scope = scope->parent;
	}
	if (typedefed == TRUE) {
		scope = g_current_scope;
		while (level > 1) {
			if (in_namespace(&(scope->other_ident), name)) {
				typedefed = FALSE;
				break;
			}
		}
	}
	return typedefed;
}



#define NEXT_TOKEN get_next_token();

#define G_TK_KIND	g_current_token.tk_kind
#define G_TK_VALUE	g_current_token.token_value
#define G_TK_LINE	g_current_token.line

#define EXPECT(tk_kind) if (G_TK_KIND != tk_kind) {			\
							error_message("expect token kind is:%d", tk_kind);				\	
						}

#define SKIP(tk_kind)   if (G_TK_KIND != tk_kind) {			\
							error_message("expect token kind is:%d", tk_kind);				\	
}									\
NEXT_TOKEN;

#define SAVE_CURR_COORDINATE	g_recorded_coord.g_cursor = G_CURSOR;	\
								g_recorded_coord.line = G_LINE;
\

#define BACK_TO_SAVED_COORDINATE	G_CURSOR = g_recorded_coord.g_cursor;	\
									G_LINE	 = g_recorded_coord.line;
										\

ast_node_t *create_token_node()
{
	ast_node_t *node;

	node = (ast_node_t*)bcc_malloc(sizeof(ast_node_t));
	bcc_memcpy(&node->tk_val, &g_current_token, sizeof(token_t));
	return node;
}

/*parse expression*/

ast_node_t *parse_primary_expr()
{
	primary_expr_t *pri_expr;

	switch (G_TK_KIND) {
	case TK_IDENTIFIER:
	case TK_INTCONST:
	case TK_UNSIGNED_INTCONST:
	case TK_LONGCONST:
	case TK_UNSIGNED_LONGCONST:
	case TK_LLONGCONST:
	case TK_UNSIGNED_LLONGCONST:
	case TK_FLOATCONST:
	case TK_DOUBLECONST:
	case TK_LDOUBLECONST:
	case TK_STRING:
		pri_expr = (primary_expr_t *)bcc_malloc(sizeof(primary_expr_t));
		pri_expr->kind = PRI_EXPR_TOKEN;
		pri_expr->expr = create_token_node();
		NEXT_TOKEN;
		break;
	case TK_LPAREN:
		pri_expr = (primary_expr_t *)bcc_malloc(sizeof(primary_expr_t));
		pri_expr->kind = PRI_EXPR_EXPR;
		NEXT_TOKEN;
		pri_expr->expr = parse_expr();
		break;
	default:
		pri_expr = NULL;
	}
	return pri_expr;
}

#define CREATE_POSTFIX_NODE(tmp_postfix, postfix)	tmp_postfix = postfix;												\
													postfix = (postfix_t *)bcc_malloc(sizeof(postfix_t));				\
													postfix->next = tmp_postfix;

ast_node_t *parse_postfix()
{
	postfix_t *postfix, *tmp_postfix;
	int kind;

	postfix = tmp_postfix = NULL;
	while (1) {
		switch (G_TK_KIND) {
		case TK_LBRACKET:
			NEXT_TOKEN;
			kind = ARRAY_POSTFIX;

			CREATE_POSTFIX_NODE(tmp_postfix, postfix);
			if (G_TK_KIND == TK_RBRACKET) {
				postfix->expr = NULL;
				break;
			}
			postfix->expr = parse_expr();
			break;
		case TK_LPAREN:
			NEXT_TOKEN;
			kind = FUNC_POSTFIX;
			
			CREATE_POSTFIX_NODE(tmp_postfix, postfix)
			if (G_TK_KIND == TK_RPAREN) {
				postfix->expr = NULL;
				break;
			}
			postfix->expr = parse_argu_expr_list();
			break;
		case TK_DOT:
		case TK_POINTER:
			kind = STRCTURE_POSTFIX;

			CREATE_POSTFIX_NODE(tmp_postfix, postfix)
			postfix->op = create_token_node();
			NEXT_TOKEN;
			postfix->ident = create_token_node();
			break;
		case TK_INC:
		case TK_DEC:
			kind = INC_DEC_POSTFIX;

			CREATE_POSTFIX_NODE(tmp_postfix, postfix)
			postfix->op = create_token_node;
			break;
		default:
			return postfix;
		}
		postfix->kind = kind;
		NEXT_TOKEN;
	}
}

ast_node_t *parse_postfix_expr()
{
	postfix_expr_t *postfix_expr;
	postfix_t *postfix;

	postfix_expr = (postfix_expr_t*)bcc_malloc(sizeof(postfix_expr_t));
	postfix_expr->primary_expr = parse_primary_expr();
	if (postfix_expr->primary_expr == NULL) {
		error_message("postfix_expr without primary expression");
	}

	postfix_expr->postfix = parse_postfix();

	return postfix_expr;
}

ast_node_t *parse_unary_expr()
{
	unary_expr_t *unary_expr;
	int expr_kind;

	unary_expr = (unary_expr_t *)bcc_malloc(sizeof(unary_expr_t));
	switch (G_TK_KIND) {
	case TK_INC:
	case TK_DEC:
		expr_kind = UNARY_EXPR;
		strncpy(&unary_expr->op->tk_val, &g_current_token, sizeof(token_t));
		NEXT_TOKEN;
		unary_expr->expr = parse_unary_expr();
		break;
	case TK_MOD:
	case TK_MULTIPLY:
	case TK_ADD:
	case TK_SUB:
	case TK_BITREVERT:
	case TK_NOT:
		expr_kind = CAST_EXPR;
		strncpy(&unary_expr->op->tk_val, &g_current_token, sizeof(token_t));
		NEXT_TOKEN;
		unary_expr->expr = parse_cast_expr();
		break;
	case TK_SIZEOF:
		strncpy(&unary_expr->op->tk_val, &g_current_token, sizeof(token_t));
		NEXT_TOKEN;
		if (G_TK_KIND == TK_LPAREN) {
			expr_kind = TYPE_NAME;
			unary_expr->expr = parse_type_name();
			NEXT_TOKEN;
		} else {
			expr_kind = UNARY_EXPR;
			unary_expr->expr = parse_unary_expr();
		}
		break;
	default:
		expr_kind = POSTFIX_EXPR;
		unary_expr->expr = parse_postfix_expr();
	}
	unary_expr->expr_kind = expr_kind;
	return unary_expr;
}

ast_node_t *parse_cast_expr()
{
	cast_expr_t *cast_expr;

	cast_expr = (cast_expr_t *)bcc_malloc(sizeof(cast_expr_t));

	if (G_TK_KIND == TK_LPAREN) {
		cast_expr->is_unary_expr = FALSE;
		cast_expr->type_name = parse_type_name();
		NEXT_TOKEN;
		cast_expr->expr = parse_cast_expr();
	} else {
		cast_expr->is_unary_expr = TRUE;
		cast_expr->expr = parse_unary_expr();
	}
	return cast_expr;
}

ast_node_t *parse_mutli_expr()
{
	multi_expr_t *multi_expr;

	multi_expr = (multi_expr_t *)bcc_malloc(sizeof(multi_expr_t));
	
	multi_expr->cast_expr = parse_cast_expr();
	multi_expr->next = NULL;
	if (G_TK_KIND == TK_MULTIPLY
		|| G_TK_KIND == TK_DIVIDE
		|| G_TK_KIND == TK_MOD) {
		multi_expr->op = create_token_node();
		NEXT_TOKEN;
		multi_expr->next = parse_mutli_expr();
	}
	return multi_expr;
}

ast_node_t *parse_addit_expr()
{
	addit_expr_t *addit_expr;

	addit_expr = (addit_expr_t *)bcc_malloc(sizeof(multi_expr_t));
	addit_expr->multi_expr = parse_mutli_expr();
	addit_expr->next = NULL;

	if (G_TK_KIND == TK_ADD
		|| G_TK_KIND == TK_SUB) {
		addit_expr->op = create_token_node();
		NEXT_TOKEN;
		addit_expr->next = parse_addit_expr();
	}
	return addit_expr;
}

ast_node_t *parse_shift_expr()
{
	shift_expr_t *shift_expr;
	
	shift_expr = (shift_expr_t *)bcc_malloc(sizeof(shift_expr_t));
	shift_expr->addit_expr = parse_addit_expr();
	shift_expr->next = NULL;

	if (G_TK_KIND == TK_LSHIFT
		|| G_TK_KIND == TK_RSHIFT) {
		shift_expr->op = create_token_node();
		NEXT_TOKEN;
		shift_expr->next = parse_shift_expr();
	}
	return shift_expr;
}

ast_node_t *parse_rela_expr()
{
	rela_expr_t *rela_expr;
	
	rela_expr = (rela_expr_t*)bcc_malloc(sizeof(rela_expr_t));
	rela_expr->shift_expr = parse_shift_expr();
	rela_expr->next = NULL;

	if (G_TK_KIND == TK_LESS
		|| G_TK_KIND == TK_LESS_EQUAL
		|| G_TK_KIND == TK_GREAT
		|| G_TK_KIND == TK_GREAT_EQUAL) {
		rela_expr->op = create_token_node();
		NEXT_TOKEN;
		rela_expr->next = parse_rela_expr();
	}

}

ast_node_t *parse_equal_expr()
{
	equal_expr_t *equal_expr;

	equal_expr = (equal_expr_t *)bcc_malloc(sizeof(equal_expr_t));
	equal_expr->rela_expr = parse_rela_expr();
	equal_expr->next = NULL;

	if (G_TK_KIND == TK_EQUAL
		|| G_TK_KIND == TK_NEQUAL) {
		equal_expr->op = create_token_node();
		NEXT_TOKEN;
		equal_expr->next = parse_equal_expr();
	}
	return equal_expr;
}

ast_node_t *parse_and_expr()
{
	and_expr_t *and_expr;

	and_expr = (and_expr_t *)bcc_malloc(sizeof(equal_expr_t));
	and_expr->equal_expr = parse_equal_expr();
	and_expr->next = NULL;

	if (G_TK_KIND == TK_BITAND) {
		and_expr->op = create_token_node();
		NEXT_TOKEN;
		and_expr->next = parse_and_expr();
	}
	return and_expr;
}

ast_node_t *parse_excl_or_expr()
{
	exclusive_or_expr_t *excl_or_expr;

	excl_or_expr = (exclusive_or_expr_t *)bcc_malloc(sizeof(exclusive_or_expr_t));
	excl_or_expr->and_expr = parse_and_expr();
	excl_or_expr->next = NULL;

	if (G_TK_KIND == TK_BITXOR) {
		excl_or_expr->op = create_token_node();
		NEXT_TOKEN;
		excl_or_expr->next = parse_excl_or_expr();
	}
	return excl_or_expr;
}

ast_node_t *parse_incl_or_expr()
{
	inclusive_or_expr_t *incl_or_expr;

	incl_or_expr = (inclusive_or_expr_t *)bcc_malloc(sizeof(inclusive_or_expr_t));
	incl_or_expr->exclu_or_expr = parse_excl_or_expr();
	incl_or_expr->next = NULL;

	if (G_TK_KIND == TK_BITOR) {
		incl_or_expr->op = create_token_node();
		NEXT_TOKEN;
		incl_or_expr->next = parse_incl_or_expr();
	}
	return incl_or_expr;
}

ast_node_t *parse_logic_and_expr()
{
	logic_and_expr_t *logic_and_expr;

	logic_and_expr = (logic_and_expr_t *)bcc_malloc(sizeof(logic_and_expr_t));
	logic_and_expr->inclusive_or_expr = parse_incl_or_expr();
	logic_and_expr->logic_and_expr = NULL;

	if (G_TK_KIND == TK_AND) {
		logic_and_expr->op = create_token_node();
		NEXT_TOKEN;
		logic_and_expr->logic_and_expr = parse_logic_and_expr();
	}
	return logic_and_expr;
}

ast_node_t *parse_logic_or_expr()
{
	logic_or_expr_t *logic_or_expr;

	logic_or_expr = (logic_or_expr_t *)bcc_malloc(sizeof(logic_or_expr_t));
	logic_or_expr->logic_and_expr = parse_logic_and_expr();
	logic_or_expr->logic_or_expr = NULL;

	if (G_TK_KIND == TK_OR) {
		logic_or_expr->op = create_token_node();
		NEXT_TOKEN;
		logic_or_expr->logic_or_expr = parse_logic_or_expr();
	}
	return logic_or_expr;
}

ast_node_t *parse_cond_expr()
{
	cond_expr_t *cond_expr;

	cond_expr = (cond_expr_t *)bcc_malloc(sizeof(cond_expr_t));
	cond_expr->logic_or_expr = parse_logic_or_expr();
	cond_expr->expr = cond_expr->cond_expr = NULL;

	if (G_TK_KIND == TK_QUESTION) {
		cond_expr->que_op = create_token_node();
		NEXT_TOKEN;
		cond_expr->expr = parse_expr();
		if (G_TK_KIND != TK_COLON) {
			error_message("expected :");
		}
		cond_expr->colon_op = create_token_node();
		NEXT_TOKEN;
		cond_expr->cond_expr = parse_cond_expr();
	}
	return cond_expr;
}

/*
assignment_expression
: conditional_expression
| unary_expression assignment_operator assignment_expression
;
*/
/*
because conditional_expression contains unary_expression,
we need tread unary_expresion as conditional
*/

ast_node_t *parse_assign_expr()
{
	assign_expr_t *assign_expr;

	assign_expr = (assign_expr_t *)bcc_malloc(sizeof(assign_expr_t));
	assign_expr->cond_expr = parse_cond_expr();
	assign_expr->assign_expr = NULL;

	if (G_TK_KIND == TK_ASSIGN
		|| G_TK_KIND == TK_ADD_ASSING
		|| G_TK_KIND == TK_SUB_ASSIGN
		|| G_TK_KIND == TK_MULTI_ASSIGN
		|| G_TK_KIND == TK_DIVIDE_ASSIGN
		|| G_TK_KIND == TK_MOD_ASSIGN
		|| G_TK_KIND == TK_BITXOR_ASSIGN
		|| G_TK_KIND == TK_BITOR_ASSIGN
		|| G_TK_KIND == TK_BITAND_ASSIGN
		|| G_TK_KIND == TK_LSHIFT_ASSIGN
		|| G_TK_KIND == TK_RSHIFT_ASSIGN) {
		assign_expr->assign_op = create_token_node();
		NEXT_TOKEN;
		assign_expr->assign_expr = parse_assign_expr();
	}

}

ast_node_t *parse_expr()
{
	expr_t *expr;
	
	expr = (expr_t *)bcc_malloc(sizeof(expr_t));
	expr->assign_expr = parse_assign_expr();
	expr->expr = NULL;

	if (G_TK_KIND == TK_COMMA) {
		expr->op = create_token_node();
		NEXT_TOKEN;
		expr->expr = parse_expr();
	}
	return expr;
}
ast_node_t *parse_const_expr()
{
	const_expr_t *const_expr;

	const_expr = (const_expr_t *)bcc_malloc(sizeof(const_expr_t));
	const_expr->cond_expr = parse_cond_expr();

	return const_expr;
}


/*parse statement*/

ast_node_t *parse_spec_qual_list()
{
	spec_qual_list_t *spec_list;
	type_spec_t *type_spec;
	type_qual_t *type_qual;
	BOOL invalid_decl_spec;

	spec_list = (spec_qual_list_t *)bcc_malloc(sizeof(spec_qual_list_t));

	type_spec = type_qual = NULL;
	invalid_decl_spec = FALSE;

	while (1) {
		switch (G_TK_KIND) {
		case TK_VOID:
		case TK_CHAR:
		case TK_SHORT:
		case TK_INT:
		case TK_LONG:
		case TK_FLOAT:
		case TK_DOUBLE:
		case TK_SIGNED:
		case TK_UNSIGNED:
			if (type_spec == NULL) {
				type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
				type_spec->kind = TYPE_SPEC_BASIC_TYPE;
				type_spec->value = create_token_node();
			} else {
				error_message("repeated type specifier");
			}
			break;
		case TK_STRUCT:
		case TK_UNION:
			if (type_spec == NULL) {
				type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
				type_spec->kind = TYPE_SPEC_STRUCT_UNION;
				type_spec->value = parse_struct_union();
			} else {
				error_message("repeated type specifier");
			}
			break;
		case TK_ENUM:
			if (type_spec == NULL) {
				type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
				type_spec->kind = TYPE_SPEC_ENUM_SPEC;
				type_spec->value = parse_enum();
			}
			else {
				error_message("repeated type specifier");
			}
			break;
		case TK_IDENTIFIER:
			if (is_typedef_name(g_current_token.token_value.ptr)) {
				if (type_spec == NULL) {
					type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
					type_spec->kind = TYPE_SPEC_TYPEDEF;
					type_spec->value = create_token_node();
				} else {
					error_message("repeated type specifier");
				}
			} else {
				invalid_decl_spec = TRUE;
			}
			break;
		case TK_CONST:
			if (type_qual == NULL) {
				type_qual = (type_qual_t *)bcc_malloc(sizeof(type_qual_t));
				type_qual->const_tk = create_token_node();
			}
			break;
		case TK_VOLATILE:
			if (type_qual == NULL) {
				type_qual = (type_qual_t *)bcc_malloc(sizeof(type_qual_t));
				type_qual->volatile_tk = create_token_node();
			}
			break;
		default:
			invalid_decl_spec = TRUE;
			break;
		}

		if (invalid_decl_spec) {
			break;
		}
		NEXT_TOKEN;
	}
	spec_list->type_sepc = type_spec;
	spec_list->type_qual = type_qual;
}

ast_node_t *parse_struct_declarator()
{
	struct_declarator_t *struct_decl;

	struct_decl = (struct_declarator_t *)bcc_malloc(sizeof(struct_declarator_t));
	struct_decl->next = NULL;

	if (G_TK_KIND != TK_COLON) {
		struct_decl->declarator = parse_declarator();
		if (G_TK_KIND == TK_COLON) {
			struct_decl->op = create_token_node();
			NEXT_TOKEN;
			struct_decl->const_expr = parse_const_expr();
		}
	} else {
		struct_decl->op = create_token_node();
		NEXT_TOKEN;
		struct_decl->const_expr = parse_const_expr();
	}
	return struct_decl;
}

ast_node_t *parse_struct_declarator_list()
{
	struct_declarator_t *list, *list_iter;
	
	list = list_iter = parse_struct_declarator();

	while (G_TK_KIND == TK_COMMA) {
		list_iter->next = parse_struct_declarator();
		list_iter = list_iter->next;
	}
	NEXT_TOKEN;
	return list;
}

ast_node_t *parse_struct_declaration()
{
	struct_declaration_t *struct_decl;

	struct_decl = (struct_declaration_t *)bcc_malloc(sizeof(struct_declaration_t));
	struct_decl->spec_qual = parse_spec_qual_list();
	struct_decl->struct_decl = parse_struct_declarator_list();
	struct_decl->next = NULL;

	return struct_decl;
}

ast_node_t *parse_struct_declaration_list()
{
	struct_declaration_t *list, *list_iter;
	
	list = list_iter = parse_struct_declaration();
	while (G_TK_KIND != TK_RBRACE) {
		list_iter->next = parse_struct_declaration();
		list_iter = list_iter->next;
	}
	NEXT_TOKEN;
	return list;

}

ast_node_t *parse_struct_union()
{
	struct_or_union_spec_t *struct_union;

	struct_union = (struct_or_union_spec_t*)bcc_malloc(sizeof(struct_or_union_spec_t));
	struct_union->s_or_u = create_token_node();

	NEXT_TOKEN;
	if (G_TK_KIND == TK_LBRACE) {
		NEXT_TOKEN;
		struct_union->struct_decl = parse_struct_declaration_list();
		NEXT_TOKEN;
	} else if (G_TK_KIND == TK_IDENTIFIER) {
		struct_union->ident = create_token_node();
		NEXT_TOKEN;
		if (G_TK_KIND == TK_LBRACE) {
			NEXT_TOKEN;
			struct_union->struct_decl = parse_struct_declaration_list();
			NEXT_TOKEN;
		}
	} else {
		error_message("invalid token");
	}
	return struct_union;
}

ast_node_t *parse_enumerator()
{
	enumerator_t *enumerator;

	enumerator = (enumerator_t *)bcc_malloc(sizeof(enumerator_t));
	enumerator->ident = create_token_node();
	NEXT_TOKEN;
	if (G_TK_KIND == TK_ASSIGN) {
		enumerator->op = create_token_node();
		NEXT_TOKEN;
		enumerator->const_expr = parse_const_expr();
	}
	return enumerator;
}

ast_node_t *parse_enumerator_list()
{
	enumerator_t *list, *list_iter;

	list = list_iter = (enumerator_t *)bcc_malloc(sizeof(enumerator_t));
	list_iter = parse_enumerator();

	while (G_TK_KIND == TK_COMMA) {
		list_iter->next = parse_enumerator();
		list_iter = list_iter->next;
	}
	return list_iter;
}

ast_node_t *parse_enum()
{
	enum_spec_t *enum_spec;
	enum_spec = (enum_spec_t*)bcc_malloc(sizeof(enumerator_t));

	enum_spec->enum_keyword = create_token_node();
	NEXT_TOKEN;
	if (G_TK_KIND = TK_LBRACE) {
		NEXT_TOKEN;
		enum_spec->enum_decl = parse_enumerator_list();
		NEXT_TOKEN;
	} else if (G_TK_KIND == TK_IDENTIFIER) {
		enum_spec->ident = create_token_node();
		NEXT_TOKEN;
		if (G_TK_KIND = TK_LBRACE) {
			NEXT_TOKEN;
			enum_spec->enum_decl = parse_enumerator_list();
			NEXT_TOKEN;
		}
	} else {
		error_message("invalid token\n");
	}
}

ast_node_t *parse_decl_spec()
{	
	decl_spec_t *decl_spec = NULL;
	store_cls_spec_t *store_cls;
	type_spec_t *type_spec;
	type_qual_t *type_qual;
	BOOL invalid_decl_spec;

	decl_spec = (decl_spec_t *)bcc_malloc(sizeof(decl_spec_t));
	store_cls = type_qual = type_spec = NULL;

	invalid_decl_spec = FALSE;
	while (1) {
		switch (G_TK_KIND) {
		case TK_TYPEDEF:
		case TK_EXTERN:
		case TK_STATIC:
		case TK_AUTO:
		case TK_REGISTER:
			if (store_cls == NULL) {
				store_cls = (store_cls_spec_t *)bcc_malloc(sizeof(store_cls_spec_t));
				store_cls->value = create_token_node();
			} else {
				error_message("repeated storage class specifier");
			}
			if (G_TK_KIND == TK_TYPEDEF) {
				insert_to_scope(&(g_current_scope->tdname), g_current_token.token_value.ptr, TDNAME, 0);
			}
			break;
		case TK_VOID:
		case TK_CHAR:
		case TK_SHORT:
		case TK_INT:
		case TK_LONG:
		case TK_FLOAT:
		case TK_DOUBLE:
		case TK_SIGNED:
		case TK_UNSIGNED:
			if (type_spec == NULL) {
				type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
				type_spec->kind = TYPE_SPEC_BASIC_TYPE;
				type_spec->value = create_token_node();
			}
			else {
				error_message("repeated type specifier");
			}
			break;
		case TK_STRUCT:
		case TK_UNION:
			if (type_spec == NULL) {
				type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
				type_spec->kind = TYPE_SPEC_STRUCT_UNION;
				type_spec->value = parse_struct_union();
			}
			else {
				error_message("repeated type specifier");
			}
			break;
		case TK_ENUM:
			if (type_spec == NULL) {
				type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
				type_spec->kind = TYPE_SPEC_ENUM_SPEC;
				type_spec->value = parse_enum();
			}
			else {
				error_message("repeated type specifier");
			}
			break;
		case TK_IDENTIFIER:
			if (is_typedef_name(g_current_token.token_value.ptr)) {
				if (type_spec == NULL) {
					type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
					type_spec->kind = TYPE_SPEC_TYPEDEF;
					type_spec->value = create_token_node();
				} else {
					error_message("repeated type specifier");
				}
			} else {
				invalid_decl_spec = TRUE;
			}
			break;
		case TK_CONST:
			if (type_qual == NULL) {
				type_qual = (type_qual_t *)bcc_malloc(sizeof(type_qual_t));
				type_qual->const_tk = create_token_node();
			}
			break;
		case TK_VOLATILE:
			if (type_qual == NULL) {
				type_qual = (type_qual_t *)bcc_malloc(sizeof(type_qual_t));
				type_qual->volatile_tk = create_token_node();
			}
			break;
		default:
			invalid_decl_spec = TRUE;
			break;
		}
		if (invalid_decl_spec) {
			break;
		}
		NEXT_TOKEN;
	}
	decl_spec->store_cls = store_cls;
	decl_spec->type_spec = type_spec;
	decl_spec->type_qual = type_qual;
	
	return decl_spec;
}

ast_node_t *parse_decl_postfix()
{
	decl_postfix_t *decl_postfix, *tmp_decl_postfix;

	decl_postfix = tmp_decl_postfix = NULL;
	while (1) {
		switch (G_TK_KIND) {
		case TK_LBRACKET:
			NEXT_TOKEN;
			CREATE_POSTFIX_NODE(tmp_decl_postfix, decl_postfix);
			if (G_TK_KIND == TK_RBRACKET) {
				decl_postfix->next = NULL;
				break;
			}
			decl_postfix->const_expr = parse_const_expr();
			break;
		case TK_LPAREN:
			NEXT_TOKEN;
			CREATE_POSTFIX_NODE(tmp_decl_postfix, decl_postfix);
			if (G_TK_KIND == TK_RPAREN) {
				decl_postfix->next = NULL;
				break;
			}
			if (G_TK_KIND == TK_IDENTIFIER) {
				decl_postfix->ident_list = parse_ident_list();
				break;
			}
			decl_postfix->param_list = parse_param_type_list();
			break;
		default:
			return decl_postfix;
		}
		NEXT_TOKEN;
	}
}

ast_node_t *parse_direct_declarator()
{
	direct_declarator_t *direct_decl;

	direct_decl = (direct_declarator_t *)bcc_malloc(sizeof(direct_declarator_t));
	direct_decl->decl = direct_decl->ident = direct_decl->post_fix = NULL;

	if (G_TK_KIND == TK_IDENTIFIER) {
		direct_decl->is_ident = TRUE;
		direct_decl->ident = create_token_node();
	} else if (G_TK_KIND == TK_LPAREN) {
		direct_decl->is_ident = FALSE;
		NEXT_TOKEN;
		direct_decl->decl = parse_declarator();
		NEXT_TOKEN;
	} else {
		error_message("invalid direct declarator");
	}
	direct_decl->post_fix = parse_decl_postfix();

	return direct_decl;
}

ast_node_t *parse_declarator()
{
	declarator_t *decl;

	decl = (declarator_t *)bcc_malloc(sizeof(declarator_t));

	if (G_TK_KIND == TK_MULTIPLY) {
		decl->pointer = create_token_node();
	}
	decl->direct_declarator = parse_direct_declarator();
	return decl;
}

ast_node_t *parse_initializer_list()
{
	initializer_list_t *init_list;

	init_list = (initializer_list_t *)bcc_malloc(sizeof(initializer_list_t));
	init_list->initializer = parse_initializer;
	init_list->next = NULL;
	while (G_TK_KIND == TK_COMMA && G_TK_KIND != TK_RBRACE) {
		NEXT_TOKEN;
		init_list->next = parse_initializer();
	}
	return init_list;
}

ast_node_t *parse_initializer()
{
	initializer_t *initializer;

	initializer = (initializer_t *)bcc_malloc(sizeof(initializer_t));
	initializer->assign_expr = initializer->initialier_list = NULL;
	if (G_TK_KIND == TK_LBRACE) {
		NEXT_TOKEN;
		initializer->initialier_list = parse_initializer_list();
		if (G_TK_KIND == TK_COMMA) {
			NEXT_TOKEN;
		}
		NEXT_TOKEN;
	}
	initializer->assign_expr = parse_assign_expr();
	return initializer;
}


ast_node_t *parse_init_declarator()
{
	init_declarator_t *init_decl;

	init_decl = (init_declarator_t *)bcc_malloc(sizeof(init_declarator_t));
	init_decl->declarator = parse_declarator();
	init_decl->op = init_decl->next_decl = NULL;

	if (G_TK_KIND == TK_ASSIGN) {
		init_decl->op = create_token_node();
		NEXT_TOKEN;
		init_decl->initializer = parse_initializer();
	}
	return init_decl;
}

ast_node_t *parse_init_declarator_list()
{
	init_declarator_t *list, *list_iter;

	list = list_iter = parse_init_declarator();
	while (G_TK_KIND == TK_COMMA) {
		list_iter->next_decl = parse_init_declarator_list();
		list_iter = list_iter->next_decl;
	}
	return list;
}

ast_node_t *parse_declaration()
{
	declaration_t *decl;
	decl = (declaration_t *)bcc_malloc(sizeof(declaration_t));

	decl->decl_spec = parse_decl_spec();
	decl->init_decl = NULL;
	decl->next = NULL;

	if (G_TK_KIND != TK_SEMICOLON) {
		decl->init_decl = parse_init_declarator_list();
	}
	NEXT_TOKEN;
	return decl;
}

BOOL is_decl_spec() {
	if (G_TK_KIND == TK_TYPEDEF
		|| G_TK_KIND == TK_EXTERN
		|| G_TK_KIND == TK_STATIC
		|| G_TK_KIND == TK_AUTO
		|| G_TK_KIND == TK_REGISTER
		|| G_TK_KIND == TK_VOID
		|| G_TK_KIND == TK_CHAR
		|| G_TK_KIND == TK_SHORT
		|| G_TK_KIND == TK_INT
		|| G_TK_KIND == TK_LONG
		|| G_TK_KIND == TK_FLOAT
		|| G_TK_KIND == TK_DOUBLE
		|| G_TK_KIND == TK_SIGNED
		|| G_TK_KIND == TK_UNSIGNED
		|| G_TK_KIND == TK_STRUCT
		|| G_TK_KIND == TK_ENUM
		|| G_TK_KIND == TK_CONST
		|| G_TK_KIND == TK_VOLATILE) {
		return TRUE;
	}
	if (G_TK_KIND == TK_IDENTIFIER) {
		if (is_typedef_name(g_current_token.token_value.ptr)) {
			return TRUE;
		}
	}
	return FALSE;
}

ast_node_t *parse_declaration_list()
{
	declaration_t *list;
	list = parse_declaration();
	
	if (is_decl_spec()) {
		list->next = parse_declaration_list();
	}
	return list;
}

ast_node_t *parse_labeled_statement()
{
	labeled_statement_t *labeled_state;

	labeled_state = (labeled_statement_t *)bcc_malloc(sizeof(labeled_statement_t));

	labeled_state->token = create_token_node();
	switch (G_TK_KIND) {
	case TK_IDENTIFIER:
	case TK_DEFAULT:
		SKIP(TK_COLON);
		break;
	case TK_CASE:
		labeled_state->const_expr_ptr = parse_const_expr();
		SKIP(TK_COLON);
		break;
	default:
		error_message("invalid token\n");
	}
	labeled_state->statement = parse_statement();

	return labeled_state;
}

ast_node_t *parse_expr_statement()
{
	expr_statement_t *expr_state;

	expr_state = (expr_statement_t *)bcc_malloc(sizeof(expr_statement_t));
	
	expr_state->expr = parse_expr();
	SKIP(TK_SEMICOLON);

	return expr_state;
}

ast_node_t *parse_selection_statement()
{
	select_statement_t *select_state;
	int select_kind;

	select_state = (select_statement_t *)bcc_malloc(sizeof(select_statement_t));
	
	select_state->fir_token = create_token_node();
	select_kind = G_TK_KIND;

	NEXT_TOKEN;
	SKIP(TK_LPAREN);
	select_state->fir_expr = parse_expr();
	SKIP(TK_RPAREN);
	select_state->statement = parse_statement();
	
	if (select_kind == TK_IF && G_TK_KIND == TK_ELSE) {
		select_state->sec_token = create_token_node();
		NEXT_TOKEN;
		select_state->sec_expr = parse_statement();
	}
	return select_state;
}

ast_node_t *parse_iteration_statement()
{
	iteration_statement_t *iter_state;

	iter_state->fir_token = create_token_node();
	
	switch (G_TK_KIND) {
	case TK_WHILE:
		NEXT_TOKEN;
		SKIP(TK_LPAREN);
		iter_state->expr = parse_expr();
		SKIP(TK_RPAREN);
		iter_state->statement = parse_statement();
		break;
	case TK_DO:
		NEXT_TOKEN;
		iter_state->statement = parse_statement();
		iter_state->sec_token = create_token_node();
		NEXT_TOKEN;
		SKIP(TK_LPAREN);
		iter_state->expr = parse_expr();
		SKIP(TK_RPAREN);
		SKIP(TK_SEMICOLON);
		break;
	case TK_FOR:
		NEXT_TOKEN;
		SKIP(TK_LPAREN);
		iter_state->expr_statement1 = parse_expr_statement();
		iter_state->expr_statement2 = parse_expr_statement();
		if (G_TK_KIND != TK_RPAREN) {
			iter_state->expr = parse_expr();
		}
		SKIP(TK_LPAREN);
		iter_state->statement = parse_statement();
		break;
	default:
		error_message("invalid token");
		break;
	}
	return iter_state;
}

ast_node_t *parse_jump_statement()
{
	jump_statement_t *jump_state;

	jump_state->jmp_cmd = create_token_node();

	switch (G_TK_KIND) {
	case TK_GOTO:
		NEXT_TOKEN;
		jump_state->value = create_token_node();
		NEXT_TOKEN;
		break;
	case TK_RETURN:
		NEXT_TOKEN;
		if (G_TK_KIND != TK_SEMICOLON) {
			jump_state->value = parse_expr();
		}
		break;
	case TK_CONTIUE:
	case TK_BREAK:
		NEXT_TOKEN;
		break;
	default:
		error_message("invalid token");
		break;
	}
	SKIP(TK_SEMICOLON);

	return jump_state;
}

ast_node_t *parse_statement()
{
	statement_t* statement;

	statement = (statement_t*)bcc_malloc(sizeof(statement_t));
	statement->statement = statement->next = NULL;

	switch (G_TK_KIND) {
	case TK_IDENTIFIER:
		SAVE_CURR_COORDINATE;
		NEXT_TOKEN;
		if (G_TK_KIND == TK_COLON) {
			BACK_TO_SAVED_COORDINATE;
			statement->node_kind = LABELED_STATEMENT;
			statement->statement = parse_labeled_statement();
		} else {
			BACK_TO_SAVED_COORDINATE;
			statement->node_kind = EXPR_STATEMENT;
			statement->statement = parse_expr_statement();
		}
		break;
	case TK_CASE:
	case TK_DEFAULT:
		statement->node_kind = LABELED_STATEMENT;
		statement->statement = parse_labeled_statement();
		break;
	case TK_LBRACE:
		statement->node_kind = COMPOUND_STATEMENT;
		statement->statement = parse_compound_statement();
		break;
	case TK_IF:
	case TK_SWITCH:
		statement->node_kind = SELECT_STATEMENT;
		statement->statement = parse_selection_statement();
		break;
	case TK_WHILE:
	case TK_DO:
	case TK_FOR:
		statement->node_kind = ITERAT_STATEMENT;
		statement->statement = parse_iteration_statement();
		break;
	case TK_GOTO:
	case TK_CONTIUE:
	case TK_BREAK:
	case TK_RETURN:
		statement->node_kind = JUMP_STATMENT;
		statement->statement = parse_jump_statement();
		break;
	default:
		error_message("invalid token");
		break;
	}
	return statement;
}

ast_node_t *parse_statement_list()
{
	statement_t *statement_list ,*statement_iter;

	statement_list = statement_iter = parse_statement();

	while (G_TK_KIND != TK_LBRACE) {
		statement_iter->next = parse_statement();
		statement_iter = statement_iter->next;
	}
	return statement_list;
}

ast_node_t *parse_compound_statement()
{
	comp_state_t *comp_state;

	if (G_TK_KIND != TK_LBRACE) {
		error_message("unexpected token\n");
	}
	SKIP(TK_LBRACE);			
	if (is_decl_spec()) {
		comp_state->decl_list = parse_declaration_list();
	}
	comp_state->decl_list = parse_statement_list();
	SKIP(TK_RBRACE);	
	return comp_state;
}

ast_node_t *parse_external_decl()
{
	declaration_t *decl;
	func_def_t *func_def;
	decl_spec_t *decl_spec;
	init_declarator_t *init_decl;

	decl = func_def = decl_spec = init_decl = NULL;

	if (is_decl_spec()) {
		decl_spec = parse_decl_spec();
	}

	if (G_TK_KIND != TK_SEMICOLON) {
		init_decl = parse_init_declarator_list();
	}

	if (G_TK_KIND == TK_SEMICOLON) {
		decl = (declaration_t *)bcc_malloc(sizeof(declaration_t));
		decl->decl_spec = decl_spec;
		decl->init_decl = init_decl;
		return decl;
	} else if(G_TK_KIND == TK_LBRACE || is_decl_spec()) {
		func_def = (declaration_t *)bcc_malloc(sizeof(declaration_t));
		func_def->decl_spec_ptr = decl_spec;
		func_def->decl_ptr = init_decl->declarator;
		if (is_decl_spec()) {
			func_def->decl_list_ptr = parse_declaration_list();
			EXPECT(TK_LBRACE);
		}
		func_def->comp_state_ptr = parse_compound_statement();

		return func_def->comp_state_ptr;
	} else {
		error_message("unexpected token");
	}
}