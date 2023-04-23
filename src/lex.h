#pragma once

#include "stdafx.h"
#include "common.h"

const char *typedef_keyword;
const char *enum_keyword;
const char *struct_keyword;
const char *union_keyword;
const char *var_keyword;
const char *const_keyword;
const char *fn_keyword;
const char *sizeof_keyword;
const char *alignof_keyword;
const char *typeof_keyword;
const char *offsetof_keyword;
const char *break_keyword;
const char *continue_keyword;
const char *return_keyword;
const char *if_keyword;
const char *else_keyword;
const char *while_keyword;
const char *do_keyword;
const char *for_keyword;
const char *switch_keyword;
const char *case_keyword;
const char *default_keyword;
const char *import_keyword;
const char *goto_keyword;
const char *new_keyword;
const char *undef_keyword;

const char *first_keyword;
const char *last_keyword;
const char **keywords;

const char *always_name;
const char *foreign_name;
const char *inline_name;
const char *complete_name;
const char *assert_name;
const char *intrinsic_name;
const char *declare_note_name;
const char *static_assert_name;
const char *void_name;

typedef enum TokenKind {
    TOKEN_EOF,
    TOKEN_COLON,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_RARROW,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_AT,
    TOKEN_POUND,
    TOKEN_DOTDOT,
    TOKEN_ELLIPSIS,
    TOKEN_QUESTION,
    TOKEN_SEMICOLON,
    TOKEN_KEYWORD,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STR,
    TOKEN_NAME,
    TOKEN_NEG,
    TOKEN_NOT,
    // Multiplicative precedence
    TOKEN_FIRST_MUL,
    TOKEN_MUL = TOKEN_FIRST_MUL,
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_AND,
    TOKEN_LSHIFT,
    TOKEN_RSHIFT,
    TOKEN_LAST_MUL = TOKEN_RSHIFT,
    // Additive precedence
    TOKEN_FIRST_ADD,
    TOKEN_ADD = TOKEN_FIRST_ADD,
    TOKEN_SUB,
    TOKEN_XOR,
    TOKEN_OR,
    TOKEN_LAST_ADD = TOKEN_OR,
    // Comparative precedence
    TOKEN_FIRST_CMP,
    TOKEN_EQ = TOKEN_FIRST_CMP,
    TOKEN_NOTEQ,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LTEQ,
    TOKEN_GTEQ,
    TOKEN_LAST_CMP = TOKEN_GTEQ,
    TOKEN_AND_AND,
    TOKEN_OR_OR,
    // Assignment operators
    TOKEN_FIRST_ASSIGN,
    TOKEN_ASSIGN = TOKEN_FIRST_ASSIGN,
    TOKEN_ADD_ASSIGN,
    TOKEN_SUB_ASSIGN,
    TOKEN_OR_ASSIGN,
    TOKEN_AND_ASSIGN,
    TOKEN_XOR_ASSIGN,
    TOKEN_LSHIFT_ASSIGN,
    TOKEN_RSHIFT_ASSIGN,
    TOKEN_MUL_ASSIGN,
    TOKEN_DIV_ASSIGN,
    TOKEN_MOD_ASSIGN,
    TOKEN_LAST_ASSIGN = TOKEN_MOD_ASSIGN,
    TOKEN_INC,
    TOKEN_DEC,
    TOKEN_COLON_ASSIGN,
    NUM_TOKEN_KINDS,
} TokenKind;

typedef enum TokenMod {
    MOD_NONE,
    MOD_HEX,
    MOD_BIN,
    MOD_CHAR,
    MOD_MULTILINE,
} TokenMod;

typedef enum TokenSuffix {
    SUFFIX_NONE,
    SUFFIX_D,
    SUFFIX_U,
    SUFFIX_L,
    SUFFIX_UL,
    SUFFIX_LL,
    SUFFIX_ULL,
} TokenSuffix;

typedef struct SrcPos {
    const char *name;
    int line;
} SrcPos;

typedef struct Token {
    TokenKind kind;
    TokenMod mod;
    TokenSuffix suffix;
    SrcPos pos;
    const char *start;
    const char *end;
    union {
        unsigned long long int_val;
        double float_val;
        const char *str_val;
        const char *name;
    };
} Token;

Token token;
const char *stream;
const char *line_start;

void init_keywords(void);
bool is_keyword_name(const char *name);
const char *token_kind_name(TokenKind kind);
void warning(SrcPos pos, const char *fmt, ...);
void error(SrcPos pos, const char *fmt, ...);

#define fatal_error(...) (error(__VA_ARGS__), exit(1))
#define error_here(...) (error(token.pos, __VA_ARGS__))
#define warning_here(...) (error(token.pos, __VA_ARGS__))
#define fatal_error_here(...) (error_here(__VA_ARGS__), exit(1)) // should be abort()

const char *token_info(void);
void scan_int(void);
void scan_float(void);
int scan_hex_escape(void);
void scan_char(void);
void scan_str(void);
void next_token(void);
void init_stream(const char *name, const char *buf);
bool is_token(TokenKind kind);
bool is_token_eof(void);
bool is_token_name(const char *name);
bool is_keyword(const char *name);
bool match_keyword(const char *name);
bool match_token(TokenKind kind);
bool expect_token(TokenKind kind);