#include "bcc.h"


hashtable_t		g_external_symbol_table;
coordinate_t	g_recorded_coord;
BOOL			g_curr_expr_compile_computable = FALSE;
double			g_curr_expr_value;


BOOL is_typedef_name(char *name)
{
	symbol_t *sym = get_symbol(g_sym_tb, name);
	if (sym && sym->is_typedef) {
		return TRUE;
	}
	return FALSE;
}


#define NEXT_TOKEN get_next_token();		

#define G_TK_KIND	g_current_token.tk_kind
#define G_TK_VALUE	g_current_token.token_value
#define G_TK_LINE	g_current_token.line

#define TK_VALUE_INT(tk)	 tk.int_num
#define TK_VALUE_SHORT(tk)	 tk.short_num
#define TK_VALUE_LONG(tk)	 tk.long_num
#define TK_VALUE_FLOAT(tk)	 tk.float_num
#define TK_VALUE_DOUBLE(tk)	 tk.double_num
#define TK_VALUE_PTR(tk)	 tk.ptr

#define EXPECT(tk_kind) if (G_TK_KIND != tk_kind) {									\
							ERROR("expect token is:%d", tk_kind);		\
						}					

#define SKIP(tk_kind)   if (G_TK_KIND != tk_kind) {									\
							ERROR("expect token is:%d", tk_kind);		\
						}															\
						NEXT_TOKEN			

#define SAVE_CURR_COORDINATE	g_recorded_coord.g_cursor = G_CURSOR;				\
								g_recorded_coord.line = G_LINE;						\
								g_recorded_coord.colum = G_COLUM

#define BACK_TO_SAVED_COORDINATE	G_CURSOR = g_recorded_coord.g_cursor;			\
									G_LINE	 = g_recorded_coord.line;				\
									G_COLUM  = g_recorded_coord.colum

token_t* peek()
{
	SAVE_CURR_COORDINATE;								
	NEXT_TOKEN;										
	BACK_TO_SAVED_COORDINATE;
	return &g_current_token;
}

ast_node_t *create_token_node()
{
	ast_node_t *node;

	node = (ast_node_t*)bcc_malloc(sizeof(ast_node_t));
	bcc_memcpy(&node->tk_val, &g_current_token, sizeof(token_t));
	return node;
}

/*parse expression*/

expr_t *create_expr_node(int kind)
{
	expr_t *expr = (expr_t *)bcc_malloc(sizeof(expr_t));
	expr->kind = kind;
	expr->child_1 = expr->child_2 = NULL;
	expr->type = NULL;
	expr->value = 0;
	return expr;
}

ast_node_t *parse_primary_expr()
{
	expr_t *expr;
	symbol_t *sym;

	if (G_TK_KIND == TK_IDENTIFIER) {
		if (!in_symbol_table(g_sym_tb, G_TK_VALUE.ptr)) {
			ERROR("Undeclare varible %s", G_TK_VALUE.ptr);
		}
		sym = get_symbol(g_sym_tb, G_TK_VALUE.ptr);
		if (sym->is_typedef) {
			ERROR("Typedef name cannot be used as variable");
		}
		if (sym->is_enum_const) {
			create_expr_node(AST_CONST);
		} else {
			create_expr_node(AST_VAR);
		}
		expr->child_1 = create_token_node();
		NEXT_TOKEN;
		return expr;
	}
	if (G_TK_KIND == TK_LPAREN) {
		SKIP(TK_LPAREN);
		return parse_comma_expr();
		EXPECT(TK_RPAREN);
		return expr;
	}
	if (G_TK_KIND == TK_STRING) {
		expr = create_expr_node(AST_STR);
		expr->child_1 = create_token_node();
		NEXT_TOKEN;
	}

	expr = create_expr_node(AST_CONST);
	switch (G_TK_KIND) {
	case TK_INTCONST:
		expr->value = TK_VALUE_INT(G_TK_VALUE);
		expr->type = g_ty_int;
		break;
	case TK_UNSIGNED_INTCONST:
		expr->value = TK_VALUE_INT(G_TK_VALUE);
		expr->type = g_ty_uint;
		break;
	case TK_LONGCONST:
		expr->type = g_ty_long;
		expr->value = TK_VALUE_LONG(G_TK_VALUE);
		break;
	case TK_UNSIGNED_LONGCONST:
		expr->type = g_ty_ulong;
		expr->value = TK_VALUE_LONG(G_TK_VALUE);
		break;
	case TK_FLOATCONST:
		expr->value = TK_VALUE_FLOAT(G_TK_VALUE);
		expr->type = g_ty_float;
		break;
	case TK_DOUBLECONST:
		expr->value = TK_VALUE_DOUBLE(G_TK_VALUE);
		expr->type = g_ty_double;
		break;
	default:
		ERROR("%s", "expect (, identfier, constant");
	}
	NEXT_TOKEN;
	return expr;
}

expr_t *parse_subscript_expr(expr_t *expr)
{
	expr_t *array_expr;
	expr_t *subscript_expr;
	
	if (!is_pointer_type(expr->type)) {
		ERROR("subscripted value is neither array nor pointer");
	}
	SKIP(TK_LBRACKET);
	subscript_expr = expr_type_conv(parse_comma_expr());
	if (!is_integer_type(subscript_expr->type)) {
		ERROR("subscrpt expect integer type");
	}
	SKIP(TK_RBRACKET);

	array_expr = build_expr_node(AST_ARRAY, expr, subscript_expr);
	array_expr->type = expr->type->base_type;
	return array_expr;
}

expr_t *parse_func_expr(expr_t *expr)
{
	expr_t *param_expr;
	expr_t *func_expr;

	if (!is_pointer_type(expr->type) || !is_function_type(expr->type->base_type)) {
		ERROR("expect function identifier");
	}
	SKIP(TK_LPAREN);
	param_expr = parse_argu_expr_list();
	SKIP(TK_RPAREN);

	func_expr = build_expr_node(AST_FUNC, expr, parse_argu_expr_list());
	func_expr->type = ((function_type_t *)expr->type)->ret;
	return func_expr;
}

expr_t *parse_struct_field(expr_t *expr)
{
	type_t *struct_ty, *field_ty;
	expr_t *struct_field_expr;
	token_t *field_tk;
	int ast_kind;
	if (G_TK_KIND == TK_POINTER) {
		if (!is_pointer_type(expr->type)
			|| !is_record_type(expr->type->base_type)) {
			ERROR("expect pointer to struct or union type");
		}
		struct_ty = expr->type->base_type;
		ast_kind = AST_POINTER_TO;
	} else if (G_TK_KIND == TK_DOT) {
		if (!is_record_type(expr->type)) {
			ERROR("expect to struct or union type");
		}
		struct_ty = expr->type;
		ast_kind = AST_CONTAIN;
	}
	NEXT_TOKEN;
	EXPECT(TK_IDENTIFIER);
	
	field_tk = create_token_node();
	field_ty = get_tag_member_type(struct_ty, field_tk->token_value.ptr);
	if (field_ty == NULL) {
		ERROR("struct have no expect member");
	}
	build_expr_node(ast_kind, expr, field_tk);
	struct_field_expr->type = field_ty;
	return struct_field_expr;
}

BOOL is_lvaue(expr_t *expr)
{
	if (expr->kind == AST_VAR
		|| expr->kind == AST_ARRAY
		|| expr->kind == AST_POINTER_TO
		|| expr->kind == AST_CONTAIN) {
		return TRUE;
	}
	return FALSE;
}


expr_t *parse_self_inc_dec_expr(expr_t *expr, int kind)
{
	expr_t *inc_expr;
	if (!is_lvaue(expr)) {
		ERROR("lvalue required as increment operand");
	}
	if (!is_integer_type(expr->type) && !is_pointer_type(expr->type)) {
		ERROR("require integer type or pointer type");
	}

	build_expr_node(kind, expr, NULL);
	inc_expr->type = expr->type;
	
	return inc_expr;
}

ast_node_t *parse_postfix(expr_t *expr)
{
	while (1) {
		switch (G_TK_KIND) {
		case TK_LBRACKET:
			expr = parse_subscript_expr(expr);
			break;
		case TK_LPAREN:
			expr = parse_func_expr(expr);
			break;
		case TK_POINTER:
		case TK_DOT:
			expr = parse_struct_field(expr);
			break;
		case TK_INC:
			expr = parse_self_inc_dec_expr(expr, AST_POST_INC);
			break;
		case TK_DEC:
			expr = parse_self_inc_dec_expr(expr, AST_POST_DEC);
			break;
		default:
			return expr;
		}
		NEXT_TOKEN;
	}
}

ast_node_t *parse_postfix_expr()
{
	expr_t *postfix_expr;
	
	postfix_expr = parse_postfix(expr_type_conv(parse_primary_expr()));

	return postfix_expr;
}

type_t *parse_type_name()
{
	type_t *type;
	
	type = parse_decl_spec(FALSE);

	if (G_TK_KIND != TK_RPAREN) {
		type = parse_abs_declarator(type);
	}
	return type;
}

type_t *parse_sizeof_expr()
{
	type_t *type;
	if (G_TK_KIND == TK_LPAREN && is_decl_spec(peek())) {
		SKIP(TK_LPAREN)
		type = parse_type_name();
		SKIP(TK_RPAREN);
	} else {
		type = parse_unary_expr()->type;
	}
	if (is_function_type(type) || is_void_type(type)) {
		ERROR("invalid sizeof operand");
	}
	return type;
}

expr_t *parse_unary_expr()
{
	expr_t *unary_expr;
	expr_t *cast_expr;
	type_t *type;
	int expr_kind;

	switch (G_TK_KIND) {
	case TK_INC:
		SKIP(TK_INC);
		unary_expr = parse_self_inc_dec_expr(expr_type_conv(parse_unary_expr()), AST_PRFIX_INC);
		break;
	case TK_DEC:
		SKIP(TK_DEC);
		unary_expr = parse_self_inc_dec_expr(expr_type_conv(parse_unary_expr()), AST_PRFIX_INC);
		break;
	case TK_BITAND:
		SKIP(TK_BITAND);
		cast_expr = parse_cast_expr();
		if (!is_function_type(cast_expr->type) && !is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '&' operand");
		} 
		unary_expr = build_expr_node(AST_ADDR, cast_expr, NULL);
		unary_expr->type = derive_pointer_type(cast_expr->type, 0);
		break;
	case TK_MULTIPLY:
		SKIP(TK_MULTIPLY);
		cast_expr = expr_type_conv(parse_cast_expr());
		if (!is_pointer_type(cast_expr->type)) {
			ERROR("expect pointer type");
		}
		if (is_function_type(cast_expr->type->base_type)) {
			return cast_expr;
		}
		unary_expr = build_expr_node(AST_DREF, cast_expr, NULL);
		unary_expr->type = cast_expr->type->base_type;
		break;
	case TK_ADD:
		SKIP(TK_ADD);
		cast_expr = expr_type_conv(parse_cast_expr());
		if (!is_arith_type(cast_expr->type)) {
			ERROR("'+'expect arith type");
		}
		if (!is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '+' operand");
		}
		unary_expr = build_expr_node(AST_UNARY_PLUS, cast_expr, NULL);
		unary_expr->type = cast_expr->type;
		unary_expr = fold_const(unary_expr);
		break;
	case TK_SUB:
		SKIP(TK_SUB);
		cast_expr = expr_type_conv(parse_cast_expr());
		if (!is_arith_type(cast_expr->type)) {
			ERROR("'+'expect arith type");
		}
		if (!is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '-' operand");
		}
		unary_expr = build_expr_node(AST_UNARY_MINUS, cast_expr, NULL);
		unary_expr->type = cast_expr->type;
		unary_expr = fold_const(unary_expr);
		break;
	case TK_BITREVERT:
		SKIP(TK_BITREVERT);
		cast_expr = expr_type_conv(parse_cast_expr());
		if (!is_integer_type(cast_expr->type)) {
			ERROR("'+'expect arith type");
		}
		if (!is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '~' operand");
		}
		unary_expr = build_expr_node(AST_BITREVERT, cast_expr, NULL);
		unary_expr->type = cast_expr->type;
		unary_expr = fold_const(unary_expr);
		break;
	case TK_NOT:
		SKIP(TK_NOT);
		cast_expr = parse_cast_expr();
		if (!is_scalar_type(cast_expr->type)) {
			ERROR("lvalue required as unary '!' operand");
		}
		unary_expr = build_expr_node(AST_NOT, cast_expr, NULL);
		unary_expr->type = cast_expr->type;
		unary_expr = fold_const(unary_expr);
		break;
	case TK_SIZEOF:
		SKIP(TK_SIZEOF);
		unary_expr = create_expr_node(AST_CONST);
		unary_expr->type = g_ty_uint;
		unary_expr->value = parse_sizeof_expr()->size;
		break;
	default:
		unary_expr = parse_postfix_expr();
	}
	return unary_expr;
}

expr_t *parse_cast_expr()
{
	expr_t *cast_expr;
	
	if (G_TK_KIND == TK_LPAREN && is_decl_spec(peek())) {
		SKIP(TK_LPAREN);
		cast_expr = create_expr_node(AST_CAST);
		cast_expr->child_1 = parse_type_name();
		SKIP(TK_RPAREN);
		cast_expr->type = cast_expr->child_1;
		cast_expr->child_2 = expr_type_conv(parse_cast_expr());
		if (!is_scalar_type(cast_expr->child_1->type) || is_scalar_type(cast_expr->child_2->type)) {
			ERROR("invalid cast oeprand");
		}
		return cast_expr;
	}

	return parse_unary_expr();
}

BOOL is_integer_type(type_t *type)
{
	if (type->kind == TYPE_CHAR || type->kind == TYPE_SHORT || type->kind == TYPE_INT || type->kind == TYPE_LONG) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_floating_type(type_t *type)
{
	if (type->kind == TYPE_FLOAT || type->kind == TYPE_DOUBLE) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_arith_type(type_t *type)
{
	if (is_integer_type(type) || is_floating_type(type)) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_null_pointer(expr_t *expr)
{
	if (expr->kind == AST_CONST && expr->value == 0) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_pointer_type(type_t *type)
{
	if (type->kind == TYPE_POINTER) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_scalar_type(type_t *type)
{
	if (is_arith_type(type) || is_scalar_type(type)) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_const_expr(expr_t *expr)
{
	if (expr->kind == AST_CONST) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_struct_type(type_t *type)
{
	if (type->kind == TYPE_STRUCT) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_union_type(type_t *type)
{
	if (type->kind == TYPE_UNION) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_record_type(type_t *type)
{
	if (is_struct_type(type) || is_union_type(type)) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_function_type(type_t *type)
{
	if (type->kind == TYPE_FUNCTION) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_void_type(type_t *type)
{
	if (type->kind == TYPE_VOID) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_void_ptr(type_t *type)
{
	if (is_pointer_type(type) && is_void_type(type->base_type)) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_compatible_struct(tag_type_t *type1, tag_type_t *type2)
{
	if (is_record_type(type1) && is_record_type(type2)) {
		return type1 == type2;
	}
	return FALSE;
}

BOOL is_compatible_function(function_type_t *type1, function_type_t *type2)
{
	int i;
	param_type_t *param_1, *param_2;

	if (!is_compatible_type(type1->ret, type2->ret)
		|| type1->param_count != type2->param_count) {
		return FALSE;
	}
	i = 0;
	param_1 = type1->head;
	param_2 = type2->head;
	while (i < type1->param_count) {
		if (!is_compatible_type(param_1->type, param_2->type)) {
			return FALSE;
		}
		param_1 = param_1->next;
		param_2 = param_2->next;
	}
	return TRUE;
}

BOOL is_compatible_array(type_t *type1, type_t *type2)
{
	if (type1->size != type2->size) {
		return FALSE;
	}
	return is_compatible_array(type1->base_type, type2->base_type);
}

BOOL is_compatible_type(type_t *type1, type_t *type2)
{
	if (type1->kind != type2->kind) {
		return FALSE;
	}
	switch (type1->kind)
	{
	case TYPE_ARRAY:
		return is_compatible_array(type1, type2);
	case TYPE_FUNCTION:
		return is_compatible_function(type1, type2);
	case TYPE_STRUCT:
	case TYPE_UNION:
		return is_compatible_struct(type1, type2);
	case TYPE_POINTER:
		return is_compatible_ptr(type1, type2);
	default:
		return TRUE;
	}
}

BOOL is_compatible_ptr(type_t *type1, type_t *type2)
{
	if (type1->kind != TYPE_POINTER || type2->kind != TYPE_POINTER) {
		return FALSE;
	}
	return is_compatible_type(type1, type2);
}

type_t *usual_arith_conv(type_t *type1, type_t *type2)
{
	type_t *new_type;
	new_type = (type_t *)bcc_malloc(sizeof(type_t));
	if (type1->kind == TYPE_DOUBLE || type2->kind == TYPE_DOUBLE) {
		*new_type = *g_ty_double;
		return new_type;
	}
	if (type1->kind == TYPE_FLOAT || type2->kind == TYPE_FLOAT) {
		*new_type = *g_ty_float;
		return new_type;
	}
	if (type1->kind == TYPE_LONG && type1->sign == FALSE
		|| type2->kind == TYPE_LONG && type2->sign == FALSE) {
		*new_type = *g_ty_ulong;
		return new_type;
	}
	if (type1->kind == TYPE_LONG && type1->kind == TYPE_LONG) {
		*new_type = *g_ty_long;
		return new_type;
	}
	if (type1->kind == TYPE_INT && type1->size == FALSE
		|| type2->kind == TYPE_INT && type2->size == FALSE) {
		*new_type = *g_ty_uint;
		return new_type;
	}
	*new_type = *g_ty_int;

	return new_type;
}

BOOL is_const_expr(expr_t *expr)
{
	if (expr->type->kind == AST_CONST) {
		return TRUE;
	}
	return FALSE;
}

expr_t *fold_const(expr_t *expr)
{
	long value1, value2;
	if (!is_const_expr(expr->child_1) || !is_const_expr(expr->child_2)) {
		return expr;
	}
	
	value1 = expr->child_1->value;
	value2 = expr->child_2->value;

	switch (expr->kind) {
	case AST_UNARY_PLUS:
		expr->value = +value1;
	case AST_UNARY_MINUS:
		expr->value = -value1;
	case AST_BITREVERT:
		expr->value = ~value1;
	case AST_NOT:
		expr->value = !value1;
	case TK_MULTIPLY:
		expr->value = value1 * value2;
	case TK_DIVIDE:
		expr->value = value1 / value2;
	case TK_MOD:
		expr->value = value1 % value2;
	case TK_ADD:
		expr->value = value1 + value2;
	case TK_SUB:
		expr->value = value1 - value2;
	case TK_LSHIFT:
		expr->value = value2 << value2;
	case TK_RSHIFT:
		expr->value = value1 >> value2;
	case TK_LESS:
		expr->value = value1 < value2;
	case TK_GREAT:
		expr->value = value1 > value2;
	case TK_LESS_EQUAL:
		expr->value = value1 <= value2;
	case TK_GREAT_EQUAL:
		expr->value = value1 >= value2;
	case TK_EQUAL:
		expr->value = value1 == value2;
	case TK_NEQUAL:
		expr->value = value1 != value2;
	case TK_BITAND:
		expr->value = value1 & value2;
	case TK_BITXOR:
		expr->value = value1 ^ value2;
	case TK_BITOR:
		expr->value = value1 | value2;
	case TK_AND:
		expr->value = value1 && value2;
	case TK_OR:
		expr->value = value1 || value2;
	case TK_QUESTION:
		expr->value = ((cond_expr_t *)expr)->logical_or->value ? value1 : value2;
	}
	expr->kind = AST_CONST;
	return expr;
}

expr_t *build_expr_node(int ast_kind, expr_t *child_1, expr_t *child_2)
{
	expr_t *expr;

	expr = create_expr_node(ast_kind);
	expr->child_1 = child_1;
	expr->child_2 = child_2;
	return expr;
}

expr_t *parse_multi_expr()
{
	expr_t *multi_expr, *child1, *child2;

	multi_expr = expr_type_conv(parse_cast_expr());
	while (G_TK_KIND == TK_MULTIPLY || G_TK_KIND == TK_DIVIDE || G_TK_KIND == TK_MOD) {
		child1 = multi_expr;
		child2 = expr_type_conv(parse_cast_expr());

		if (G_TK_KIND == TK_MOD) {
			if (!is_integer_type(child1->type) || !is_integer_type(child2->type)) {
				ERROR("invalid operand");
			}
		} else {
			if (!is_arith_type(child1->type) || !is_arith_type(child2->type)) {
				ERROR("invalid operand");
			}
		}

		multi_expr = build_expr_node(G_TK_KIND, child1, child2);
		multi_expr->type = usual_arith_conv(child1->type, child2->type);
		fold_const(multi_expr);
	}
	return multi_expr;
}

expr_t *parse_addit_expr()
{
	expr_t *addit_expr, *child_1, *child_2;
	type_t *type1, *type2;

	addit_expr = expr_type_conv(parse_multi_expr());
	while (G_TK_KIND == TK_ADD || G_TK_KIND == TK_SUB) {
		child_1 = addit_expr;
		child_2 = expr_type_conv(parse_multi_expr());

		type1 = child_1->type;
		type2 = child_2->type;
		if (is_arith_type(type1) && is_arith_type(type2)) {
			addit_expr = build_expr_node(G_TK_KIND, child_1, child_2);
			addit_expr->type = usual_arith_conv(type1, type2);
			fold_const(addit_expr);
			continue;
		}
		if (is_pointer_type(type1) && is_pointer_type(type2)) {
			if (G_TK_KIND != TK_SUB) {
				ERROR("invalid operand");
			}
			if (!is_compatible_ptr(type1, type2)) {
				WARN("invalid operand");
			}
			addit_expr = build_expr_node(G_TK_KIND, child_1, child_2);
			addit_expr->type = g_ty_int;
			fold_const(addit_expr);
			continue;
		}
		if (is_pointer_type(type1) && is_integer_type(type2)) {
			addit_expr = build_expr_node(G_TK_KIND, child_1, child_2);
			addit_expr->type = child_1->type;
			fold_const(addit_expr);
			continue;
		}
		if (is_integer_type(type1) && is_pointer_type(type2)) {
			if (G_TK_KIND == TK_SUB) {
				ERROR("invalid operand");
			}
			addit_expr = build_expr_node(G_TK_KIND, child_1, child_2);
			addit_expr->type = child_2->type;
			fold_const(addit_expr);
			continue;
		}
		ERROR("invalid operand");
	}
	return addit_expr;
}

expr_t *parse_shift_epxr()
{
	expr_t *shift_expr, *child1, *child2;

	shift_expr = expr_type_conv(parse_addit_expr());
	while (G_TK_KIND == TK_LSHIFT || G_TK_KIND == TK_RSHIFT) {
		child1 = shift_expr;
		child2 = expr_type_conv(parse_addit_expr());
		if (!is_integer_type(child1->type) || !is_integer_type(child2->type)) {
			ERROR("invalid operands");
		}
		shift_expr = build_expr_node(G_TK_KIND, child1, child2);
		shift_expr->type = child1->type;
		fold_const(shift_expr);
	}
	return shift_expr;
}

expr_t *parse_realtional_expr()
{
	expr_t *rela_expr, *child1, *child2;
	type_t *type1, *type2;

	rela_expr = expr_type_conv(parse_shift_epxr());
	while (G_TK_KIND == TK_LESS || G_TK_KIND == TK_LESS_EQUAL
		|| G_TK_KIND == TK_GREAT || G_TK_KIND == TK_GREAT_EQUAL) {
		child1 = rela_expr;
		child2 = expr_type_conv(parse_shift_epxr());
		type1 = child1->type;
		type2 = child2->type;
		if (!(is_arith_type(type1) && is_arith_type(type2))
			&& !(is_pointer_type(type1) && is_pointer_type(type2))) {
			ERROR("invalid operands");
		}
		if (is_pointer_type(type1) && is_pointer_type(type2)
			&& !is_compatible_ptr(type1, type2)) {
			WARN("incompatible pointer");
		}

		rela_expr = build_expr_node(G_TK_KIND, child1, child2);
		rela_expr->type = g_ty_int;
		fold_const(rela_expr);
	}
	return rela_expr;
}

expr_t *parse_equality_expr()
{
	expr_t *equality_expr, *child1, *child2;
	type_t *type1, *type2;

	equality_expr = expr_type_conv(parse_realtional_expr());
	while (G_TK_KIND == TK_EQUAL || G_TK_KIND == TK_NEQUAL) {
		child1 = equality_expr;
		child2 = expr_type_conv(parse_realtional_expr());
		type1 = child1->type;
		type2 = child2->type;
		if (is_arith_type(type1) && is_arith_type(type2)
			|| is_pointer_type(type1) && is_pointer_type(type2)
			|| is_pointer_type(type1) && is_null_pointer(child2)
			|| is_null_pointer(child1) && is_pointer_type(type2)) {
			equality_expr = build_expr_node(G_TK_KIND, child1, child2);
			equality_expr->type = g_ty_int;
		} else {
			ERROR("invalid operand");
		}
		fold_const(equality_expr);
	}
	return equality_expr;
}

expr_t *parse_bit_and_expr()
{
	expr_t *bit_and_expr, *child1, *child2;

	bit_and_expr = expr_type_conv(parse_equality_expr());
	while (G_TK_KIND == TK_BITAND) {
		child1 = bit_and_expr;
		child2 = expr_type_conv(parse_equality_expr());
		if (!is_integer_type(child1->type) || !is_integer_type(child2->type)) {
			ERROR("invalid operand");
		}
		bit_and_expr = build_expr_node(G_TK_KIND, child1, child2);
		bit_and_expr->type = child1->type;
		fold_const(bit_and_expr);
	}
	return bit_and_expr;
}

expr_t *parse_exclusive_or_expr()
{
	expr_t *excl_expr, *child1, *child2;
	
	excl_expr = expr_type_conv(parse_bit_and_expr());
	while (G_TK_KIND == TK_BITXOR) {
		child1 = excl_expr;
		child2 = expr_type_conv(parse_bit_and_expr());
		if (!is_integer_type(child1->type) || !is_integer_type(child2->type)) {
			ERROR("invalid operand");
		}
		excl_expr = build_expr_node(G_TK_KIND, child1, child2);
		excl_expr->type = child1->type;
		fold_const(excl_expr);
	}
	return excl_expr;
}

expr_t *parse_inclusive_or_expr()
{
	expr_t *inclu_or_expr, *child1, *child2;

	inclu_or_expr = expr_type_conv(parse_bit_and_expr());
	while (G_TK_KIND == TK_BITOR) {
		child1 = inclu_or_expr;
		child2 = expr_type_conv(parse_bit_and_expr());
		if (!is_integer_type(child1->type) || !is_integer_type(child2->type)) {
			ERROR("invalid operand");
		}
		inclu_or_expr = build_expr_node(G_TK_KIND, child1, child2);
		inclu_or_expr->type = child1->type;
		fold_const(inclu_or_expr);
	}
	return inclu_or_expr;
}

expr_t *parse_logic_and_expr()
{
	expr_t *logic_and, *child1, *child2;
	
	logic_and = expr_type_conv(parse_inclusive_or_expr());
	while (G_TK_KIND == TK_AND) {
		child1 = logic_and;
		child2 = expr_type_conv(parse_inclusive_or_expr());
		if (!is_scalar_type(child1->type) || !is_scalar_type(child2->type)) {
			ERROR("invalid operand");
		}
		logic_and = build_expr_node(G_TK_KIND, child1, child2);
		logic_and->type = g_ty_int;
		fold_const(logic_and);
	}
	return logic_and;
}

expr_t *parse_logic_or_expr()
{
	expr_t *logic_or, *child1, *child2;

	logic_or = expr_type_conv(parse_inclusive_or_expr());
	while (G_TK_KIND == TK_OR) {
		child1 = logic_or;
		child2 = expr_type_conv(parse_inclusive_or_expr());
		if (!is_scalar_type(child1->type) || !is_scalar_type(child2->type)) {
			ERROR("invalid operand");
		}
		logic_or = build_expr_node(G_TK_KIND, child1, child2);
		logic_or->type = g_ty_int;
		fold_const(logic_or);
	}
	return logic_or;
}

cond_expr_t *parse_cond_expr()
{
	cond_expr_t *cond_expr, *child_cond_expr;
	expr_t *logic_or_expr, *expr;
	type_t *type_1, *type_2;

	logic_or_expr = expr_type_conv(parse_logic_or_expr());

	if (G_TK_KIND == TK_QUESTION) {
		cond_expr = bcc_malloc(sizeof(cond_expr_t));
		cond_expr->logical_or = logic_or_expr;
		if (!is_scalar_type(logic_or_expr->type)) {
			ERROR("The first expression shall be scalar type");
		}
		cond_expr->expr.child_1 = expr_type_conv(parse_comma_expr());
		if (G_TK_KIND != TK_COLON) {
			ERROR("expect colon");
		}
		SKIP(TK_COLON);
		cond_expr->expr.child_2 = expr_type_conv(parse_cond_expr());
		type_1 = cond_expr->expr.child_1->type;
		type_2 = cond_expr->expr.child_2->type;
		if (is_arith_type(type_1) && is_arith_type(type_2)) {
			cond_expr->expr.type = usual_arith_conv(type_1, type_2);
		} else if (is_compatible_struct(type_1, type_2)) {
			cond_expr->expr.type = type_1;
		} else if (is_void_type(type_1) && is_void_type(type_2)) {
			cond_expr->expr.type = type_1;
		} else if (is_pointer_type(type_1) && is_pointer_type(type_2)) {
			cond_expr->expr.type = type_1;
		} else if (is_pointer_type(type_1) && is_null_pointer(cond_expr->expr.child_2)) {
			cond_expr->expr.type = type_1;
		} else if (is_null_pointer(cond_expr->expr.child_1) && is_null_pointer(cond_expr->expr.child_2)) {
			cond_expr->expr.type = type_2;
		} else {
			ERROR("invalid operand");
		}
		return fold_const(cond_expr);
	}

	return logic_or_expr;
}

expr_t *parse_assign_expr()
{
	expr_t *assign_expr, *child1, *child2;
	type_t *type_1, *type_2;

	assign_expr = expr_type_conv(parse_cond_expr());
	while (G_TK_KIND == TK_ASSIGN
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
		if (!is_lvaue(assign_expr)) {
			ERROR("expect lvalue");
		}
		child1 = assign_expr;
		NEXT_TOKEN;
		child2 = expr_type_conv(parse_assign_expr());

		type_1 = child1->type;
		type_2 = child2->type;
		assign_expr = build_expr_node(G_TK_KIND, child1, child2);
		if (is_arith_type(type_1) && is_arith_type(type_2)) {
			assign_expr->type = usual_arith_conv(type_1, type_2);
		} else if (is_compatible_struct(type_1, type_2)) {
			assign_expr->type = type_1;
		} else if (is_void_type(type_1) && is_void_type(type_2)) {
			assign_expr->type = type_1;
		}else if (is_pointer_type(type_1) && is_pointer_type(type_2)) {
			assign_expr->type = type_1;
		} else if (is_pointer_type(type_1) && is_null_pointer(child2)) {
			assign_expr->type = type_1;
		} else if (is_null_pointer(child1) && is_null_pointer(child2)) {
			assign_expr->type = type_2;
		}
		else {
			ERROR("invalid assign operand");
		}
	}
	return assign_expr;
}

expr_t *parse_comma_expr()
{
	expr_t *comma_expr, *child1, *child2;

	comma_expr = expr_type_conv(parse_assign_expr());
	while (G_TK_KIND == TK_COMMA) {
		child1 = comma_expr;
		child2 = expr_type_conv(parse_assign_expr());
		comma_expr = build_expr_node(G_TK_KIND, child1, child2);
		comma_expr->type = child2->type;
	}
	return comma_expr;
}

expr_t *parse_const_expr()
{
	expr_t *const_expr;
	const_expr = parse_cond_expr();

	if (!is_const_expr(const_expr) || !is_integer_type(const_expr->type)) {
		ERROR("expect integer type");
	}
	return const_expr;
}

/*parse statement*/

void parse_struct_declarator(tag_type_t *tag_type, type_t *base_type)
{
	type_t *field_ty;
	char *name;
	int bits;
	expr_t *const_expr;

	field_ty = NULL;
	if (G_TK_KIND != TK_COLON) {
		field_ty = parse_declarator(base_type, &name);
	}

	bits = -1;
	if (G_TK_KIND == TK_COLON) {
		SKIP(TK_COLON);
		const_expr = parse_const_expr();
		bits = const_expr->value;
	}
	if (!field_ty || bits == -1) {
		ERROR("invalid struct declaration");
	}
	add_field_to_tag(tag_type, field_ty, name, bits);
}

void parse_struct_declaration(tag_type_t *tag_type)
{
	type_t *base_type, *type;

	base_type = parse_decl_spec(FALSE);

	parse_struct_declarator(tag_type, base_type);
	while (G_TK_KIND == TK_COMMA) {
		parse_struct_declarator(tag_type, base_type);
	}
	SKIP(TK_SEMICOLON);
}

void parse_struct_declaration_list(tag_type_t *type)
{
	parse_struct_declaration(type);
	while (G_TK_KIND != TK_RBRACE) {
		parse_struct_declaration(type);
	}
}

type_t *parse_struct_union()
{
	int struct_or_union, has_declaration;
	char *tag_name;
	type_t *tag_type;
	user_define_type_t *before_type;

	struct_or_union = (G_TK_KIND == TK_STRUCT) ? TYPE_STRUCT : TYPE_UNION;
	NEXT_TOKEN;

	tag_name = NULL;
	if (G_TK_KIND == TK_IDENTIFIER) {
		tag_name = G_TK_VALUE.ptr;
		SKIP(TK_IDENTIFIER);
	}

	has_declaration = (peek()->tk_kind == TK_LBRACE) ? TRUE : FALSE;
	if (tag_name && is_curr_scope_define_type(g_tag_tb, tag_name)) {
		before_type = get_user_def(g_tag_tb, tag_name);
		if (before_type->type->kind != struct_or_union) {
			ERROR("defined as wrong kind of tag");
		}
		if (before_type->has_declaration == TRUE && has_declaration) {
			ERROR("redefinition tag");
		}
		if (has_declaration) {
			SKIP(TK_LBRACE);
			before_type->has_declaration = TRUE;
			parse_struct_declaration_list(before_type->type);
			return before_type->type;
		}
		return before_type->type;
	}

	tag_type = create_tag_type(tag_name, struct_or_union);
	if (tag_name) {
		insert_to_user_define_type(g_tag_tb, tag_name, tag_type, has_declaration);
	}
	if (has_declaration) {
		SKIP(TK_LBRACE);
		parse_struct_declaration_list(tag_type);
	}
	return tag_type;
}

int parse_enumerator(val)
{
	char *name;
	expr_t *const_expr;

	if (G_TK_KIND != TK_IDENTIFIER) {
		ERROLR("expect identifier");
	}
	name = G_TK_VALUE.ptr;
	
	SKIP(TK_IDENTIFIER);
	if (G_TK_KIND == TK_ASSIGN) {
		const_expr = parse_const_expr;
		val = const_expr->value;
	}
	if (in_curr_scope_sym_tb(g_sym_tb, name)) {
		ERROR("redeclare varible");
	}
	insert_to_sym_table(name, g_ty_int, FALSE, TRUE, val);
	return val + 1;
}

void parse_enum_list()
{
	int val = 0;

	val = parse_enumerator(val);
	while (G_TK_KIND == TK_COMMA) {
		val = parse_enumerator(val);
	}
}

type_t *parse_enum()
{
	int  has_declaration;
	char *tag_name;
	type_t *tag_type;
	user_define_type_t *before_type;

	NEXT_TOKEN;

	tag_name = NULL;
	if (G_TK_KIND == TK_IDENTIFIER) {
		tag_name = G_TK_VALUE.ptr;
		SKIP(TK_IDENTIFIER);
	}

	has_declaration = (peek()->tk_kind == TK_LBRACE) ? TRUE : FALSE;
	if (tag_name && is_curr_scope_define_type(g_tag_tb, tag_name)) {
		before_type = get_user_def(g_tag_tb, tag_name);
		if (before_type->type->kind != TYPE_ENUM) {
			ERROR("defined as wrong kind of tag");
		}
		if (before_type->has_declaration == TRUE && has_declaration) {
			ERROR("redefinition tag");
		}
		if (has_declaration) {
			SKIP(TK_LBRACE);
			before_type->has_declaration = TRUE;
			parse_struct_declaration_list(before_type->type);
			return before_type->type;
		}
		return before_type->type;
	}

	if (tag_name) {
		insert_to_user_define_type(g_tag_tb, tag_name, g_ty_int, has_declaration);
	}
	if (has_declaration) {
		SKIP(TK_LBRACE);
		parse_struct_declaration_list(tag_type);
	}
	return tag_type;
}

decl_spec_t *create_decl_spec()
{
	decl_spec_t *decl_spec;
	type_spec_t *type_spec;
	type_qual_t *type_qual;
	store_cls_spec_t *store_cls;

	type_spec = (type_spec_t *)bcc_malloc(sizeof(type_spec_t));
	type_spec->kind = 0;
	type_spec->sign = 0;
	type_spec->value = NULL;

	type_qual = (type_qual_t *)bcc_malloc(sizeof(type_qual_t));
	type_qual->qual = 0;

	store_cls = (store_cls_spec_t *)bcc_malloc(sizeof(type_qual_t));
	store_cls->kind = 0;
	
	decl_spec = (decl_spec_t *)bcc_malloc(sizeof(decl_spec_t));
	decl_spec->type_spec = type_spec;
	decl_spec->type_qual = type_qual;
	decl_spec->store_cls = store_cls;
	return decl_spec;
}

type_t *parse_decl_spec(int can_with_store_cls)
{	
	BOOL invalid_decl_spec;
	int type_spec, size, store_cls, qual, sign;
	type_t *type, *user_def_type;

	invalid_decl_spec = FALSE;
	type_spec = store_cls = qual = sign = size = 0;
	while (1) {
		switch (G_TK_KIND) {
		case TK_TYPEDEF:
		case TK_EXTERN:
		case TK_STATIC:
		case TK_AUTO:
		case TK_REGISTER:
			if (!can_with_store_cls) {
				ERROR("unexpected store class");
			}
			if (store_cls) {
				ERROR("repeated store class");
			}
			store_cls = G_TK_KIND;
			break;
		case TK_VOID:
			if (type_spec) ERROR("repeated type specifier"); type_spec = G_TK_KIND; break;
		case TK_CHAR:								
			if (type_spec) ERROR("repeated type specifier"); type_spec = G_TK_KIND; break;
		case TK_INT:								
			if (type_spec) ERROR("repeated type specifier"); type_spec = G_TK_KIND; break;
		case TK_FLOAT:								
			if (type_spec) ERROR("repeated type specifier"); type_spec = G_TK_KIND; break;
		case TK_DOUBLE:								
			if (type_spec) ERROR("repeated type specifier"); type_spec = G_TK_KIND; break;
		case TK_SHORT:
			if (size) ERROR("repeated type specifier"); size = G_TK_KIND; break;
		case TK_LONG:
			if (size) ERROR("repeated type specifier"); size = G_TK_KIND; break;
		case TK_SIGNED:
		case TK_UNSIGNED:
			if (sign) ERROR("repeated sign"); sign = G_TK_KIND; break; 
		case TK_STRUCT:
		case TK_UNION:
			if (type_spec) ERROR("repeated type specifier"); type_spec = G_TK_KIND; user_def_type = parse_struct_union(); break;
			break;
		case TK_ENUM:
			if (type_spec) ERROR("repeated type specifier"); type_spec = G_TK_KIND; user_def_type = parse_enum(); break;
			break;
		case TK_IDENTIFIER:
			if (is_typedef_name(g_current_token.token_value.ptr)) {
				if (type_spec) {
					ERROR("repeated type specifier");
				}
				type_spec = G_TK_KIND;
				user_def_type = get_symbol_type(g_sym_tb, g_current_token.token_value.ptr);
			} else {
				invalid_decl_spec = TRUE;
			}
			break;
		case TK_CONST:
			if (!(qual | WITH_CONST)) {
				qual |= WITH_CONST;
			} else {
				ERROR("repeated const");
			}
			break;
		case TK_VOLATILE:
			if (!(qual | WITH_VOLATILE)) {
				qual |= WITH_VOLATILE;
			} else {
				ERROR("repeated voliatile");
			}
			break;
		default:
			invalid_decl_spec = TRUE;
		}
		if (invalid_decl_spec) {
			break;
		}
		NEXT_TOKEN;
	}
	if (user_def_type != NULL) {		//struct ,union , enum, typedef
		if (sign && size) {
			ERROR("two or more data type in type spec");
		}
		if (qual == 0) {
			type = user_def_type;
		} else {
			switch (type_spec) {
			case TK_STRUCT:
			case TK_UNION:
				type = (tag_type_t *)bcc_malloc(sizeof(tag_type_t *));
				bcc_memcpy(type, user_def_type, sizeof(tag_type_t));
				break;
			case TK_TYPEDEF:
				type = (type_t *)bcc_malloc(sizeof(type_t *));
				*type = *user_def_type;
				break;
			case TK_ENUM:
				type = (enum_t *)bcc_malloc(sizeof(enum_t *));
				bcc_memcpy(type, user_def_type, sizeof(enum_t));
				break;
			}
			type->qual = qual;
		}
	} else {
		if (size == TK_SHORT && (type_spec != TK_INT && type_spec != TK_VOID)) {
			ERROR("size and type not match");
		}
		if (size == TK_LONG && (type_spec != TK_VOID && type_spec != TK_INT && type_spec != TK_DOUBLE)) {
			ERROR("size and type not match");
		}
		if (sign && (type_spec == TK_VOID || type_spec == TK_FLOAT || type_spec == TK_DOUBLE)) {
			ERROR("sign and type not match");
		}
		if (!sign && !size && !type_spec) {
			ERROR("with out type spec");
		}

		type = (type_t*)bcc_malloc(sizeof(type_t));
		if (size) {
			if (size == TK_SHORT) {
				*type = *g_ty_short;
			} else if (size = TK_LONG) {
				*type = *g_ty_long;
			}
		} else {
			switch (type_spec) {
			case TK_VOID:
				*type = *g_ty_void; break;
			case TK_CHAR:
				*type = *g_ty_char; break;
			case TK_INT:
				*type = *g_ty_int; break;
			case TK_FLOAT:
				*type = *g_ty_float; break;
			case TK_DOUBLE:
				*type = *g_ty_double; break;
			default:
				*type = *g_ty_int; break;
			}
		}
		type->sign = sign;
		type->qual = qual;
	}
	type->store_cls = store_cls;
	return type;
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

type_t *parse_param_declarator(type_t *base_type)
{
	type_t *type, *dummy_type, *real_type;

	type = parse_pointer(base_type);
	
	if (G_TK_KIND == TK_IDENTIFIER) {
		type = parse_decl_postfix(base_type);
	}
	if (G_TK_KIND == TK_LPAREN) {
		SAVE_CURR_COORDINATE;
		SKIP(TK_LPAREN);
		if (is_decl_spec() || G_TK_KIND == TK_RPAREN) {		//function decl
			BACK_TO_SAVED_COORDINATE;
			type = parse_decl_postfix(type);
		} else {											//abstract decl
			dummy_type = (type_t *)bcc_malloc(sizeof(type_t));
			type = parse_param_declarator(dummy_type);			/*because we don't know base whether declarator has postfix, so we don't know its base type and we use dummy*/
			SKIP(TK_RPAREN);

			real_type = parse_decl_postfix(type);
			*dummy_type = *real_type;
			bcc_free(real_type);
		}
	}
	if (G_TK_KIND == TK_LBRACKET) {
		type = parse_decl_postfix(base_type);
	}
	return type;
}

void parse_param_declaration(function_type_t *func_type)
{
	type_t *spec_type, *type;

	spec_type = parse_decl_spec(TRUE);
	if (spec_type->store_cls == TK_TYPEDEF) {
		ERROR("invalid store class in param declaration");
	}
	type = parse_param_declarator(spec_type);
	add_param_to_func(func_type, type, NULL);
}

void parse_param_type_list(function_type_t *func_type)
{
	if (G_TK_KIND == TK_RPAREN) {
		SKIP(TK_RPAREN);
		return;
	}
	parse_param_declaration(func_type);
	while (G_TK_KIND == TK_COMMA) {
		NEXT_TOKEN;
		if (G_TK_KIND == TK_ELLIPSE) {
			break;
		}
		parse_param_declaration(func_type);
	}
	if (G_TK_KIND == TK_ELLIPSE) {
		func_type->with_ellipse = TRUE;
		NEXT_TOKEN;
	}
	SKIP(TK_RPAREN);
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

type_t *parse_decl_postfix(type_t *base_type)
{
	type_t *type = NULL;
	expr_t *const_expr;
	ast_node_t *param_list;
	int len;

	switch (G_TK_KIND) {
	case TK_LBRACKET:
		SKIP(TK_LBRACKET);
		if (G_TK_KIND == TK_RBRACKET) {
			len = -1;
		} else {
			const_expr = parse_const_expr();
			len = const_expr->value;
		}
		SKIP(TK_RBRACKET);

		base_type = parse_decl_postfix(base_type);
		if (type->kind == TYPE_ARRAY) {
			ERROR("declared as function returning an array");
		}
		return derive_array_type(base_type, len);
	case TK_LPAREN:
		SKIP(TK_LPAREN);

		type = create_func_type(NULL, type);
		if (G_TK_KIND != TK_RPAREN) {
			parse_param_type_list(type);
		}
		SKIP(TK_RPAREN);
		base_type = parse_decl_postfix(base_type);
		if (type->kind == TYPE_ARRAY) {
			ERROR("declared as function returning an array");
		}
		if (type->kind = TYPE_FUNCTION) {
			ERROR("declaration a array of function");
		}
		((function_type_t *)type)->ret = base_type;
		return type;
	default:
		return base_type;
	}
}

int parse_qual_list()
{
	int qual = 0;

	while (G_TK_KIND == TK_VOLATILE || G_TK_KIND == TK_CONST) {
		if (G_TK_KIND == TK_CONST) {
			if (qual | WITH_CONST) {
				ERROR("repeated const");
			}
			qual |= WITH_CONST;
		}
		if (G_TK_KIND == TK_VOLATILE) {
			if (qual | WITH_VOLATILE) {
				ERROR("repeated volatile");
			}
			qual |= WITH_VOLATILE;
		}
		NEXT_TOKEN;
	}
	return qual;
}

type_t *parse_pointer(type_t *base_type)
{
	pointer_t *type;

	type = base_type;
	while (G_TK_KIND == TK_MULTIPLY) {
		NEXT_TOKEN;
		type = derive_pointer_type(type, parse_qual_list());
	}
	return type;
}

type_t *parse_direct_declarator(type_t *base_type, char **name)
{
	type_t *type;
	type_t *dummy_type, *real_type;

	if (G_TK_KIND == TK_IDENTIFIER) {
		*name = G_TK_VALUE.ptr;
		if (in_curr_scope_sym_tb(g_sym_tb, *name)) {
			ERROR("redeclare symbol");
		}
		type = base_type;
		SKIP(TK_IDENTIFIER);
		return parse_decl_postfix(type);
	} else if (G_TK_KIND == TK_LPAREN) {
		SKIP(TK_LPAREN);
		dummy_type = (type_t *)bcc_malloc(sizeof(type_t));
		type = parse_declarator(dummy_type, name);			/*because we don't know base whether declarator has postfix, so we don't know its base type and we use dummy*/
		SKIP(TK_RPAREN);

		real_type = parse_decl_postfix(base_type);
		*dummy_type = *real_type;
		bcc_free(real_type);
		return type;
	} else {
		ERROR("invalid direct declarator%d", G_TK_KIND);
	}
}

type_t *parse_declarator(type_t *base_type, char** name)
{
	type_t *type;

	type = parse_pointer(base_type);
	type = parse_direct_declarator(type, name);
	
	return type;
}

type_t *parse_direct_abs_declarator(type_t *base_type)
{
	type_t *type, *dummy_type, *tmp_type;
	if (G_TK_KIND == TK_LPAREN) {
		SAVE_CURR_COORDINATE;
		SKIP(TK_LPAREN);
		if ((G_TK_KIND == TK_POINTER
				|| G_TK_KIND == TK_LBRACKET
				|| G_TK_KIND == TK_LPAREN)) {
			type = parse_abs_declarator(base_type);
			dummy_type = (type_t *)bcc_malloc(sizeof(type_t));
			type = parse_abs_declarator(dummy_type);			/*because we don't know base whether declarator has postfix, so we don't know its base type and we use dummy*/
			SKIP(TK_RPAREN);

			tmp_type = parse_decl_postfix(base_type);
			*dummy_type = *tmp_type;
			bcc_free(tmp_type);
			return type;
		}
		else {
			BACK_TO_SAVED_COORDINATE;
		}
	}
	type = parse_decl_postfix(base_type);
}

type_t *parse_abs_declarator(type_t *base_type)
{
	type_t *type;
	type = parse_pointer(base_type);
	if (G_TK_KIND == TK_LPAREN || G_TK_KIND == TK_LBRACKET) {
		type = parse_direct_abs_declarator(type);
	}
	return type;
}

init_ast_node_t *create_init_ast_node(expr_t *expr, int offset)
{
	init_ast_node_t *init_node;

	init_node = (init_ast_node_t *)bcc_malloc(sizeof(init_ast_node_t));
	init_node->kind = AST_INIT;
	init_node->expr = expr;
	init_node->offset = offset;

	return init_node;
}

void parse_struct_initializer_list(vector_t *init_node_list, tag_type_t *type, int offset)
{
	field_t *field;
	type_t *field_type;
	field = type->head;
	while (field) {
		field_type = field->type;
		parse_initializer(init_node_list, field_type, offset);
		offset += field_type->size;
	}
}

void parse_init_array_string(vector_t *init_node_list, type_t *type, int offset)
{
	init_ast_node_t *init_node;
	expr_t *expr;
	int str_len, i;

	str_len = strlen(TK_VALUE_PTR(G_TK_VALUE)) + 1;
	if (type->size == -1) {
		type->size = str_len;
	}
	for (i = 0; i <= str_len; i++) {
		if (i <= type->size) {
			expr = create_expr_node(AST_CONST);
			expr->value = TK_VALUE_INT(G_TK_VALUE);
			expr->type = g_ty_char;
			create_init_ast_node(expr, offset + i);
			insert_vector(init_node_list, init_node);
		}
		else {
			WARN("excess elements in scalar initializer");
			break;
		}
	}
}

void parse_array_initializer_list(vector_t *init_node_list, type_t *type, int offset)
{
	type_t *base_type;
	int i;

	base_type = type->base_type;
	parse_initializer(init_node_list, base_type, offset);
	i = 0;
	while (G_TK_KIND == TK_COMMA) {
		NEXT_TOKEN;
		i++;
		parse_initializer(init_node_list, base_type, offset + base_type->size * i);
	}
	if (type->size == -1) {
		type->size = i;
	}

}

void parse_initializer(vector_t *init_node_list, type_t *type, int offset)
{
	init_ast_node_t *init_node;
	expr_t *expr;
	int i, str_len;

	if (G_TK_KIND == TK_LBRACE) {
		NEXT_TOKEN;
		if (type->kind == TYPE_STRUCT) {
			parse_struct_initializer_list(init_node_list, type, offset);
		} else if (type->kind = TYPE_ARRAY){
			parse_array_initializer_list(init_node_list, type, offset);
		}
		if (G_TK_KIND == TK_COMMA) {
			NEXT_TOKEN;
		}
		SKIP(TK_RBRACE);
	} else if (G_TK_KIND == TK_STRING && type->kind == TYPE_ARRAY){
		parse_init_array_string(init_node_list, type, offset);
	} else {
		expr = parse_assign_expr();
		init_node = create_init_ast_node(expr, offset);
		insert_vector(init_node_list, init_node);
	}
}

decl_node_t *create_decl_node(char *name, init_ast_node_t *init_node)
{
	decl_node_t *node;

	node = (decl_node_t *)bcc_malloc(sizeof(decl_node_t));

	node->kind = AST_DECL;
	node->name = name;
	node->init_node = init_node;
}

void *parse_init_declarator(vector_t *list, type_t *spec_type)
{
	type_t *type;
	expr_t *expr;
	vector_t *decl_init_ast;
	BOOL is_typedef;
	decl_node_t *decl_node;
	char *name;
	
	type = parse_declarator(spec_type, &name);

	is_typedef = (spec_type->store_cls == TK_TYPEDEF) ? TRUE : FALSE;
	insert_to_sym_table(name, type, is_typedef, spec_type->qual | WITH_CONST, 0);

	decl_init_ast = NULL;
	if (G_TK_KIND == TK_ASSIGN) {
		NEXT_TOKEN;
		decl_init_ast = create_vector(10);
		parse_initializer(decl_init_ast, type, 0);
	}

	insert_vector(list, create_decl_node(name, decl_init_ast));
}

ast_node_t *parse_declaration(vector_t *list)
{
	type_t *base_type, *type;


	base_type = parse_decl_spec(TRUE);

	parse_init_declarator(list, base_type);
	while (G_TK_KIND == TK_COMMA) {
		parse_init_declarator(list, base_type);
	}
	SKIP(TK_SEMICOLON);
}

BOOL is_decl_spec(token_t *token) {
	if (token->tk_kind== TK_TYPEDEF
		|| token->tk_kind == TK_EXTERN
		|| token->tk_kind == TK_STATIC
		|| token->tk_kind == TK_AUTO
		|| token->tk_kind == TK_REGISTER
		|| token->tk_kind == TK_VOID
		|| token->tk_kind == TK_CHAR
		|| token->tk_kind == TK_SHORT
		|| token->tk_kind == TK_INT
		|| token->tk_kind == TK_LONG
		|| token->tk_kind == TK_FLOAT
		|| token->tk_kind == TK_DOUBLE
		|| token->tk_kind == TK_SIGNED
		|| token->tk_kind == TK_UNSIGNED
		|| token->tk_kind == TK_STRUCT
		|| token->tk_kind == TK_ENUM
		|| token->tk_kind == TK_CONST
		|| token->tk_kind == TK_VOLATILE) {
		return TRUE;
	}
	if (token->tk_kind == TK_IDENTIFIER) {
		if (is_typedef_name(token->token_value.ptr)) {
			return TRUE;
		}
	}
	return FALSE;
}

vector_t *parse_declaration_list()
{
	vector_t *list;

	list = create_vector(20);
	parse_declaration(list);
	
	while (is_decl_spec(&g_current_token)) {
		parse_declaration(list);
	}
	return list;
}

statement_t *parse_labeled_statement()
{
	statement_t *labeled_state;
	int kind;

	labeled_state = (statement_t *)bcc_malloc(sizeof(statement_t));

	labeled_state->kind = AST_LABEL_IDENT;
	labeled_state->ident = create_token_node();
	SKIP(TK_COLON);
	labeled_state->statement1 = parse_statement();

	return labeled_state;
}

statement_t *parse_default_statement()
{
	statement_t *default_statment;

	default_statment = (statement_t *)bcc_malloc(sizeof(statement_t));
	default_statment->kind = AST_LABEL_DEFAULT;
	SKIP(TK_DEFAULT);
	default_statment->statement1 = parse_statement();
	return default_statment;
}

statement_t *parse_case_statment()
{
	statement_t *case_statment;

	case_statment = (statement_t *)bcc_malloc(sizeof(statement_t));
	case_statment->kind = AST_LABEL_CASE;
	case_statment->expr1 = parse_const_expr();
	SKIP(TK_COLON);
	case_statment->statement1 = parse_statement();
	
	return case_statment;
}

statement_t *parse_expr_statement()
{
	statement_t *expr_state;

	expr_state = (statement_t *)bcc_malloc(sizeof(statement_t));
	
	expr_state->kind = AST_EXPR_STATEMENT;
	expr_state->expr1 = parse_comma_expr();
	SKIP(TK_SEMICOLON);

	return expr_state;
}

statement_t *parse_if_statement()
{
	statement_t *if_statement;

	if_statement = (statement_t *)bcc_malloc(sizeof(statement_t));
	if_statement->kind = AST_IF;

	SKIP(TK_IF);
	SKIP(TK_LPAREN);
	if_statement->expr1 = parse_comma_expr();
	SKIP(TK_RPAREN);
	if_statement->statement1 = parse_statement();
	if (G_TK_KIND == TK_ELSE) {
		SKIP(TK_ELSE);
		if_statement->statement2 = parse_statement();
	}
	return if_statement;
}

statement_t *parse_switch_statement()
{
	statement_t *switch_statement;

	switch_statement = (statement_t *)bcc_malloc(sizeof(statement_t));
	switch_statement->kind = AST_SWITCH;
	SKIP(TK_SWITCH);
	SKIP(TK_LPAREN);
	switch_statement->expr1 = parse_comma_expr();
	SKIP(TK_RPAREN);
	switch_statement->statement1 = parse_statement();
}

statement_t *parse_while_statement()
{
	statement_t *while_statment;

	while_statment = (statement_t *)bcc_malloc(sizeof(statement_t));
	while_statment->kind = AST_WHILE;
	SKIP(TK_WHILE);
	SKIP(TK_LPAREN);
	while_statment->expr1 = parse_comma_expr();
	SKIP(TK_RPAREN);
	while_statment->statement1 = parse_statement();
}

statement_t *parse_do_statement()
{
	statement_t *do_statement;
	
	do_statement = (statement_t *)bcc_malloc(sizeof(statement_t));
	do_statement->kind = AST_DO;
	SKIP(TK_DO);
	do_statement->statement1 = parse_statement();
	SKIP(TK_LPAREN);
	do_statement->expr1 = parse_comma_expr();
	SKIP(TK_LPAREN);
	SKIP(TK_SEMICOLON);
}

statement_t *parse_for_statement()
{
	statement_t *for_statement;

	for_statement = (statement_t *)bcc_malloc(sizeof(statement_t));
	for_statement->kind = AST_FOR;

	SKIP(TK_LPAREN);
	for_statement->statement1 = parse_expr_statement();
	for_statement->statement2 = parse_expr_statement();
	if (G_TK_KIND != TK_RPAREN) {
		for_statement->expr1 = parse_comma_expr();
	}
	SKIP(TK_RPAREN);
	for_statement->statement3 = parse_statement();
}

statement_t *parse_jump_statement()
{
	statement_t *goto_statement;

	goto_statement = (statement_t *)bcc_malloc(sizeof(statement_t));
	
	switch (G_TK_KIND)
	{
	case TK_GOTO:
		goto_statement->kind = AST_GOTO;
		SKIP(TK_GOTO);
		goto_statement->ident = create_token_node();
		SKIP(TK_IDENTIFIER);
	case TK_CONTIUE:
		goto_statement->kind = AST_CONTINUE;
		SKIP(TK_CONTIUE);
	case TK_BREAK:
		goto_statement->kind = AST_BREAK;
		SKIP(TK_BREAK);
	case TK_RETURN:
		goto_statement->kind = AST_RETURN;
		if (G_TK_KIND != TK_SEMICOLON) {
			goto_statement->expr1 = parse_comma_expr();
		}
	default:
		break;
	}
	SKIP(TK_SEMICOLON);

	return goto_statement;
}

statement_t *parse_statement()
{
	statement_t* statement;

	switch (G_TK_KIND) {
	case TK_IDENTIFIER:
		if (peek()->tk_kind == TK_COLON) {
			return parse_labeled_statement();
		}
	case TK_CASE:		return parse_case_statment();
	case TK_DEFAULT:	return parse_default_statement();
	case TK_LBRACE:		return parse_compound_statement();
	case TK_IF:			return parse_if_statement();
	case TK_SWITCH:		return parse_switch_statement();
	case TK_WHILE:		return parse_while_statement();
	case TK_DO:			return parse_do_statement();
	case TK_FOR:		return parse_for_statement();
	case TK_GOTO:	
	case TK_CONTIUE:
	case TK_BREAK:
	case TK_RETURN:		return parse_jump_statement();
	default:			return parse_expr_statement();
	}
}

vector_t *parse_statement_list()
{
	vector_t *list;
	statement_t *statement_list;

	list = (vector_t *)bcc_malloc(sizeof(vector_t));

	insert_vector(list, parse_statement());

	while (G_TK_KIND != TK_LBRACE) {
		insert_vector(list, parse_statement());
	}
	return list;
}

statement_t *parse_compound_statement()
{
	statement_t *comp_state;

	if (G_TK_KIND != TK_LBRACE) {
		ERROR("expect '{'");
	}
	SKIP(TK_LBRACE);
	comp_state = (statement_t *)bcc_malloc(sizeof(statement_t));
	if (is_decl_spec(&g_current_token)) {
		comp_state->vec1 = parse_declaration_list();
	}
	comp_state->vec2 = parse_statement_list();
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

	if (is_decl_spec(&g_current_token)) {
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
	}
	else if (G_TK_KIND == TK_LBRACE || is_decl_spec(&g_current_token)) {
		func_def = (declaration_t *)bcc_malloc(sizeof(declaration_t));
		func_def->decl_spec = decl_spec;
		func_def->decl = declarator;
		if (is_decl_spec(&g_current_token)) {
			func_def->decl_list = parse_declaration_list();
			EXPECT(TK_LBRACE);
		}
		func_def->comp_state_ptr = parse_compound_statement();

		external_decl->func = func_def;
		external_decl->kind = FUNC_DEFINATION;
	}
	else {
		ERROR("unexpected token");
	}
	return external_decl;
}