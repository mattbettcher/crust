#include "lex.h"

static SrcPos pos_builtin = {.name = "<builtin>"};

static const char *token_suffix_names[] = {
    [SUFFIX_NONE] = "",
    [SUFFIX_D] = "d",
    [SUFFIX_U] = "u",
    [SUFFIX_L] = "l",
    [SUFFIX_UL] = "ul",
    [SUFFIX_LL] = "ll",
    [SUFFIX_ULL] = "ull",
};

static const char *token_kind_names[] = {
    [TOKEN_EOF] = "EOF",
    [TOKEN_COLON] = ":",
    [TOKEN_LPAREN] = "(",
    [TOKEN_RPAREN] = ")",
    [TOKEN_LBRACE] = "{",
    [TOKEN_RBRACE] = "}",
    [TOKEN_LBRACKET] = "[",
    [TOKEN_RBRACKET] = "]",
    [TOKEN_COMMA] = ",",
    [TOKEN_DOT] = ".",
    [TOKEN_AT] = "@",
    [TOKEN_POUND] = "#",
    [TOKEN_ELLIPSIS] = "...",
    [TOKEN_QUESTION] = "?",
    [TOKEN_SEMICOLON] = ";",
    [TOKEN_KEYWORD] = "keyword",
    [TOKEN_INT] = "int",
    [TOKEN_FLOAT] = "float",
    [TOKEN_STR] = "string",
    [TOKEN_NAME] = "name",
    [TOKEN_NEG] = "~",
    [TOKEN_NOT] = "!",
    [TOKEN_MUL] = "*",
    [TOKEN_DIV] = "/",
    [TOKEN_MOD] = "%",
    [TOKEN_AND] = "&",
    [TOKEN_LSHIFT] = "<<",
    [TOKEN_RSHIFT] = ">>",
    [TOKEN_ADD] = "+",
    [TOKEN_SUB] = "-",
    [TOKEN_OR] = "|",
    [TOKEN_XOR] = "^",
    [TOKEN_EQ] = "==",
    [TOKEN_NOTEQ] = "!=",
    [TOKEN_LT] = "<",
    [TOKEN_GT] = ">",
    [TOKEN_LTEQ] = "<=",
    [TOKEN_GTEQ] = ">=",
    [TOKEN_AND_AND] = "&&",
    [TOKEN_OR_OR] = "||",
    [TOKEN_ASSIGN] = "=",
    [TOKEN_ADD_ASSIGN] = "+=",
    [TOKEN_SUB_ASSIGN] = "-=",
    [TOKEN_OR_ASSIGN] = "|=",
    [TOKEN_AND_ASSIGN] = "&=",
    [TOKEN_XOR_ASSIGN] = "^=",
    [TOKEN_MUL_ASSIGN] = "*=",
    [TOKEN_DIV_ASSIGN] = "/=",
    [TOKEN_MOD_ASSIGN] = "%=",
    [TOKEN_LSHIFT_ASSIGN] = "<<=",
    [TOKEN_RSHIFT_ASSIGN] = ">>=",
    [TOKEN_INC] = "++",
    [TOKEN_DEC] = "--",
    [TOKEN_COLON_ASSIGN] = ":=",
};

static TokenKind assign_token_to_binary_token[NUM_TOKEN_KINDS] = {
    [TOKEN_ADD_ASSIGN] = TOKEN_ADD,
    [TOKEN_SUB_ASSIGN] = TOKEN_SUB,
    [TOKEN_OR_ASSIGN] = TOKEN_OR,
    [TOKEN_AND_ASSIGN] = TOKEN_AND,
    [TOKEN_XOR_ASSIGN] = TOKEN_XOR,
    [TOKEN_LSHIFT_ASSIGN] = TOKEN_LSHIFT,
    [TOKEN_RSHIFT_ASSIGN] = TOKEN_RSHIFT,
    [TOKEN_MUL_ASSIGN] = TOKEN_MUL,
    [TOKEN_DIV_ASSIGN] = TOKEN_DIV,
    [TOKEN_MOD_ASSIGN] = TOKEN_MOD,
};

static uint8_t char_to_digit[256] = {
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10, ['A'] = 10,
    ['b'] = 11, ['B'] = 11,
    ['c'] = 12, ['C'] = 12,
    ['d'] = 13, ['D'] = 13,
    ['e'] = 14, ['E'] = 14,
    ['f'] = 15, ['F'] = 15,
};

static char escape_to_char[256] = {
    ['0'] = '\0',
    ['\''] = '\'',
    ['"'] = '"',
    ['\\'] = '\\',
    ['n'] = '\n',
    ['r'] = '\r',
    ['t'] = '\t',
    ['v'] = '\v',
    ['b'] = '\b',
    ['a'] = '\a',
};

#define KEYWORD(name) name##_keyword = str_intern(#name); buf_push(keywords, name##_keyword)

static void 
init_keywords(void) {
    static bool inited;
    if (inited) {
        return;
    }
    KEYWORD(typedef);
    char *arena_end = intern_arena.end;
    KEYWORD(enum);
    KEYWORD(struct);
    KEYWORD(union);
    KEYWORD(const);
    KEYWORD(var);
    KEYWORD(fn);
    KEYWORD(import);
    KEYWORD(goto);
    KEYWORD(sizeof);
    KEYWORD(alignof);
    KEYWORD(typeof);
    KEYWORD(offsetof);
    KEYWORD(new);
    KEYWORD(break);
    KEYWORD(continue);
    KEYWORD(return);
    KEYWORD(if);
    KEYWORD(else);
    KEYWORD(while);
    KEYWORD(do);
    KEYWORD(for);
    KEYWORD(switch);
    KEYWORD(case);
    KEYWORD(default);
    KEYWORD(undef);
    assert(intern_arena.end == arena_end);
    first_keyword = typedef_keyword;
    last_keyword = undef_keyword;

    always_name = str_intern("always");
    foreign_name = str_intern("foreign");
    inline_name = str_intern("inline");
    complete_name = str_intern("complete");
    assert_name = str_intern("assert");
    intrinsic_name = str_intern("intrinsic");
    declare_note_name = str_intern("declare_note");
    static_assert_name = str_intern("static_assert");
    void_name = str_intern("void");

    inited = true;
}

#undef KEYWORD

static bool 
is_keyword_name(const char *name) {
    return first_keyword <= name && name <= last_keyword;
}

static const char *
token_kind_name(TokenKind kind) {
    if (kind < sizeof(token_kind_names)/sizeof(*token_kind_names)) {
        return token_kind_names[kind];
    } else {
        return "<unknown>";
    }
}

static void 
warning(SrcPos pos, const char *fmt, ...) {
    if (pos.name == NULL) {
        pos = pos_builtin;
    }
    va_list args;
    va_start(args, fmt);
    printf("%s(%d): warning: ", pos.name, pos.line);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

static void 
error(SrcPos pos, const char *fmt, ...) {
    if (pos.name == NULL) {
        pos = pos_builtin;
    }
    va_list args;
    va_start(args, fmt);
    printf("%s(%d): error: ", pos.name, pos.line);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

static const char *
token_info(void) {
    if (token.kind == TOKEN_NAME || token.kind == TOKEN_KEYWORD) {
        return token.name;
    } else {
        return token_kind_name(token.kind);
    }
}

static void 
scan_int(void) {
    int base = 10;
    const char *start_digits = stream;
    if (*stream == '0') {
        stream++;
        if (tolower(*stream) == 'x') {
            stream++;
            token.mod = MOD_HEX;
            base = 16;
            start_digits = stream;
        } else if (tolower(*stream) == 'b') {
            stream++;
            token.mod = MOD_BIN;
            base = 2;
            start_digits = stream;
        }
    }
    unsigned long long val = 0;
    for (;;) {
        if (*stream == '_') {
            stream++;
            continue;
        }
        int digit = char_to_digit[(unsigned char)*stream];
        if (digit == 0 && *stream != '0') {
            break;
        }
        if (digit >= base) {
            error_here("Digit '%c' out of range for base %d", *stream, base);
            digit = 0;
        }
        if (val > (ULLONG_MAX - digit)/base) {
            error_here("Integer literal overflow");
            while (isdigit(*stream)) {
                stream++;
            }
            val = 0;
            break;
        }
        val = val*base + digit;
        stream++;
    }
    if (stream == start_digits) {
        error_here("Expected base %d digit, got '%c'", base, *stream);
    }
    token.kind = TOKEN_INT;
    token.int_val = val;
    if (tolower(*stream) == 'u') {
        token.suffix = SUFFIX_U;
        stream++;
        if (tolower(*stream) == 'l') {
            token.suffix = SUFFIX_UL;
            stream++;
            if (tolower(*stream) == 'l') {
                token.suffix = SUFFIX_ULL;
                stream++;
            }
        }
    } else if (tolower(*stream) == 'l') {
        token.suffix = SUFFIX_L;
        stream++;
        if (tolower(*stream) == 'l') {
            token.suffix = SUFFIX_LL;
            stream++;
        }
    }
}

static void 
scan_float(void) {
    const char *start = stream;
    while (isdigit(*stream)) {
        stream++;
    }
    if (*stream == '.') {
        stream++;
    }
    while (isdigit(*stream)) {
        stream++;
    }
    if (tolower(*stream) == 'e') {
        stream++;
        if (*stream == '+' || *stream == '-') {
            stream++;
        }
        if (!isdigit(*stream)) {
            error_here("Expected digit after float literal exponent, found '%c'.", *stream);
        }
        while (isdigit(*stream)) {
            stream++;
        }
    }
    double val = strtod(start, NULL);
    if (val == HUGE_VAL) {
        error_here("Float literal overflow");
    }
    token.kind = TOKEN_FLOAT;
    token.float_val = val;
    if (tolower(*stream) == 'd') {
        token.suffix = SUFFIX_D;
        stream++;
    }
}

static int 
scan_hex_escape(void) {
    assert(*stream == 'x');
    stream++;
    int val = char_to_digit[(unsigned char)*stream];
    if (!val && *stream != '0') {
        error_here("\\x needs at least 1 hex digit");
    }
    stream++;
    int digit = char_to_digit[(unsigned char)*stream];
    if (digit || *stream == '0') {
        val *= 16;
        val += digit;
        if (val > 0xFF) {
            error_here("\\x argument out of range");
            val = 0xFF;
        }
        stream++;
    }
    return val;
}

static void 
scan_char(void) {
    assert(*stream == '\'');
    stream++;
    int val = 0;
    if (*stream == '\'') {
        error_here("Char literal cannot be empty");
        stream++;
    } else if (*stream == '\n') {
        error_here("Char literal cannot contain newline");
    } else if (*stream == '\\') {
        stream++;
        if (*stream == 'x') {
            val = scan_hex_escape();
        } else {
            val = escape_to_char[(unsigned char)*stream];
            if (val == 0 && *stream != '0') {
                error_here("Invalid char literal escape '\\%c'", *stream);
            }
            stream++;
        }
    } else {
        val = *stream;
        stream++;
    }
    if (*stream != '\'') {
        error_here("Expected closing char quote, got '%c'", *stream);
    } else {
        stream++;
    }
    token.kind = TOKEN_INT;
    token.int_val = val;
    token.mod = MOD_CHAR;
}

static void 
scan_str(void) {
    assert(*stream == '"');
    stream++;
    char *str = NULL;
    if (stream[0] == '"' && stream[1] == '"') {
        stream += 2;
        while (*stream) {
            if (stream[0] == '"' && stream[1] == '"' && stream[2] == '"') {
                stream += 3;
                break;
            }
            if (*stream != '\r') {
                // TODO: Should probably just read files in text mode instead.
                buf_push(str, *stream);
            }
            if (*stream == '\n') {
                token.pos.line++;
            }
            stream++;
        }
        if (!*stream) {
            error_here("Unexpected end of file within multi-line string literal");
        }
        token.mod = MOD_MULTILINE;
    } else {
        while (*stream && *stream != '"') {
            char val = *stream;
            if (val == '\n') {
                error_here("String literal cannot contain newline");
                break;
            } else if (val == '\\') {
                stream++;
                if (*stream == 'x') {
                    val = scan_hex_escape();
                } else {
                    val = escape_to_char[(unsigned char)*stream];
                    if (val == 0 && *stream != '0') {
                        error_here("Invalid string literal escape '\\%c'", *stream);
                    }
                    stream++;
                }
            } else {
                stream++;
            }
            buf_push(str, val);
        }
        if (*stream) {
            stream++;
        } else {
            error_here("Unexpected end of file within string literal");
        }
    }
    buf_push(str, 0);
    token.kind = TOKEN_STR;
    token.str_val = str;
}

#define CASE1(c1, k1) \
    case c1: \
        token.kind = k1; \
        stream++; \
        break;

#define CASE2(c1, k1, c2, k2) \
    case c1: \
        token.kind = k1; \
        stream++; \
        if (*stream == c2) { \
            token.kind = k2; \
            stream++; \
        } \
        break;

#define CASE3(c1, k1, c2, k2, c3, k3) \
    case c1: \
        token.kind = k1; \
        stream++; \
        if (*stream == c2) { \
            token.kind = k2; \
            stream++; \
        } else if (*stream == c3) { \
            token.kind = k3; \
            stream++; \
        } \
        break;

static void 
next_token(void) {
repeat:
    token.start = stream;
    token.mod = 0;
    token.suffix = 0;
    switch (*stream) {
    case ' ': case '\n': case '\r': case '\t': case '\v':
        while (isspace(*stream)) {
            if (*stream++ == '\n') {
                line_start = stream;
                token.pos.line++;
            }
        }
        goto repeat;
    case '\'':
        scan_char();
        break;
    case '"':
        scan_str();
        break;
    case '.':
        if (isdigit(stream[1])) {
            scan_float();
        } else if (stream[1] == '.' && stream[2] != '.') {
            token.kind = TOKEN_DOTDOT;
            stream += 3;
        } else if (stream[1] == '.' && stream[2] == '.') {
            token.kind = TOKEN_ELLIPSIS;
            stream += 3;
        } else {
            token.kind = TOKEN_DOT;
            stream++;
        }
        break;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
        while (isdigit(*stream)) {
            stream++;
        }
        char c = *stream;
        stream = token.start;
        if (c == '.' || tolower(c) == 'e') {
            scan_float();
        } else {
            scan_int();
        }
        break;
    }
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
    case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
    case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
    case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
    case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_':
        while (isalnum(*stream) || *stream == '_') {
            stream++;
        }
        token.name = str_intern_range(token.start, stream);
        token.kind = is_keyword_name(token.name) ? TOKEN_KEYWORD : TOKEN_NAME;
        break;
    case '<':
        token.kind = TOKEN_LT;
        stream++;
        if (*stream == '<') {
            token.kind = TOKEN_LSHIFT;
            stream++;
            if (*stream == '=') {
                token.kind = TOKEN_LSHIFT_ASSIGN;
                stream++;
            }
        } else if (*stream == '=') {
            token.kind = TOKEN_LTEQ;
            stream++;
        }
        break;
    case '>':
        token.kind = TOKEN_GT;
        stream++;
        if (*stream == '>') {
            token.kind = TOKEN_RSHIFT;
            stream++;
            if (*stream == '=') {
                token.kind = TOKEN_RSHIFT_ASSIGN;
                stream++;
            }
        } else if (*stream == '=') {
            token.kind = TOKEN_GTEQ;
            stream++;
        }
        break;
    //CASE3('-', TOKEN_SUB, '=', TOKEN_SUB_ASSIGN, '-', TOKEN_DEC)
    case '-':
        token.kind = TOKEN_SUB;
        stream++;
        if (*stream == '>') {
            token.kind = TOKEN_RARROW;
            stream++;
        } else if (*stream == '=') {
            token.kind = TOKEN_SUB_ASSIGN;
            stream++;
        } else if (*stream == '-') {
            token.kind = TOKEN_DEC;
            stream++;
        }
        break;
    case '/':
        token.kind = TOKEN_DIV;
        stream++;
        if (*stream == '=') {
            token.kind = TOKEN_DIV_ASSIGN;
            stream++;
        } else if (*stream == '/') {
            stream++;
            while (*stream && *stream != '\n') {
                stream++;
            }
            goto repeat;
        } else if (*stream == '*') {
            stream++;
            int level = 1;
            while (*stream && level > 0) {
                if (stream[0] == '/' && stream[1] == '*') {
                    level++;
                    stream += 2;
                } else if (stream[0] == '*' && stream[1] == '/') {
                    level--;
                    stream += 2;
                } else {
                    if (*stream == '\n') {
                        token.pos.line++;
                    }
                    stream++;
                }
            }
            goto repeat;
        }
        break;
    CASE1('\0', TOKEN_EOF)
    CASE1('(', TOKEN_LPAREN)
    CASE1(')', TOKEN_RPAREN)
    CASE1('{', TOKEN_LBRACE)
    CASE1('}', TOKEN_RBRACE)
    CASE1('[', TOKEN_LBRACKET)
    CASE1(']', TOKEN_RBRACKET)
    CASE1(',', TOKEN_COMMA)
    CASE1('@', TOKEN_AT)
    CASE1('#', TOKEN_POUND)
    CASE1('?', TOKEN_QUESTION)
    CASE1(';', TOKEN_SEMICOLON)
    CASE1('~', TOKEN_NEG)
    CASE2('!', TOKEN_NOT, '=', TOKEN_NOTEQ)
    CASE2(':', TOKEN_COLON, '=', TOKEN_COLON_ASSIGN)
    CASE2('=', TOKEN_ASSIGN, '=', TOKEN_EQ)
    CASE2('^', TOKEN_XOR, '=', TOKEN_XOR_ASSIGN)
    CASE2('*', TOKEN_MUL, '=', TOKEN_MUL_ASSIGN)
    CASE2('%', TOKEN_MOD, '=', TOKEN_MOD_ASSIGN)
    CASE3('+', TOKEN_ADD, '=', TOKEN_ADD_ASSIGN, '+', TOKEN_INC)
    //CASE3('-', TOKEN_SUB, '=', TOKEN_SUB_ASSIGN, '-', TOKEN_DEC)
    CASE3('&', TOKEN_AND, '=', TOKEN_AND_ASSIGN, '&', TOKEN_AND_AND)
    CASE3('|', TOKEN_OR, '=', TOKEN_OR_ASSIGN, '|', TOKEN_OR_OR)
    default:
        error_here("Invalid '%c' token, skipping", *stream);
        stream++;
        goto repeat;
    }
    token.end = stream;
}

#undef CASE1
#undef CASE2
#undef CASE3

static void 
init_stream(const char *name, const char *buf) {
    stream = buf;
    line_start = stream;
    token.pos.name = name ? name : "<string>";
    token.pos.line = 1;
    next_token();
}

static bool 
is_token(TokenKind kind) {
    return token.kind == kind;
}

static bool 
is_token_eof(void) {
    return token.kind == TOKEN_EOF;
}

static bool 
is_token_name(const char *name) {
    return token.kind == TOKEN_NAME && token.name == name;
}

static bool 
is_keyword(const char *name) {
    return is_token(TOKEN_KEYWORD) && token.name == name;
}

static bool 
match_keyword(const char *name) {
    if (is_keyword(name)) {
        next_token();
        return true;
    } else {
        return false;
    }
}

static bool 
match_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    } else {
        return false;
    }
}

static bool 
expect_token(TokenKind kind, TokenKind *next_token_kind, bool is_recoverable) {
    if (is_token(kind)) {
        next_token();
        return true;
    } else if (is_recoverable) {
        Error err = {
            .kind = ERROR_EXPECTED,
            .pos = token.pos,
            .corrected = true,
            .expected = {
                .expected_token = kind,
                .found_token = token.kind,
            }
        };
        u8 i = 0;
        while (next_token_kind[i]) {
            if (is_token(next_token_kind[i])) {
                // assume expected token is just missing and the next token is
                // of the expected kind
                goto end;
            }
            i++;
        }
        // assume token is wrong and consume it
        Token wrong_token = token;
        next_token();
        i = 0;
        while (next_token_kind[i]) {
            if (is_token(next_token_kind[i])) {
                // we found a correct token carry on
                goto end;
            }
            i++;
        }
        end:
        buf_push(errors, err);
        return true;
    } else {
        fatal_error_here("Expected token %s, got %s", token_kind_name(kind), token_info());
        return false;
    }
}