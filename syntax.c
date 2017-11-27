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

	if (G_TK_KIND == TK_IDENTIFIER) {
		create_expr_node(AST_VAR);
		expr->child_1 = create_token_node();
		if (!in_symbol_table(g_sym_tb, G_TK_VALUE.ptr)) {
			ERROR("Undeclare varible %s", G_TK_VALUE.ptr);
		}
		expr->type = get_symbol_type(g_sym_tb, G_TK_VALUE.ptr);
		NEXT_TOKEN;
		return expr;
	}
	if (G_TK_KIND == TK_LPAREN) {
		SKIP(TK_LPAREN);
		return parse_expr();
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
	
	expr->type = type_conv(expr->type);
	if (expr->type->kind != TYPE_POINTER) {
		ERROR("subscripted value is neither array nor pointer");
	}
	SKIP(TK_LBRACKET);
	subscript_expr = parse_expr();
	SKIP(TK_RBRACKET);

	array_expr = create_expr_node(AST_ARRAY);
	array_expr->child_1 = expr;
	array_expr->child_2 = subscript_expr;
	array_expr->type = expr->type->base_type;
	return array_expr;
}

expr_t *parse_func_expr(expr_t *expr)
{
	expr_t *param_expr;
	expr_t *func_expr;

	expr->type = type_conv(expr->type);
	if (expr->type->kind != TYPE_POINTER || expr->type->base_type->kind != TYPE_FUNCTION) {
		ERROR("expect function identifier");
	}
	SKIP(TK_LPAREN);
	param_expr = parse_argu_expr_list();
	SKIP(TK_RPAREN);

	func_expr = create_expr_node(AST_FUNC);
	func_expr->child_1 = expr;
	func_expr->child_2 = parse_argu_expr_list();
	func_expr->type = ((function_type_t *)expr->type)->ret;
	return func_expr;
}

expr_t *parse_struct_field(expr_t *expr)
{
	type_t *struct_ty, *field_ty;
	expr_t *struct_field_expr;
	token_t *field_tk;
	if (G_TK_KIND == TK_POINTER) {
		if (expr->type->kind != TYPE_POINTER
			|| (expr->type->base_type->kind != TYPE_STRUCT
			&& expr->type->base_type->kind != TYPE_UNION)) {
			ERROR("expect pointer to struct or union type");
		}
		struct_ty = expr->type->base_type;
		struct_field_expr = create_expr_node(AST_POINTER_TO);
	} else if (G_TK_KIND == TK_DOT) {
		if (expr->type->kind != TYPE_STRUCT
			&& expr->type->kind != TYPE_UNION) {
			ERROR("expect to struct or union type");
		}
		struct_ty = expr->type;
		struct_field_expr = create_expr_node(AST_CONTAIN);
	}
	NEXT_TOKEN;
	EXPECT(TK_IDENTIFIER);
	
	field_tk = create_token_node();
	field_ty = get_tag_member_type(struct_ty, field_tk->token_value.ptr);
	if (field_ty == NULL) {
		ERROR("struct have no expect member");
	}
	struct_field_expr->child_1 = expr;
	struct_field_expr->child_2 = field_tk;
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
	inc_expr = create_expr_node(kind);
	inc_expr->child_1 = expr;
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
	
	postfix_expr = parse_postfix(parse_primary_expr());

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
	}
	type = parse_unary_expr()->type;
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
		unary_expr = parse_self_inc_dec_expr(parse_unary_expr(), AST_PRFIX_INC);
		break;
	case TK_DEC:
		SKIP(TK_DEC);
		unary_expr = parse_self_inc_dec_expr(parse_unary_expr(), AST_PRFIX_INC);
		break;
	case TK_BITAND:
		SKIP(TK_BITAND);
		cast_expr = parse_cast_expr();
		if (!is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '&' operand");
		}
		unary_expr = create_expr_node(AST_ADDR);
		unary_expr->child_1 = cast_expr;
		unary_expr->type = derive_pointer_type(cast_expr->type, 0);
		break;
	case TK_MULTIPLY:
		SKIP(TK_MULTIPLY);
		cast_expr = parse_cast_expr();
		if (cast_expr->type->kind != TYPE_POINTER) {
			ERROR("expect pointer type");
		}
		if (cast_expr->type->base_type->kind == TYPE_FUNCTION) {
			return cast_expr;
		}
		unary_expr = create_expr_node(AST_DREF);
		unary_expr->child_1 = cast_expr;
		unary_expr->type = cast_expr->type->base_type;
		break;
	case TK_ADD:
		SKIP(TK_ADD);
		cast_expr = parse_cast_expr();
		if (!is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '+' operand");
		}
		unary_expr = create_expr_node(AST_UNARY_PLUS);
		unary_expr->child_1 = cast_expr;
		unary_expr->type = cast_expr->type;
		break;
	case TK_SUB:
		SKIP(TK_SUB);
		cast_expr = parse_cast_expr();
		if (!is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '-' operand");
		}
		unary_expr = create_expr_node(AST_UNARY_MINUS);
		unary_expr->child_1 = cast_expr;
		unary_expr->type = cast_expr->type;
		break;
	case TK_BITREVERT:
		SKIP(TK_BITREVERT);
		cast_expr = parse_cast_expr();
		if (!is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '~' operand");
		}
		unary_expr = create_expr_node(AST_BITREVERT);
		unary_expr->child_1 = cast_expr;
		unary_expr->type = cast_expr->type;
		break;
	case TK_NOT:
		SKIP(TK_NOT);
		cast_expr = parse_cast_expr();
		if (!is_lvaue(cast_expr)) {
			ERROR("lvalue required as unary '!' operand");
		}
		unary_expr = create_expr_node(AST_NOT);
		unary_expr->child_1 = cast_expr;
		unary_expr->type = cast_expr->type;
		break;
	case TK_SIZEOF:
		SKIP(TK_SIZEOF);
		unary_expr = create_expr_node(AST_SIZEOF);
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
		cast_expr->child_2 = parse_cast_expr();
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

BOOL is_struct_type(type_t *type)
{
	if (type->kind == TYPE_STRUCT) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_compatible_struct(tag_type_t *type1, tag_type_t *type2)
{
	return type1 == type2;
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

BOOL is_valid_both_pointer_bin_op(int ast_kind)
{
	if (ast_kind == AST_SUB || ast_kind == AST_LESS || ast_kind == AST_GREAT
		|| ast_kind == AST_EQUAL || ast_kind == AST_NEQUAL
		|| ast_kind == AST_GREAT_EQUAL || ast_kind == AST_LESS_EQUAL) {
		return TRUE;
	}
	return FALSE;
}

BOOL is_valid_l_pointer_bin_op(int ast_kind)
{
	if (ast_kind == AST_ADD || ast_kind == AST_SUB || ast_kind == AST_LEFT || ast_kind == AST_RIGHT
		|| ast_kind == AST_BIT_AND || AST_BIT_AND == AST_BIT_OR) {
		return TRUE;
	}
	return FALSE;
}

void binary_operand_check(int ast_kind, expr_t *child_1, expr_t *child_2)
{
	type_t *type1, *type2;
	type1 = child_1->type; type2 = child_2->type;
	if (is_pointer_type(type1) && is_pointer_type(type2)) {
		if (!is_valid_both_pointer_bin_op(ast_kind)) {
			ERROR("invalid pointer arith");
		}
		return;
	}
	if (is_pointer_type(type1)) {
		if (!is_valid_l_pointer_bin_op(ast_kind) || !is_arith_type(type2)) {
			ERROR("invalid pointer arith");
		}
		return;
	}
	if (is_pointer_type(type2)) {
		ERROR("invalid pointer arith");
		return;
	}
	if (!is_arith_type(type1) || !is_arith_type(type2)) {
		ERROR("invalid arith type");
	}
	
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

expr_t *create_binary_expr(int ast_kind, expr_t *child_1, expr_t *child_2)
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

	multi_expr = parse_cast_expr();
	while (G_TK_KIND == TK_MULTIPLY || G_TK_KIND == TK_DIVIDE || G_TK_KIND == TK_MOD) {
		child1 = multi_expr;
		child2 = parse_cast_expr();

		if (G_TK_KIND == TK_MOD) {
			if (!is_integer_type(child1) || !is_integer_type(child2)) {
				ERROR("invalid operand");
			}
		} else {
			if (!is_arith_type(child1) || !is_arith_type(child2)) {
				ERROR("invalid operand");
			}
		}

		multi_expr = create_binary_expr(G_TK_KIND, child1, child2);
		multi_expr->type = usual_arith_conv(child1, child2);
	}
	return multi_expr;
}

expr_t *parse_addit_expr()
{
	expr_t *addit_expr, *child_1, *child_2;

	addit_expr = parse_multi_expr();
	while (G_TK_KIND == TK_ADD || G_TK_KIND == TK_SUB) {
		child_1 = addit_expr;
		child_2 = parse_multi_expr();
		if (is_arith_type(child_1) && is_arith_type(child_2)) {
			addit_expr = create_binary_expr(G_TK_KIND, child_1, child_2);
			addit_expr->type = usual_arith_conv(child_1, child_2);
			continue;
		}
		if (is_pointer_type(child_1) && is_pointer_type(child_2)) {
			if (G_TK_KIND != TK_SUB) {
				ERROR("invalid operand");
			}
			if (!is_compatible_ptr(child_1, child_2)) {
				WARN("invalid operand");
			}
			addit_expr = create_binary_expr(G_TK_KIND, child_1, child_2);
			addit_expr->type = g_ty_int;
			continue;
		}
		if (is_pointer_type(child_1) && is_integer_type(child_2)) {
			addit_expr = create_binary_expr(G_TK_KIND, child_1, child_2);
			addit_expr->type = child_1->type;
			continue;
		}
		if (is_integer_type(child_1) && is_pointer_type(child_2)) {
			if (G_TK_KIND == TK_SUB) {
				ERROR("invalid operand");
			}
			addit_expr = create_binary_expr(G_TK_KIND, child_1, child_2);
			addit_expr->type = child_2->type;
			continue;
		}
		ERROR("invalid operand");
	}
	return addit_expr;
}

expr_t *parse_shift_epxr()
{
	expr_t *shift_expr, *child1, *child2;

	shift_expr = parse_addit_expr();
	while (G_TK_KIND == TK_LSHIFT || G_TK_KIND == TK_RSHIFT) {
		child1 = shift_expr;
		child2 = parse_addit_expr();
		if (!is_integer_type(child1) || !is_integer_type(child2)) {
			ERROR("invalid operands");
		}
		shift_expr = create_binary_expr(G_TK_KIND, child1, child2);
		shift_expr->type = type_conv(child1->type);
	}
	return shift_expr;
}

expr_t *parse_realtional_expr()
{
	expr_t *rela_expr, *child1, *child2;

	rela_expr = parse_shift_epxr();
	while (G_TK_KIND == TK_LESS || G_TK_KIND == TK_LESS_EQUAL
		|| G_TK_KIND == TK_GREAT || G_TK_KIND == TK_GREAT_EQUAL) {
		child1 = rela_expr;
		child2 = parse_shift_epxr();
		if (!(is_arith_type(child1) && is_arith_type(child2))
			&& !(is_pointer_type(child1) && is_pointer_type(child2))) {
			ERROR("invalid operands");
		}
		if (is_pointer_type(child1) && is_pointer_type(child2)
			&& !is_compatible_ptr(child1, child2)) {
			WARN("incompatible pointer");
		}

		rela_expr = create_binary_expr(G_TK_KIND, child1, child2);
		rela_expr->type = g_ty_int;
	}
	return rela_expr;
}

expr_t *parse_equality_expr()
{
	expr_t *equality_expr, *child1, *child2;

	equality_expr = parse_realtional_expr();
	while (G_TK_KIND == TK_EQUAL || G_TK_KIND == TK_NEQUAL) {
		child1 = equality_expr;
		child2 = parse_realtional_expr();
		if (is_arith_type(child1) && is_arith_type(child2)
			|| is_pointer_type(child1) && is_pointer_type(child2)
			|| is_pointer_type(child1) && is_null_pointer(child2)
			|| is_pointer_type(child2) && is_pointer_type(child2)) {
			equality_expr = create_binary_expr(G_TK_KIND, child1, child2);
			equality_expr->type = g_ty_int;
		} else {
			ERROR("invalid operand");
		}
	}
	return equality_expr;
}

expr_t *parse_bit_and_expr()
{
	expr_t *bit_and_expr, *child1, *child2;

	bit_and_expr = parse_equality_expr();
	while (G_TK_KIND == TK_BITAND) {
		child1 = bit_and_expr;
		child2 = parse_equality_expr();
		if (!is_integer_type(child1) || !is_integer_type(child2)) {
			ERROR("invalid operand");
		}
		bit_and_expr = create_binary_expr(G_TK_KIND, child1, child2);
		bit_and_expr->type = type_conv(child1->type);
	}
	return bit_and_expr;
}

expr_t *parse_exclusive_or_expr()
{
	expr_t *excl_expr, *child1, *child2;
	
	excl_expr = parse_bit_and_expr();
	while (G_TK_KIND == TK_BITXOR) {
		child1 = excl_expr;
		child2 = parse_bit_and_expr();
		if (!is_integer_type(child1) || !is_integer_type(child2)) {
			ERROR("invalid operand");
		}
		excl_expr = create_binary_expr(G_TK_KIND, child1, child2);
		excl_expr->type = type_conv(child1->type);
	}
	return excl_expr;
}

expr_t *parse_inclusive_or_expr()
{
	expr_t *inclu_or_expr, *child1, *child2;

	inclu_or_expr = parse_bit_and_expr();
	while (G_TK_KIND == TK_BITOR) {
		child1 = inclu_or_expr;
		child2 = parse_bit_and_expr();
		if (!is_integer_type(child1) || !is_integer_type(child2)) {
			ERROR("invalid operand");
		}
		inclu_or_expr = create_binary_expr(G_TK_KIND, child1, child2);
		inclu_or_expr->type = type_conv(child1->type);
	}
	return inclu_or_expr;
}

expr_t *parse_logic_and_expr()
{
	expr_t *logic_and, *child1, *child2;
	
	logic_and = parse_inclusive_or_expr();
	while (G_TK_KIND == TK_AND) {
		child1 = logic_and;
		child2 = parse_inclusive_or_expr();
		if (!is_scalar_type(child1) || !is_scalar_type(child2)) {
			ERROR("invalid operand");
		}
		logic_and = create_binary_expr(G_TK_KIND, child1, child2);
		logic_and->type = g_ty_int;
	}
	return logic_and;
}

expr_t *parse_logic_or_expr()
{
	expr_t *logic_or, *child1, *child2;

	logic_or = parse_inclusive_or_expr();
	while (G_TK_KIND == TK_OR) {
		child1 = logic_or;
		child2 = parse_inclusive_or_expr();
		if (!is_scalar_type(child1) || !is_scalar_type(child2)) {
			ERROR("invalid operand");
		}
		logic_or = create_binary_expr(G_TK_KIND, child1, child2);
		logic_or->type = g_ty_int;
	}
	return logic_or;
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
	declaration_t *list, *list_iter;
	
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
	declaration_t *param_decl;

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

void parse_array_initializer_lsit(vector_t *init_node_list, type_t *type, int offset)
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

ast_node_t *parse_initializer(vector_t *init_node_list, type_t *type, int offset)
{
	init_ast_node_t *init_node;
	expr_t *expr;
	int i, str_len;

	if (G_TK_KIND == TK_LBRACE) {
		NEXT_TOKEN;
		if (type->kind == TYPE_STRUCT) {
			parse_struct_initializer_list(init_node_list, type, offset);
		} else if (type->kind = TYPE_ARRAY){
			parse_initializer_list(init_node_list, type, offset);
		}
		if (G_TK_KIND == TK_COMMA) {
			NEXT_TOKEN;
		}
		SKIP(TK_RBRACE);
	} else if (G_TK_KIND == TK_STRING && type->kind == TYPE_ARRAY){
		parse_init_array_string(init_node_list, type, offset);
	} else {
		expr = parse_assign_expr();
		create_init_ast_node(expr, offset);
		insert_vector(init_node_list, init_node);
	}
}

vector_t *parse_init_declarator(type_t *spec_type)
{
	type_t *type;
	expr_t *expr;
	vector_t *decl_init_ast;
	BOOL is_typedef;
	char *name;
	
	type = parse_declarator(spec_type, &name);

	is_typedef = (spec_type->store_cls == TK_TYPEDEF) ? TRUE : FALSE;
	insert_to_sym_table(name, type, is_typedef);

	if (G_TK_KIND == TK_ASSIGN) {
		NEXT_TOKEN;
		decl_init_ast = create_vector(10);
		expr = parse_initializer(decl_init_ast, type, 0);
	}
	return decl_init_ast;
}

ast_node_t *parse_declaration()
{
	type_t *base_type, *type;

	base_type = parse_decl_spec(TRUE);

	parse_init_declarator(base_type);
	while (G_TK_KIND == TK_COMMA) {
		parse_init_declarator(base_type);
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

ast_node_t *parse_declaration_list()
{
	declaration_t *list;
	list = parse_declaration();
	
	if (is_decl_spec(&g_current_token)) {
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
		ERROR("expect identifier, 'default', 'case'");
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
	if (is_decl_spec(&g_current_token)) {
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
	} else if(G_TK_KIND == TK_LBRACE || is_decl_spec(&g_current_token)) {
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
	} else {
		ERROR("unexpected token");
	}
	return external_decl;
}