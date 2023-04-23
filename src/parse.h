#pragma once

#include "stdafx.h"
#include "common.h"
#include "lex.h"
#include "ast.h"

Typespec *parse_type_tuple(Typespec *type);
Typespec *parse_type_base(void);
Typespec *parse_type(void);

const char *parse_name(void);
FuncParam parse_decl_func_param(void);
Decl *parse_decl_fn(SrcPos pos);

Expr *parse_expr_operand(void);
Expr *parse_expr_base(void);
bool is_unary_op(void);
Expr *parse_expr_unary(void);
bool is_mul_op(void);
Expr *parse_expr_mul(void);
bool is_add_op(void);
Expr *parse_expr_add(void);
bool is_cmp_op(void);
Expr *parse_expr_cmp(void);
Expr *parse_expr_and(void);
Expr *parse_expr_or(void);
Expr *parse_expr(void);
Expr *parse_paren_expr(void);
