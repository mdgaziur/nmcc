//
// Created by MD Gaziur Rahman Noor on 7/1/26.
//

#include <nmcc/preprocess.h>

#include "nmcc/nmdebug.h"
#include "nmcc/nmlex.h"

void preprocess_add_include_directory(NMString *path) {

}
bool preprocess_code(NMFile *src_code, NMString *preprocessed_code) {
    Lexer *lexer = lexer_new(src_code, true);

    NMVec *lexer_diagnostics = nmvec_new(sizeof(Diagnostic*));
    LexicalToken *token = lex_next(lexer, lexer_diagnostics);

    while (true) {
        if (token) {
            debug_lexical_token(token);
            if (token->kind == LEX_EOF) break;
        }

        token = lex_next(lexer, lexer_diagnostics);
    }

    size_t i;
    for (i = 0; i < lexer_diagnostics->size; i++) {
        print_diagnostic(V(lexer_diagnostics, Diagnostic*, i));
    }

    while (lexer_diagnostics->size > 0)
        diagnostic_free(*(Diagnostic **)nmvec_pop(lexer_diagnostics));
    nmvec_free(lexer_diagnostics);
}
