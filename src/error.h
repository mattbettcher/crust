#pragma once

#include "stdafx.h"
#include "common.h"
#include "lex.h"

typedef enum ErrorKind {
    ERROR_NONE,
    ERROR_EXPECTED,
} ErrorKind;

typedef struct Error {
    ErrorKind kind;
    SrcPos pos;
    bool corrected;
    
    union {
        struct {
            TokenKind expected_token;
            TokenKind found_token;
        } expected;
    };
    
} Error;

static Error *errors = NULL;

static void print_error(Error *error);