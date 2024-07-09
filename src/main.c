
// headers
#include "stdafx.h"
#include "common.h"
#include "lex.h"
#include "ast.h"
#include "parse.h"

// source
#include "common.c"
#include "lex.c"
#include "ast.c"
#include "parse.c"

i32 main(i32 argc, const char **argv) {
    init_keywords();
    char *filename = "../test.cr";
    char *test_file = read_file(filename);
    init_stream(filename, test_file);

    //Expr *e = parse_expr();
    match_keyword(fn_keyword);
    Decl *d = parse_decl_fn(token.pos);
}