//
// Created by MD Gaziur Rahman Noor on 1/1/26.
//

#ifndef NMLEX_H
#define NMLEX_H

#include "nmast.h"
#include "nmdiagnostics.h"
#include "nmfile.h"

typedef struct {
    NMFile *file;
    const char *file_path;
    NMString *file_content;
    const char *cur_char;
    size_t pos;
    size_t cur_line;
    size_t cur_col;
    bool handle_whitespace;
} Lexer;

Lexer *lexer_new(NMFile *file, bool handle_whitespace);
LexicalToken *lex_next(Lexer *lexer, Diagnostic **diagnostic);
void debug_lexical_token(LexicalToken *token);

#endif //NMLEX_H
