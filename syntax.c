#include "bcc.h"


hashtable_t g_external_symbol_table;

#define NEXT_TOKEN get_next_token();

#define G_TK_KIND	g_current_token.tk_kind
#define G_TK_VALUE	g_current_token.token_value
#define G_TK_LINE	g_current_token.line;


ast_node_t *create_node(int size)
{
	ast_node_t *new_node;
	new_node = (ast_node_t*)bcc_malloc(sizeof(ast_node_t));
	new_node->kind = kind;
	return new_node;
}

ast_node_t *parse_spec_qual_list()
{
	spec_qual_list_t *spec_list;
	type_spec_t *type_spec;
	type_qual_t *type_qual;
	BOOL invalid_decl_spec;

	spec_list = (spec_qual_list_t *)bcc_malloc(sizeof(spec_qual_list_t));

	type_spec = spec_list->type_sepc;
	type_qual = spec_list->type_qual;
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
				bcc_strncpy(&type_spec->value->tk_val, &g_current_token, sizeof(token_t));
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
			if (type_spec == NULL) {
				type_spec = (type_spec_t*)bcc_malloc(sizeof(type_spec_t));
				bcc_strncpy(&type_spec->value->tk_val, &g_current_token, sizeof(token_t));
			} else {
				error_message("repeated type specifier");
			}
			break;
		case TK_CONST:
			if (type_qual == NULL) {
				type_qual->const_tk = (ast_node_t *)bcc_malloc(sizeof(ast_node_t));
				bcc_strncpy(&type_qual->const_tk->tk_val, &g_current_token, sizeof(token_t));
			}
			break;
		case TK_VOLATILE:
			if (type_qual == NULL) {
				type_qual->volatile_tk = (ast_node_t *)bcc_malloc(sizeof(ast_node_t));
				bcc_strncpy(&type_qual->volatile_tk->tk_val, &g_current_token, sizeof(token_t));
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

}

ast_node_t *parse_constant_expr()
{

}

ast_node_t *parse_struct_declarator()
{
	declarator_t *declarator;

	declarator = create_node(DECLARATOR, sizeof(declarator_t));

	return declarator;
}

ast_node_t *parse_struct_declarator_list()
{
	struct_declarator_list_t *list, *iter_list;
	declarator_t *declarator;
	const_expr_t *const_expr;
	struct_declarator_list_t *next_declarator;

	list = iter_list = (struct_declarator_list_t*)bcc_malloc(sizeof(struct_declarator_list_t));

	while (1) {
		declarator = iter_list->declarator;
		const_expr = iter_list->const_expr;
		next_declarator = iter_list->next;
		declarator = const_expr = next_declarator = NULL;

		if (G_TK_KIND == TK_COLON) {
			NEXT_TOKEN;
			const_expr = parse_constant_expr();
		} else {
			NEXT_TOKEN;
			declarator = parse_struct_declarator();
			if (G_TK_KIND == TK_COLON) {
				NEXT_TOKEN;
				const_expr = parse_constant_expr();
			}
		}
		if (G_TK_KIND == TK_RBRACE) {
			break;
		}
		next_declarator = (struct_declarator_list_t*)bcc_malloc(sizeof(struct_declarator_list_t));
		iter_list = next_declarator;
	}
	return list;
}

ast_node_t *parse_struct_declaration_list()
{
	struct_declaration_t *struct_decl, *iter_struct_decl;
	spec_qual_list_t *spec_qual;
	struct_declarator_list_t *decl_list;
	struct_declaration_t *next_decl;

	struct_decl = iter_struct_decl = (struct_declaration_t*)bcc_malloc(sizeof(struct_declaration_t));
	NEXT_TOKEN;
	while (1) {
		spec_qual = iter_struct_decl->spec_qual_list;
		decl_list = iter_struct_decl->struct_decl_list;
		next_decl = iter_struct_decl->next;
		spec_qual = decl_list = next_decl = NULL;

		spec_qual = parse_spec_qual_list();
		decl_list = parse_struct_declarator_list();

		if (G_TK_KIND != TK_RBRACE) {
			break;
		}
		next_decl = (struct_declaration_t*)bcc_malloc(sizeof(struct_declaration_t));
		iter_struct_decl = next_decl;
	}
	return struct_decl;
}

ast_node_t *parse_struct_union()
{
	struct_or_union_spec_t *struct_union;

	struct_union = (struct_or_union_spec_t*)bcc_malloc(sizeof(struct_or_union_spec_t));
	bcc_strncpy(&struct_union->s_or_u, &g_current_token, sizeof(token_t));

	NEXT_TOKEN;
	if (G_TK_KIND == TK_LBRACE) {
		struct_union->struct_decl = parse_struct_declaration_list();
	} else if (G_TK_KIND == TK_IDENTIFIER) {
		bcc_strncpy(&struct_union->ident, &g_current_token, sizeof(token_t));
		NEXT_TOKEN;
		if (G_TK_KIND == TK_LBRACE) {
			struct_union->struct_decl = parse_struct_declaration_list();
		}
	} else {
		error_message("invalid token");
	}
}

ast_node_t *parse_enum_list()
{
	enumerator_t *enum_list, *enum_iter;
	const_expr_t *const_expr;
	enumerator_t *next_enum;
	
	enum_list = enum_iter = (enumerator_t*)bcc_malloc(sizeof(enumerator_t));

	while (G_TK_KIND != TK_RBRACE) {
		const_expr = enum_iter->const_expr;
		next_enum = enum_iter->next;
		const_expr = next_enum = NULL;

		bcc_strncpy(&enum_iter->ident->tk_val, &g_current_token, sizeof(token_t));
		NEXT_TOKEN;
		if (G_TK_KIND == TK_ASSIGN) {
			const_expr = parse_constant_expr();
		}
		if (G_TK_KIND == TK_DOT) {
			enum_iter = (enumerator_t*)bcc_malloc(sizeof(enumerator_t));
			next_enum = enum_iter;
			NEXT_TOKEN;
		}
	}
}

ast_node_t *parse_enum()
{
	enum_spec_t *enum_spec;
	enum_spec = (enum_spec_t*)bcc_malloc(sizeof(enumerator_t));

	NEXT_TOKEN;

	if (G_TK_KIND = TK_LBRACE) {
		enum_spec->enum_decl = parse_enum_list();
	} else if (G_TK_KIND == TK_IDENTIFIER) {
		bcc_strncpy(&enum_spec->ident, &g_current_token, sizeof(token_t));
		NEXT_TOKEN;
		if (G_TK_KIND = TK_LBRACE) {
			enum_spec->enum_decl = parse_enum_list();
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

	decl_spec = create_node(DECL_SPEC, sizeof(decl_spec_t));
	store_cls = decl_spec->store_cls;
	type_spec = decl_spec->type_spec;
	type_qual = decl_spec->type_qual;
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
				store_cls = create_node(STORE_CLS, sizeof(store_cls_spec_t));
				bcc_strncpy(&store_cls->token, &g_current_token, sizeof(token_t));
			} else {
				error_message("repeated storage class specifier");
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
				bcc_strncpy(&type_spec->value->tk_val, &g_current_token, sizeof(token_t));
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
			if (type_spec == NULL) {
				type_spec = (type_spec_t*)bcc_malloc(sizeof(type_spec_t));
				bcc_strncpy(&type_spec->value->tk_val, &g_current_token, sizeof(token_t));
			}
			else {
				error_message("repeated type specifier");
			}
			break;
		case TK_CONST:
			if (type_qual == NULL) {
				type_qual->const_tk = (ast_node_t *)bcc_malloc(sizeof(ast_node_t));
				bcc_strncpy(&type_qual->const_tk->tk_val, &g_current_token, sizeof(token_t));
			}
			break;
		case TK_VOLATILE:
			if (type_qual == NULL) {
				type_qual->volatile_tk = (ast_node_t *)bcc_malloc(sizeof(ast_node_t));
				bcc_strncpy(&type_qual->volatile_tk->tk_val, &g_current_token, sizeof(token_t));
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
}

ast_node_t *parse_external_decl()
{
	ast_node_t *decl_spec;

	NEXT_TOKEN;
	decl_spec = parse_decl_spec();

}