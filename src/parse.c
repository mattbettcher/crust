#include "parse.h"

// Already parsed
// v  v  v
// ( type, type, ...)
Typespec *parse_type_tuple(Typespec *type) {
    SrcPos pos = token.pos;
    Typespec **fields = NULL;
    buf_push(fields, type);
    while (!is_token(TOKEN_RPAREN)) {
        Typespec *field = parse_type();
        buf_push(fields, field);
        if (!match_token(TOKEN_COMMA)) {
            break;
        }
    }
    expect_token(TOKEN_RPAREN);
    return new_typespec_tuple(pos, fields, buf_len(fields));
}

// name | name.name | '(' type , tuple ')'
Typespec *parse_type_base(void) {
    if (is_token(TOKEN_NAME)) {
        SrcPos pos = token.pos;
        const char **names = NULL;
        buf_push(names, token.name);
        next_token();
        while (match_token(TOKEN_DOT)) {
            buf_push(names, parse_name());
        }
        return new_typespec_name(pos, names, buf_len(names));
    } else if (match_keyword(fn_keyword)) {
        // todo: fn types ie. function pointers
        //return parse_type_func();
    } else if (match_token(TOKEN_LPAREN)) {
        Typespec *type = parse_type();
        if (match_token(TOKEN_COMMA)) {
            return parse_type_tuple(type);
        }
        expect_token(TOKEN_RPAREN);
        return type;
    }
    fatal_error_here("Unexpected token %s in type", token_info());
    return NULL;
}

// todo: take some flags to limit what types are allowed
Typespec *parse_type(void) {
    Typespec *type = parse_type_base();
    SrcPos pos = token.pos;
    // todo: subscript/array and bracket and pointers
    return type;
}

const char *parse_name(void) {
    const char *name = token.name;
    expect_token(TOKEN_NAME);
    return name;
}


FuncParam parse_decl_func_param(void) {
    SrcPos pos = token.pos;
    const char *name = parse_name();
    expect_token(TOKEN_COLON);
    Typespec *type = parse_type();
    return (FuncParam){pos, name, type};
}

// 'const'? name (':' type)? ('=' expr)?
GenericParam parse_decl_generic_param(bool is_fn_decl) {
    SrcPos pos = token.pos;
    bool is_const = match_keyword(const_keyword);
    const char *name = parse_name();
    Typespec *type = NULL;
    if (match_token(TOKEN_COLON)) {
        type = parse_type();
    }
    if (match_token(TOKEN_EQ) && !is_fn_decl) {
        // todo: parse default generic value here if not an fn
    } else {
        fatal_error_here("defaults for const parameters are only allowed in `struct`, `enum`, `type`, or `trait` definitions");
    }
    return (GenericParam){pos, is_const, name, type};
}

// Already parsed
// v  
// fn name ('<' generic_param_list '>')? '(' param_list ')' ('->' type)? '{' block '}'
Decl *parse_decl_fn(SrcPos pos) {
    const char *name = parse_name();
    // generics go here
    GenericParam *generics = NULL;
    if (match_token(TOKEN_LT)) {
        buf_push(generics, parse_decl_generic_param(true));
        while (match_token(TOKEN_COMMA)) {
            buf_push(generics, parse_decl_generic_param(true));
        }
        expect_token(TOKEN_GT);
    }
    expect_token(TOKEN_LPAREN);
    FuncParam *params = NULL;
    if (!is_token(TOKEN_RPAREN)) {
        buf_push(params, parse_decl_func_param());
        while (match_token(TOKEN_COMMA)) {
            buf_push(params, parse_decl_func_param());
        }
    }
    expect_token(TOKEN_RPAREN);
    Typespec *ret_type = NULL;
    if (match_token(TOKEN_RARROW)) {
        ret_type = parse_type();
    }
    expect_token(TOKEN_LBRACE);
    // BLOCK !
    expect_token(TOKEN_RBRACE);
    Decl *decl = new_decl_func(pos, name, params, buf_len(params), ret_type);
    return decl;
}

Expr *parse_expr_operand(void) {
    SrcPos pos = token.pos;
    if (is_token(TOKEN_INT)) {
        unsigned long long val = token.int_val;
        TokenMod mod = token.mod;
        TokenSuffix suffix = token.suffix;
        next_token();
        return new_expr_int(pos, val, mod, suffix);
    } else if (is_token(TOKEN_FLOAT)) {
        const char *start = token.start;
        const char *end = token.end;
        double val = token.float_val;
        TokenSuffix suffix = token.suffix;
        next_token();
        return new_expr_float(pos, start, end, val, suffix);
    } else if (is_token(TOKEN_STR)) {
        const char *val = token.str_val;
        TokenMod mod = token.mod;
        next_token();
        return new_expr_str(pos, val, mod);    
    } else if (is_token(TOKEN_NAME)) {
        const char *name = token.name;
        next_token();
        return new_expr_name(pos, name);
    } else if (match_token(TOKEN_LPAREN)) { // possible tuple
        Expr *expr = parse_expr();
        if (match_token(TOKEN_COMMA)) {
            // tuple!
            Expr **args = NULL;
            buf_push(args, expr);
            if (!is_token(TOKEN_RPAREN)) {
                while (match_token(TOKEN_COMMA)) {
                    buf_push(args, parse_expr());
                }
            }
            expect_token(TOKEN_RPAREN);
            return new_expr_tuple(pos, args, buf_len(args));
        } else {
            expect_token(TOKEN_RPAREN);
            return new_expr_paren(pos, expr);
        }
    } else {
        fatal_error_here("Unexpected token %s in expression", token_info());
        return NULL;
    }
}

Expr *parse_expr_base(void) {
    Expr *expr = parse_expr_operand();
    while (is_token(TOKEN_LPAREN) || is_token(TOKEN_LBRACKET) || is_token(TOKEN_DOT) || is_token(TOKEN_INC) || is_token(TOKEN_DEC)) {
        SrcPos pos = token.pos;
        if (match_token(TOKEN_LPAREN)) {
            Expr **args = NULL;
            if (!is_token(TOKEN_RPAREN)) {
                buf_push(args, parse_expr());
                while (match_token(TOKEN_COMMA)) {
                    buf_push(args, parse_expr());
                }
            }
            expect_token(TOKEN_RPAREN);
            expr = new_expr_call(pos, expr, args, buf_len(args));
        } else if (match_token(TOKEN_LBRACKET)) {
            Expr *index = parse_expr();
            expect_token(TOKEN_RBRACKET);
            expr = new_expr_index(pos, expr, index);
        } else if (is_token(TOKEN_DOT)) {
            next_token();
            const char *field = token.name;
            expect_token(TOKEN_NAME);
            expr = new_expr_field(pos, expr, field);
        } else {
            assert(is_token(TOKEN_INC) || is_token(TOKEN_DEC));
            TokenKind op = token.kind;
            next_token();
            expr = new_expr_modify(pos, op, true, expr);
        }
    }
    return expr;
}

bool is_unary_op(void) {
    return
        is_token(TOKEN_ADD) ||
        is_token(TOKEN_SUB) ||
        is_token(TOKEN_MUL) ||
        is_token(TOKEN_AND) ||
        is_token(TOKEN_NEG) ||
        is_token(TOKEN_NOT) ||
        is_token(TOKEN_INC) ||
        is_token(TOKEN_DEC);
}

Expr *parse_expr_unary(void) {
    if (is_unary_op()) {
        SrcPos pos = token.pos;
        TokenKind op = token.kind;
        next_token();
        if (op == TOKEN_INC || op == TOKEN_DEC) {
            return new_expr_modify(pos, op, false, parse_expr_unary());
        } else {
            return new_expr_unary(pos, op, parse_expr_unary());
        }
    } else {
        return parse_expr_base();
    }
}

bool is_mul_op(void) {
    return TOKEN_FIRST_MUL <= token.kind && token.kind <= TOKEN_LAST_MUL;
}

Expr *parse_expr_mul(void) {
    Expr *expr = parse_expr_unary();
    while (is_mul_op()) {
        SrcPos pos = token.pos;
        TokenKind op = token.kind;
        next_token();
        expr = new_expr_binary(pos, op, expr, parse_expr_unary());
    }
    return expr;
}

bool is_add_op(void) {
    return TOKEN_FIRST_ADD <= token.kind && token.kind <= TOKEN_LAST_ADD;
}

Expr *parse_expr_add(void) {
    Expr *expr = parse_expr_mul();
    while (is_add_op()) {
        SrcPos pos = token.pos;
        TokenKind op = token.kind;
        next_token();
        expr = new_expr_binary(pos, op, expr, parse_expr_mul());
    }
    return expr;
}

bool is_cmp_op(void) {
    return TOKEN_FIRST_CMP <= token.kind && token.kind <= TOKEN_LAST_CMP;
}

Expr *parse_expr_cmp(void) {
    Expr *expr = parse_expr_add();
    while (is_cmp_op()) {
        SrcPos pos = token.pos;
        TokenKind op = token.kind;
        next_token();
        expr = new_expr_binary(pos, op, expr, parse_expr_add());
    }
    return expr;
}

Expr *parse_expr_and(void) {
    Expr *expr = parse_expr_cmp();
    while (match_token(TOKEN_AND_AND)) {
        SrcPos pos = token.pos;
        expr = new_expr_binary(pos, TOKEN_AND_AND, expr, parse_expr_cmp());
    }
    return expr;
}

Expr *parse_expr_or(void) {
    Expr *expr = parse_expr_and();
    while (match_token(TOKEN_OR_OR)) {
        SrcPos pos = token.pos;
        expr = new_expr_binary(pos, TOKEN_OR_OR, expr, parse_expr_and());
    }
    return expr;
}

Expr *parse_expr(void) {
    return parse_expr_or();
}

Expr *parse_paren_expr(void) {
    expect_token(TOKEN_LPAREN);
    Expr *expr = parse_expr();
    expect_token(TOKEN_RPAREN);
    return expr;
}