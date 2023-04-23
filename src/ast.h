#pragma once

#include "stdafx.h"
#include "common.h"
#include "lex.h"

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;
typedef struct Typespec Typespec;

typedef struct GenericParam {
    SrcPos pos;
    bool is_const;
    const char *name;
    Typespec *type;
} GenericParam;

typedef struct FuncParam {
    SrcPos pos;
    const char *name;
    Typespec *type;
} FuncParam;

typedef enum AggregateItemKind {
    AGGREGATE_ITEM_NONE,
    AGGREGATE_ITEM_FIELD,
    AGGREGATE_ITEM_SUBAGGREGATE,
} AggregateItemKind;

struct Aggregate;

typedef struct AggregateItem {
    SrcPos pos;
    AggregateItemKind kind;
    union {
        struct {
            const char **names;
            size_t num_names;
            Typespec *type;
        };
        struct Aggregate *subaggregate;
    };
} AggregateItem;

typedef enum AggregateKind {
    AGGREGATE_NONE,
    AGGREGATE_STRUCT,
    AGGREGATE_UNION,
} AggregateKind;

typedef struct Aggregate {
    SrcPos pos;
    AggregateKind kind;
    AggregateItem *items;
    size_t num_items;
} Aggregate;

typedef enum DeclKind {
    DECL_NONE,
    DECL_ENUM,
    DECL_STRUCT,
    DECL_UNION,
    DECL_VAR,
    DECL_CONST,
    DECL_TYPEDEF,
    DECL_FUNC,
    DECL_NOTE,
    DECL_IMPORT,
} DeclKind;

struct Decl {
    DeclKind kind;
    SrcPos pos;
    const char *name;
    //Notes notes;
    //bool is_incomplete;
    union {
        //Note note;
        //struct {
        //    Typespec *type;
        //    EnumItem *items;
        //    size_t num_items;
        //} enum_decl;
        Aggregate *aggregate;
        struct {
            FuncParam *params;
            size_t num_params;
            Typespec *ret_type;
            bool has_varargs;
            Typespec *varargs_type;
            //StmtList block;
        } fn;
        struct {
            Typespec *type;
        } typedef_decl;
        struct {
            Typespec *type;
            Expr *expr;
        } var;
        struct {
            Typespec *type;
            Expr *expr;
        } const_decl;
        struct {
            bool is_relative;
            const char **names;
            size_t num_names;
            bool import_all;
            //ImportItem *items;
            size_t num_items;
        } import;
    };
};

typedef struct Decls {
    Decl **decls;
    size_t num_decls;
} Decls;

typedef enum TypespecKind {
    TYPESPEC_NONE,
    TYPESPEC_NAME,
    TYPESPEC_FUNC,
    TYPESPEC_ARRAY,
    TYPESPEC_PTR,
    TYPESPEC_CONST,
    TYPESPEC_TUPLE,
} TypespecKind;

struct Typespec {
    TypespecKind kind;
    SrcPos pos;
    Typespec *base;
    union {
        struct {
            const char **names;
            size_t num_names;
        };
        struct {
            Typespec **args;
            size_t num_args;
            bool has_varargs;
            Typespec *ret;
        } fn;
        struct {
            Typespec **fields;
            size_t num_fields;
        } tuple;
        Expr *num_elems;
    };
};

typedef enum ExprKind {
    EXPR_NONE,
    EXPR_PAREN,
    EXPR_INT,
    EXPR_FLOAT,
    EXPR_STR,
    EXPR_NAME,
    EXPR_TUPLE,
    EXPR_CAST,
    EXPR_CALL,
    EXPR_INDEX,
    EXPR_FIELD,
    EXPR_COMPOUND,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_TERNARY,
    EXPR_MODIFY,
    EXPR_SIZEOF_EXPR,
    EXPR_SIZEOF_TYPE,
    EXPR_TYPEOF_EXPR,
    EXPR_TYPEOF_TYPE,
    EXPR_ALIGNOF_EXPR,
    EXPR_ALIGNOF_TYPE,
    EXPR_OFFSETOF,
    EXPR_NEW,
} ExprKind;

struct Expr {
    ExprKind kind;
    SrcPos pos;
    union {
        struct {
            Expr *expr;
        } paren;
        struct {
            unsigned long long val;
            TokenMod mod;
            TokenSuffix suffix;
        } int_lit;
        struct {
            const char *start;
            const char *end;
            double val;
            TokenSuffix suffix;
        } float_lit;
        struct {
            const char *val;
            TokenMod mod;
        } str_lit;
        const char *name;
        Expr *sizeof_expr;
        Typespec *sizeof_type;
        Expr *typeof_expr;
        Typespec *typeof_type;
        Expr *alignof_expr;
        Typespec *alignof_type;
        struct {
            Typespec *type;
            const char *name;
        } offsetof_field;
        //struct {
        //    Typespec *type;
        //    CompoundField *fields;
        //    size_t num_fields;
        //} compound;
        struct {
            Typespec *type;
            Expr *expr;            
        } cast;
        struct {
            TokenKind op;
            bool post;
            Expr *expr;
        } modify;
        struct {
            TokenKind op;
            Expr *expr;
        } unary;
        struct {
            TokenKind op;
            Expr *left;
            Expr *right;
        } binary;
        struct {
            Expr *cond;
            Expr *then_expr;
            Expr *else_expr;
        } ternary;
        struct {
            Expr *expr;
            Expr **args;
            size_t num_args;            
        } call;
        struct {
            Expr *expr;
            Expr *index;
        } index;
        struct {
            Expr *expr;
            const char *name;
        } field;
        struct {
            Expr *alloc;
            Expr *len;
            Expr *arg;
        } new_expr;
        struct {
            Expr **args;
            size_t num_args;          
        } tuple;
    };
};

void *ast_alloc(size_t size);
void *ast_dup(const void *src, size_t size);

Decl *new_decl(DeclKind kind, SrcPos pos, const char *name);
Decl *new_decl_func(SrcPos pos, const char *name, FuncParam *params, size_t num_params, Typespec *ret_type);

Expr *new_expr(ExprKind kind, SrcPos pos);
Expr *new_expr_paren(SrcPos pos, Expr *expr);
Expr *new_expr_binary(SrcPos pos, TokenKind op, Expr *left, Expr *right);
Expr *new_expr_int(SrcPos pos, unsigned long long val, TokenMod mod, TokenSuffix suffix);
Expr *new_expr_float(SrcPos pos, const char *start, const char *end, double val, TokenSuffix suffix);
Expr *new_expr_str(SrcPos pos, const char *val, TokenMod mod);
Expr *new_expr_name(SrcPos pos, const char *name);
Expr *new_expr_modify(SrcPos pos, TokenKind op, bool post, Expr *expr);
Expr *new_expr_unary(SrcPos pos, TokenKind op, Expr *expr);
Expr *new_expr_tuple(SrcPos pos, Expr **args, size_t num_args);
Expr *new_expr_call(SrcPos pos, Expr *expr, Expr **args, size_t num_args);
Expr *new_expr_index(SrcPos pos, Expr *expr, Expr *index);
Expr *new_expr_field(SrcPos pos, Expr *expr, const char *name);


Typespec *new_typespec(TypespecKind kind, SrcPos pos);
Typespec *new_typespec_name(SrcPos pos, const char **names, size_t num_names);
Typespec *new_typespec_tuple(SrcPos pos, Typespec **fields, size_t num_fields);