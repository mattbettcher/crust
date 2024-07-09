#pragma once

#include "stdafx.h"
#include "common.h"
#include "lex.h"
#include "ast.h"

static Typespec *parse_type_tuple(Typespec *type);
static Typespec *parse_type_base(void);
static Typespec *parse_type(void);

static const char *parse_name(void);
static FuncParam parse_decl_func_param(void);
static Decl *parse_decl_fn(SrcPos pos);

static Expr *parse_expr_operand(void);
static Expr *parse_expr_base(void);
static bool is_unary_op(void);
static Expr *parse_expr_unary(void);
static bool is_mul_op(void);
static Expr *parse_expr_mul(void);
static bool is_add_op(void);
static Expr *parse_expr_add(void);
static bool is_cmp_op(void);
static Expr *parse_expr_cmp(void);
static Expr *parse_expr_and(void);
static Expr *parse_expr_or(void);
static Expr *parse_expr(void);
static Expr *parse_paren_expr(void);
