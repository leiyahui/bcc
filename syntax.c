#include "bcc.h"


hashtable_t  g_external_symbol_table;
coordinate_t g_recorded_coord;



BOOL is_typedef_name(char *name)
{
	scope_t *scope;
	BOOL typedefed;
	int level;

	level = 0;
	typedefed = FALSE;
	scope = g_curr_scope;
	while (scope != NULL) {
		level++;
		if (in_curr_user_define_type(&(scope->tdname_head), name)) {
			typedefed = TRUE;
			break;
		}
		scope = scope->parent;
	}
	if (typedefed == TRUE) {
		scope = g_curr_scope;
		while (level > 1) {
			if (in_curr_user_define_type(&(scope->sym_head), name)) {
				typedefed = FALSE;
				break;
			}
			level--;
			scope = scope->parent;
		}
	}
	return typedefed;
}



#define NEXT_TOKEN get_next_token();		

#define G_TK_KIND	g_current_token.tk_kind
#define G_TK_VALUE	g_current_token.token_value
#define G_TK_LINE	g_current_token.line

#define TK_VALUE_INT(tk)	= (tk).int_num
#define TK_VALUE_SHORT(tk)	= tk.short_num
#define TK_VALUE_LONG(tk)	= tk.long_num
#define TK_VALUE_FLOAT(tk)	= tk.float_num
#define TK_VALUE_DOUBLE(tk)	= tk.double_num
#define TK_VALUE_PTR(tk)	= tk.ptr

#define EXPECT(tk_kind) if (G_TK_KIND != tk_kind) {									\
							ERROR("expect token is:%d", tk_kind);		\
						}					

#define SKIP(tk_kind)   if (G_TK_KIND != tk_kind) {									\
							ERROR("expect token is:%d", tk_kind);		\
						}															\
						NEXT_TOKEN;						

#define SAVE_CURR_COORDINATE	g_recorded_coord.g_cursor = G_CURSOR;				\
								g_recorded_coord.line = G_LINE;						\
								g_recorded_coord.colum = G_COLUM;

#define BACK_TO_SAVED_COORDINATE	G_CURSOR = g_recorded_coord.g_cursor;			\
									G_LINE	 = g_recorded_coord.line;				\
									G_COLUM  = g_recorded_coord.colum;
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

	pri_expr = (primary_expr_t *)bcc_malloc(sizeof(primary_expr_t));
	pri_expr->kind = PRI_TOKEN;
	pri_expr->compile_evaluated = FALSE;

	switch (G_TK_KIND) {
	case TK_IDENTIFIER:
		pri_expr->expr = create_token_node();
		NEXT_TOKEN;
		break;
	case TK_INTCONST:
	case TK_UNSIGNED_INTCONST:
		pri_expr->expr = create_token_node();
		pri_expr->compile_evaluated = TRUE;
		pri_expr->value = TK_VALUE_INT(G_TK_VALUE);
		NEXT_TOKEN;
		break;
	case TK_LONGCONST:
	case TK_UNSIGNED_LONGCONST:
	case TK_LLONGCONST:
	case TK_UNSIGNED_LLONGCONST:
		pri_expr->expr = create_token_node();
		pri_expr->compile_evaluated = TRUE;
		pri_expr->value = TK_VALUE_LONG(G_TK_VALUE);
		NEXT_TOKEN;
		break;
	case TK_FLOATCONST:
		pri_expr->expr = create_token_node();
		pri_expr->compile_evaluated = TRUE;
		pri_expr->value = TK_VALUE_FLOAT(G_TK_VALUE);
		NEXT_TOKEN;
		break;
	case TK_DOUBLECONST:
	case TK_LDOUBLECONST:
		pri_expr->expr = create_token_node();
		pri_expr->compile_evaluated = TRUE;
		pri_expr->value = TK_VALUE_DOUBLE(G_TK_VALUE);
		NEXT_TOKEN;
		break;
	case TK_STRING:
		pri_expr->expr = create_token_node();
		pri_expr->value = TK_VALUE_PTR(G_TK_VALUE);
		NEXT_TOKEN;
		break;
	case TK_LPAREN:
		pri_expr->kind = PRI_EXPR;
		SKIP(TK_LPAREN)
		pri_expr->expr = parse_expr();
		pri_expr->compile_evaluated = pri_expr->expr->compile_evaluated;
		pri_expr->value = ((expr_t*)(pri_expr->expr))->value;
		EXPECT(TK_RPAREN)
		break;
	default:
		ERROR("%s", "expect (, identfier, constant");
	}
	return pri_expr;
}

#define CREATE_POSTFIX_NODE(tmp_postfix, postfix)	tmp_postfix = postfix;												\
													postfix = (postfix_t *)bcc_malloc(sizeof(postfix_t));				\
													postfix->next = tmp_postfix

#define CREATE_DECL_POSTFIX_NODE(tmp_postfix, postfix)	tmp_postfix = postfix;												\
														postfix = (postfix_t *)bcc_malloc(sizeof(postfix_t));				\
														postfix->next = tmp_postfix


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
			if (postfix->expr->compile_evaluated == FALSE) {
				ERROR("expression must be constant");
			}
			postfix->value = postfix->expr->value;
			break;
		case TK_LPAREN:
			NEXT_TOKEN;
			kind = FUNC_POSTFIX;
			
			CREATE_POSTFIX_NODE(tmp_postfix, postfix);
			if (G_TK_KIND == TK_RPAREN) {
				postfix->expr = NULL;
				break;
			}
			postfix->expr = parse_argu_expr_list();
			break;
		case TK_DOT:
		case TK_POINTER:
			kind = STRCTURE_POSTFIX;

			CREATE_POSTFIX_NODE(tmp_postfix, postfix);
			postfix->op = G_TK_KIND;
			NEXT_TOKEN;
			postfix->ident = create_token_node();
			break;
		case TK_INC:
		case TK_DEC:
			kind = INC_DEC_POSTFIX;

			CREATE_POSTFIX_NODE(tmp_postfix, postfix);
			postfix->op = G_TK_KIND;
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
	
	postfix_expr->postfix = parse_postfix();

	return postfix_expr;
}

ast_node_t *parse_type_name()
{
	type_name_t *type_name;
	
	type_name = (type_name_t *)bcc_malloc(sizeof(type_name_t));
	
	type_name->spec_qual_list = parse_decl_spec(FALSE);

	if (G_TK_KIND != TK_RPAREN) {
		type_name->abs_decl = parse_abs_declarator();
	}
	return type_name;
}

ast_node_t *parse_unary_expr()
{
	unary_expr_t *unary_expr;
	int expr_kind;

	unary_expr = (unary_expr_t *)bcc_malloc(sizeof(unary_expr_t));
	switch (G_TK_KIND) {
	case TK_INC:
	case TK_DEC:
	case TK_MOD:
	case TK_MULTIPLY:
	case TK_ADD:
	case TK_SUB:
	case TK_BITREVERT:
	case TK_NOT:
		expr_kind = UNARY_EXPR;
		unary_expr->op = G_TK_KIND;
		NEXT_TOKEN;
		unary_expr->expr = parse_unary_expr();
		break;
	case TK_LPAREN:
		SAVE_CURR_COORDINATE;
		NEXT_TOKEN;
		if (is_decl_spec()) {
			expr_kind = CAST_EXPR;
			unary_expr->cast_type = parse_type_name();
			unary_expr->expr = parse_unary_expr();
		} else {
			expr_kind = POSTFIX_EXPR;
			BACK_TO_SAVED_COORDINATE;
			unary_expr->expr_kind = parse_postfix_expr();
		}
	case TK_SIZEOF:
		unary_expr->op = G_TK_KIND;
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

int get_binary_op_prec(int tk) {

	switch (tk) {
	case TK_OR:
		return 1;
	case TK_AND:
		return 2;
	case TK_BITOR:
		return 3;
	case TK_BITXOR:
		return 4;
	case TK_BITAND:
		return 5;
	case TK_NEQUAL:
	case TK_EQUAL:
		return 6;
	case TK_LESS:
	case TK_LESS_EQUAL:
	case TK_GREAT:
	case TK_GREAT_EQUAL:
		return 7;
	case TK_LSHIFT:
	case TK_RSHIFT:
		return 8;
	case TK_ADD:
	case TK_SUB:
		return 9;
	case TK_MULTIPLY:
	case TK_DIVIDE:
	case TK_MOD:
		return 10;
	default:
		return 0;
	}
}

ast_node_t *parse_binary_expr(int prev_prec)
{
	binary_expr_t *binary_expr;
	unary_expr_t *unary_expr;
	int curr_prec;

	unary_expr = parse_unary_expr();
	while (get_binary_op_prec(G_TK_KIND) >= prev_prec) {
		binary_expr = (binary_expr_t *)bcc_malloc(sizeof(binary_expr_t));
		binary_expr->op1 = unary_expr;
		binary_expr->op = G_TK_KIND;
		binary_expr->op2 = parse_binary_expr(curr_prec + 1);
		unary_expr = binary_expr;
	}
	return unary_expr;
}


ast_node_t *parse_cond_expr()
{
	cond_expr_t *cond_expr;

	cond_expr = (cond_expr_t *)bcc_malloc(sizeof(cond_expr_t));
	cond_expr->logic_or_expr = parse_binary_expr(0);
	cond_expr->expr = cond_expr->cond_expr = NULL;

	if (G_TK_KIND == TK_QUESTION) {
		NEXT_TOKEN;
		cond_expr->expr = parse_expr();
		if (G_TK_KIND != TK_COLON) {
			ERROR("expect: :");
		}
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
	assign_expr->assign_expr = assign_expr->next = NULL;

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

ast_node_t *parse_argu_expr_list()
{
	assign_expr_t *list, *iter_list;

	list = iter_list = parse_assign_expr();

	while (G_TK_KIND == TK_COMMA) {
		iter_list->next = parse_assign_expr();
		iter_list = iter_list->next;
	}

	return list;
}

ast_node_t *parse_expr()
{
	expr_t *expr;
	
	expr = (expr_t *)bcc_malloc(sizeof(expr_t));
	expr->assign_expr = parse_assign_expr();
	expr->next = NULL;

	if (G_TK_KIND == TK_COMMA) {
		NEXT_TOKEN;
		expr->next = parse_expr();
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

ast_node_t *parse_struct_declarator()
{
	declarator_t *struct_decl;

	struct_decl = (declarator_t *)bcc_malloc(sizeof(declarator_t));
	struct_decl->next = NULL;

	if (G_TK_KIND != TK_COLON) {
		struct_decl->pointer = parse_pointer();
		struct_decl->direct_declarator = parse_direct_declarator();
		if (G_TK_KIND == TK_COLON) {
			SKIP(TK_COLON);
			struct_decl->const_expr = parse_const_expr();
		}
	} else {
		SKIP(TK_COLON);
		struct_decl->const_expr = parse_const_expr();
	}
	return struct_decl;
}

ast_node_t *parse_struct_declarator_list()
{
	declarator_t *list, *list_iter;
	
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
	declaration_t *struct_decl;

	struct_decl = (declaration_t * *)bcc_malloc(sizeof(declaration_t));
	struct_decl->decl_spec = parse_decl_spec(FALSE);
	struct_decl->declarator_list = parse_struct_declarator_list();
	struct_decl->next = NULL;

	return struct_decl;
}

ast_node_t *parse_struct_declaration_list()
{
	declaration_t *list, *list_iter;
	
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
	struct_union->s_or_u = G_TK_KIND;

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
		ERROR("expect '{' or identifier");
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

	SKIP(TK_ENUM);
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
		ERROR("expect '{' or identifier");
	}
}

ast_node_t *parse_decl_spec(int with_store_cls)
{	
	decl_spec_t *decl_spec = NULL;
	BOOL invalid_decl_spec;

	decl_spec = (decl_spec_t *)bcc_malloc(sizeof(decl_spec_t));

	invalid_decl_spec = FALSE;
	while (1) {
		switch (G_TK_KIND) {
		case TK_TYPEDEF:
		case TK_EXTERN:
		case TK_STATIC:
		case TK_AUTO:
		case TK_REGISTER:
			if (!with_store_cls) {
				ERROR("unexpected store class");
			}
			if (decl_spec->store_cls.kind == 0) {
				decl_spec->store_cls.kind = G_TK_KIND;
			} else {
				ERROR("repeated storage class specifier");
			}
			break;
		case TK_VOID:
		case TK_CHAR:
		case TK_SHORT:
		case TK_INT:
		case TK_LONG:
		case TK_FLOAT:
		case TK_DOUBLE:
			if (decl_spec->type_spec.kind = 0) {
				decl_spec->type_spec.kind = G_TK_KIND;
			} else {
				ERROR("repeated type specifier");
			}
			break;
		case TK_SIGNED:
		case TK_UNSIGNED:
			if (decl_spec->type_spec.sign = 0) {
				decl_spec->type_spec.sign = G_TK_KIND;
			} else {
				ERROR("repeated type specifier ");
			}
		case TK_STRUCT:
		case TK_UNION:
			if (decl_spec->type_spec.kind = 0) {
				decl_spec->type_spec.kind = G_TK_KIND;
				decl_spec->type_spec.value = parse_struct_union();
			} else {
				ERROR("repeated type specifier");
			}
			break;
		case TK_ENUM:
			if (decl_spec->type_spec.kind = 0) {
				decl_spec->type_spec.kind = G_TK_KIND;
				decl_spec->type_spec.value = parse_enum();
			} else {
				ERROR("repeated type specifier");
			}
			break;
		case TK_IDENTIFIER:
			if (is_typedef_name(g_current_token.token_value.ptr)) {
				if (decl_spec->type_spec.kind = 0) {
					decl_spec->type_spec.kind = TK_IDENTIFIER;
					decl_spec->type_spec.value = create_token_node();
				} else {
					ERROR("repeated type specifier");
				}
			} else {
				invalid_decl_spec = TRUE;
			}
			break;
		case TK_CONST:
			if (decl_spec->type_qual.with_const == 0) {
				decl_spec->type_qual.with_const = TRUE;
			} else {
				ERROR("repeated const");
			}
			break;
		case TK_VOLATILE:
			if (decl_spec->type_qual.with_volatile == 0) {
				decl_spec->type_qual.with_volatile = TRUE;
			} else {
				ERROR("repeated const");
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
	
	return decl_spec;
}

ast_node_t *parse_direct_abs_declarator()
{
	direct_declarator_t *decl;

	decl = (direct_declarator_t *)bcc_malloc(sizeof(direct_declarator_t));
	decl->ident = decl->decl = decl->post = NULL;
	decl->is_abs_decl = TRUE;

	if (G_TK_KIND == TK_LPAREN) {
		SAVE_CURR_COORDINATE;
		NEXT_TOKEN;
		if (! is_decl_spec() 
			&& (G_TK_KIND == TK_POINTER 
			|| G_TK_KIND == TK_LBRACKET
			|| G_TK_KIND == TK_LPAREN)) {
			decl->decl = parse_abs_declarator();
			SKIP(TK_RPAREN);
		} else {
			BACK_TO_SAVED_COORDINATE;
		}
	}
	decl->post = parse_decl_postfix();
	return decl;
}

ast_node_t *parse_abs_declarator()
{
	declarator_t *decl;

	decl = (declarator_t *)bcc_malloc(sizeof(declarator_t));
	if (G_TK_KIND == TK_POINTER) {
		decl->pointer = parse_pointer();
		NEXT_TOKEN;
	}
	if (G_TK_KIND == TK_LPAREN
		|| G_TK_KIND == TK_LBRACKET)
		decl->direct_declarator = parse_direct_abs_declarator();
	return decl;
}

#define WITHOUT_DECL		1 << 0
#define WITH_DECL			1 << 1
#define WITH_ABSTRACT_DECL	1 << 2

int decl_type()
{
	if (G_TK_KIND != TK_POINTER
		&& G_TK_KIND != TK_LPAREN
		&& G_TK_KIND != TK_LBRACKET
		&& G_TK_KIND != TK_IDENTIFIER) {
		return WITHOUT_DECL;
	}
	if (G_TK_KIND == TK_POINTER) {
		NEXT_TOKEN;
	}
	while (G_TK_KIND == TK_LPAREN) {
		NEXT_TOKEN;
	}
	if (G_TK_KIND == TK_IDENTIFIER) {
		return WITHOUT_DECL;
	}
	return WITH_ABSTRACT_DECL;
}

ast_node_t *parse_param_declaration()
{
	declaration_t *param_decl;
	int type;

	param_decl = (declaration_t *)bcc_malloc(sizeof(declaration_t));
	param_decl->decl_spec = param_decl->declarator_list = param_decl->next = NULL;

	param_decl->decl_spec = parse_decl_spec(TRUE);
	if (param_decl->decl_spec->store_cls.kind == TK_TYPEDEF) {
		ERROR("invalid store class in param declaration");
	}
	SAVE_CURR_COORDINATE;
	type = decl_type();
	BACK_TO_SAVED_COORDINATE;
	if (type == WITH_DECL) {
		param_decl->declarator_list = parse_declarator();
	} else if (type == WITH_ABSTRACT_DECL) {
		param_decl->declarator_list = parse_abs_declarator();
	}
	return param_decl;
}

ast_node_t *parse_param_list()
{
	declaration_t *param_list, *param_iter;

	param_iter = param_list = parse_param_declaration();
	while (G_TK_KIND == TK_COMMA) {
		NEXT_TOKEN;
		if (G_TK_KIND == TK_ELLIPSE) {
			break;
		}
		param_iter->next = parse_param_list();
		param_iter = param_iter->next;
	}
	return param_list;
}

ast_node_t *parse_param_type_list()
{
	param_type_list_t *list;

	list = (param_type_list_t *)bcc_malloc(sizeof(param_type_list_t));
	list->param_list = parse_param_list();
	if (G_TK_KIND == TK_ELLIPSE) {
		list->with_ellipse = TRUE;
		NEXT_TOKEN;
	}
	return list;
}

ast_node_t *parse_ident_list()
{
	ident_list_t *ident_list, *ident_iter;

	ident_list = ident_iter = (ident_list_t*)bcc_malloc(sizeof(ident_list_t));
	ident_iter->ident = create_token_node();
	ident_iter->next = NULL;

	NEXT_TOKEN;
	while (G_TK_KIND == TK_COMMA) {
		ident_iter->next = (ident_list_t *)bcc_malloc(sizeof(ident_list_t));
		ident_iter = ident_list->next;	
	}
	return ident_list;
}

ast_node_t *parse_decl_postfix()
{
	decl_postfix_t *decl_postfix, *tmp_decl_postfix;

	decl_postfix = tmp_decl_postfix = NULL;
	while (1) {
		switch (G_TK_KIND) {
		case TK_LBRACKET:
			NEXT_TOKEN;
			CREATE_DECL_POSTFIX_NODE(tmp_decl_postfix, decl_postfix);
			decl_postfix->paren_or_barcket = BRACKET;
			if (G_TK_KIND == TK_RBRACKET) {
				decl_postfix->next = NULL;
				break;
			}
			decl_postfix->const_expr = parse_const_expr();
			break;
		case TK_LPAREN:
			NEXT_TOKEN;
			CREATE_DECL_POSTFIX_NODE(tmp_decl_postfix, decl_postfix);
			decl_postfix->paren_or_barcket = PAREN;
			if (G_TK_KIND == TK_RPAREN) {
				decl_postfix->next = NULL;
			} else if (G_TK_KIND == TK_IDENTIFIER) {
				decl_postfix->ident_list = parse_ident_list();
			} else {
				decl_postfix->param_list = parse_param_type_list();
			}
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
	direct_decl->decl = direct_decl->ident = direct_decl->post = NULL;

	if (G_TK_KIND == TK_IDENTIFIER) {
		direct_decl->ident = create_token_node();
	} else if (G_TK_KIND == TK_LPAREN) {
		NEXT_TOKEN;
		direct_decl->decl = parse_declarator();
		NEXT_TOKEN;
	} else {
		ERROR("invalid direct declarator%d", G_TK_KIND);
	}
	direct_decl->post = parse_decl_postfix();

	return direct_decl;
}

ast_node_t *parse_qual_list()
{
	int had_const, had_volatile;

	type_qual_t *type_qual_ptr = (type_qual_t *)bcc_malloc(sizeof(type_qual_t));

	had_volatile = had_const = FALSE;
	while (G_TK_KIND == TK_VOLATILE || G_TK_KIND == TK_CONST) {
		if (type_qual_ptr == NULL) {
			type_qual_ptr = bcc_malloc(sizeof(type_qual_t));
		}
		if (G_TK_KIND == TK_CONST) {
			if (had_const == TRUE) {
				ERROR("repeated const");
			}
			had_const = TRUE;
		}
		if (G_TK_KIND == TK_VOLATILE) {
			if (had_volatile == TRUE) {
				ERROR("repeated volatile");
			}
			had_volatile = TRUE;
		}
		NEXT_TOKEN;
	}

	type_qual_ptr->with_const = had_const;
	type_qual_ptr->with_volatile = had_volatile;
	return type_qual_ptr;
}

ast_node_t *parse_pointer()
{
	pointer_t *pointer = NULL;

	if (G_TK_KIND == TK_MULTIPLY) {
		pointer = (pointer_t *)bcc_malloc(sizeof(pointer_t));
		pointer->type_qual_ptr = pointer->next = NULL;
		pointer->pointer_num = 0;
		while (G_TK_KIND == TK_MULTIPLY) {
			pointer->pointer_num++;
			NEXT_TOKEN;
		}

		pointer->type_qual_ptr = parse_qual_list();
		pointer->next = parse_pointer();
	}
	return pointer;
}

ast_node_t *parse_declarator()
{
	declarator_t *decl;

	decl = (declarator_t *)bcc_malloc(sizeof(declarator_t));
	decl->pointer = decl->direct_declarator = NULL;

	decl->pointer = parse_pointer();
	decl->direct_declarator = parse_direct_declarator();
	return decl;
}

ast_node_t *parse_initializer_list()
{
	initializer_t *init_list, *init_iter;

	init_iter = init_list = parse_initializer();
	while (G_TK_KIND == TK_COMMA) {
		NEXT_TOKEN;
		init_iter->next = parse_initializer_list();
		init_iter = init_iter->next;
	}
	return init_list;
}

ast_node_t *parse_initializer()
{
	initializer_t *initializer;

	initializer = (initializer_t *)bcc_malloc(sizeof(initializer_t));
	initializer->assign_expr = initializer->initialier_list = initializer->next = NULL;
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
	declarator_t *decl;

	
	decl = parse_declarator();

	if (G_TK_KIND == TK_ASSIGN) {
		NEXT_TOKEN;
		decl->initializer = parse_initializer();
	}
	return decl;
}

ast_node_t *parse_init_declarator_list()
{
	declarator_t *list, *list_iter;

	list = list_iter = parse_init_declarator();
	while (G_TK_KIND == TK_COMMA) {
		NEXT_TOKEN;
		list_iter->next = parse_init_declarator_list();
		list_iter = list_iter->next;
	}
	return list;
}

ast_node_t *parse_declaration()
{
	declaration_t *decl;

	decl = (declaration_t *)bcc_malloc(sizeof(declaration_t));

	decl->decl_spec = parse_decl_spec(TRUE);
	decl->declarator_list = NULL;
	decl->next = NULL;

	if (G_TK_KIND != TK_SEMICOLON) {
		decl->declarator_list = parse_init_declarator_list();
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
		labeled_state->const_expr = parse_const_expr();
		SKIP(TK_COLON);
		break;
	default:
		ERROR("expeect identifier, 'default', 'case'");
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
	
	select_kind = G_TK_KIND;

	NEXT_TOKEN;
	SKIP(TK_LPAREN);
	select_state->fir_expr = parse_expr();
	SKIP(TK_RPAREN);
	select_state->statement = parse_statement();
	
	if (select_kind == TK_IF && G_TK_KIND == TK_ELSE) {
		select_kind = G_TK_KIND;
		NEXT_TOKEN;
		select_state->sec_expr = parse_statement();
	}
	select_state->kind = select_kind;
	return select_state;
}

ast_node_t *parse_iteration_statement()
{
	iteration_statement_t *iter_state;

	iter_state->kind = G_TK_KIND;
	
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
		ERROR("expect 'while', 'do', 'for'");
		break;
	}
	return iter_state;
}

ast_node_t *parse_jump_statement()
{
	jump_statement_t *jump_state;

	jump_state->kind = G_TK_KIND;

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
		ERROR("expect 'goto', 'return', 'continue', 'break'");
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
		parse_expr_statement();
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
		ERROR("expect '{'");
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
	external_declaration_t *external_decl;
	declaration_t *decl;
	declarator_t *declarator;
	func_def_t *func_def;
	decl_spec_t *decl_spec;
	

	decl = func_def = decl_spec = declarator = NULL;

	if (is_decl_spec()) {
		decl_spec = parse_decl_spec(TRUE);
	}

	if (G_TK_KIND != TK_SEMICOLON) {
		declarator = parse_init_declarator_list();
	}

	if (G_TK_KIND == TK_SEMICOLON) {
		decl = (declaration_t *)bcc_malloc(sizeof(declaration_t));
		decl->decl_spec = decl_spec;
		decl->declarator_list = declarator;
		external_decl->decl = decl;
		external_decl->kind = DECLARATION;
	} else if(G_TK_KIND == TK_LBRACE || is_decl_spec()) {
		func_def = (declaration_t *)bcc_malloc(sizeof(declaration_t));
		func_def->decl_spec = decl_spec;
		func_def->decl = declarator;
		if (is_decl_spec()) {
			func_def->decl_list = parse_declaration_list();
			EXPECT(TK_LBRACE);
		}
		func_def->comp_state_ptr = parse_compound_statement();

		external_decl->func = func_def;
		external_decl->kind = FUNC_DEFINATION;
	} else {
		ERROR("unexpected token");
	}
	return external_decl;
}