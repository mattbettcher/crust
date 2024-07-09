#include "ast.h"

Arena ast_arena;

size_t ast_memory_usage;

static void *
ast_alloc(size_t size) {
    assert(size != 0);
    void *ptr = arena_alloc(&ast_arena, size);
    memset(ptr, 0, size);
    ast_memory_usage += size;
    return ptr;
}

static void *
ast_dup(const void *src, size_t size) {
    if (size == 0) {
        return NULL;
    }
    void *ptr = arena_alloc(&ast_arena, size);
    memcpy(ptr, src, size);
    return ptr;
}

#define AST_DUP(x) ast_dup(x, num_##x * sizeof(*x))

static Expr *
new_expr(ExprKind kind, SrcPos pos) {
    Expr *e = ast_alloc(sizeof(Expr));
    e->kind = kind;
    e->pos = pos;
    return e;
}

static Expr *
new_expr_paren(SrcPos pos, Expr *expr) {
    Expr *e = new_expr(EXPR_PAREN, pos);
    e->paren.expr = expr;
    return e;
}

static Expr *
new_expr_binary(SrcPos pos, TokenKind op, Expr *left, Expr *right) {
    Expr *e = new_expr(EXPR_BINARY, pos);
    e->binary.op = op;
    e->binary.left = left;
    e->binary.right = right;
    return e;
}

static Expr *
new_expr_int(SrcPos pos, unsigned long long val, TokenMod mod, TokenSuffix suffix) {
    Expr *e = new_expr(EXPR_INT, pos);
    e->int_lit.val = val;
    e->int_lit.mod = mod;
    e->int_lit.suffix = suffix;
    return e;
}

static Expr *
new_expr_float(SrcPos pos, const char *start, const char *end, double val, TokenSuffix suffix) {
    Expr *e = new_expr(EXPR_FLOAT, pos);
    e->float_lit.start = start;
    e->float_lit.end = end;
    e->float_lit.val = val;
    e->float_lit.suffix = suffix;
    return e;
}

static Expr *
new_expr_str(SrcPos pos, const char *val, TokenMod mod) {
    Expr *e = new_expr(EXPR_STR, pos);
    e->str_lit.val = val;
    e->str_lit.mod = mod;
    return e;
}

static Expr *
new_expr_name(SrcPos pos, const char *name) {
    Expr *e = new_expr(EXPR_NAME, pos);
    e->name = name;
    return e;
}

static Expr *
new_expr_modify(SrcPos pos, TokenKind op, bool post, Expr *expr) {
    Expr *e = new_expr(EXPR_MODIFY, pos);
    e->modify.op = op;
    e->modify.post = post;
    e->modify.expr = expr;
    return e;
}

static Expr *
new_expr_unary(SrcPos pos, TokenKind op, Expr *expr) {
    Expr *e = new_expr(EXPR_UNARY, pos);
    e->unary.op = op;
    e->unary.expr = expr;
    return e;
}

static Expr *
new_expr_tuple(SrcPos pos, Expr **args, size_t num_args) {
    Expr *e = new_expr(EXPR_TUPLE, pos);
    e->tuple.args = AST_DUP(args);
    e->tuple.num_args = num_args;
    return e;
}

static Expr *
new_expr_call(SrcPos pos, Expr *expr, Expr **args, size_t num_args) {
    Expr *e = new_expr(EXPR_CALL, pos);
    e->call.expr = expr;
    e->call.args = AST_DUP(args);
    e->call.num_args = num_args;
    return e;
}

static Expr *
new_expr_index(SrcPos pos, Expr *expr, Expr *index) {
    Expr *e = new_expr(EXPR_INDEX, pos);
    e->index.expr = expr;
    e->index.index = index;
    return e;
}

static Expr *
new_expr_field(SrcPos pos, Expr *expr, const char *name) {
    Expr *e = new_expr(EXPR_FIELD, pos);
    e->field.expr = expr;
    e->field.name = name;
    return e;
}

static Decl *
new_decl(DeclKind kind, SrcPos pos, const char *name) {
    Decl *d = ast_alloc(sizeof(Decl));
    d->kind = kind;
    d->pos = pos;
    d->name = name;
    return d;
}

static Decl *
new_decl_func(SrcPos pos, const char *name, FuncParam *params, size_t num_params, Typespec *ret_type) {
    Decl *d = new_decl(DECL_FUNC, pos, name);
    d->fn.params = AST_DUP(params);
    d->fn.num_params = num_params;
    d->fn.ret_type = ret_type;
    //d->fn.has_varargs = has_varargs;
    //d->fn.varargs_type = varargs_type;
    //d->fn.block = block;
    return d;
}

static Typespec *
new_typespec(TypespecKind kind, SrcPos pos) {
    Typespec *t = ast_alloc(sizeof(Typespec));
    t->kind = kind;
    t->pos = pos;
    return t;
}

static Typespec *
new_typespec_name(SrcPos pos, const char **names, size_t num_names) {
    Typespec *t = new_typespec(TYPESPEC_NAME, pos);
    t->names = AST_DUP(names);
    t->num_names = num_names;
    return t;
}

static Typespec *
new_typespec_tuple(SrcPos pos, Typespec **fields, size_t num_fields) {
    Typespec *t = new_typespec(TYPESPEC_TUPLE, pos);
    t->tuple.fields = AST_DUP(fields);
    t->tuple.num_fields = num_fields;
    return t;
}