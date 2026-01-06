//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <nmcc/nmlex.h>

#include "nmcc/nmcolors.h"
#include "nmcc/nmdebug.h"
#include "nmcc/nmdiagnostics.h"
#include "nmcc/nmerror.h"
#include "nmcc/nmmust.h"

void lexical_token_free(LexicalToken *token) {
    if (token->lexeme) nmstring_free(token->lexeme);
    free(token);
}

#define LEXER_ADVANCE(adv) \
    lexer->cur_char += adv; \
    lexer->cur_col += adv;

Lexer *lexer_new(NMFile *file, bool handle_whitespace) {
    Lexer *lexer = malloc(sizeof(Lexer));
    NOT_NULL(lexer, "Failed to allocate data for the lexer");

    lexer->file = file;
    lexer->file_path = file->path;
    lexer->file_content = nmfile_read_to_string(file);
    lexer->pos = 0;
    lexer->cur_char = S(lexer->file_content);
    lexer->cur_line = 1;
    lexer->cur_col = 1;
    lexer->handle_whitespace = handle_whitespace;
    return lexer;
}

void lexer_free(Lexer *lexer) {
    nmstring_free(lexer->file_content);
    free(lexer);
}

LexicalToken *maybe_handle_space(Lexer *lexer, LexicalToken *token, Diagnostic **diagnostic) {
    if (lexer->handle_whitespace) {
        char cur_char = *lexer->cur_char;

        switch (cur_char) {
            case ' ':
                token->kind = LEX_SPACE;
                break;
            case '\n':
                token->kind = LEX_NEWLINE;
                break;
            case '\f':
                token->kind = LEX_FORMFEED;
                break;
            case '\r':
                token->kind = LEX_CARRIAGE_RETURN;
                break;
            case '\t':
                token->kind = LEX_TAB;
                break;
            case '\v':
                token->kind = LEX_VERTICAL_TAB;
                break;
            default:
                fprintf(stderr, DIAG_CERROR "INTERNAL ERROR: lex:maybe_handle_space called on non space character: `%c`(`%d`) [%s:%zu:%zu]" ANSI_RESET,
                    cur_char,
                    cur_char,
                    lexer->file_path,
                    lexer->cur_line,
                    lexer->cur_col);
                FATAL();
        }

        token->lexeme = nmstring_new_from_char(cur_char);
        token->span = span_for_single_char(lexer->file_path, lexer->cur_line, lexer->cur_col);

        return token;
    }

    LEXER_ADVANCE(1);

    free(token);
    return lex_next(lexer, diagnostic);
}

/*
 * `lexer->cur_char` must point to the first matching character. Also,
 * this assumes that NUL is not used as the second matching character.
 *
 * Another important assumption is that each char from `other_char` corresponds
 * to a kind from `other_kinds` where both are located in the same index.
 */
void lex_upto_two_char_token(Lexer *lexer,
    LexicalToken *token,
    LexKind single_kind,
    const char *other_chars,
    const LexKind *other_kinds,
    size_t n_other_kinds,
    bool *is_single_char_token) {
    size_t line_start = lexer->cur_line;
    size_t col_start = lexer->cur_col;
    const char next_char = *(lexer->cur_char + 1);

    // EOF
    if (!next_char) return;

    size_t cur_token;
    for (cur_token = 0; cur_token < n_other_kinds; cur_token++) {
        if (next_char == other_chars[cur_token]) {
            token->kind = other_kinds[cur_token];

            NMString *lexeme = nmstring_new_from_char(*lexer->cur_char);
            LEXER_ADVANCE(1);
            nmstring_append(lexeme, next_char);

            token->lexeme = lexeme;

            token->span = span_new(lexer->file_path,
                line_start,
                col_start,
                lexer->cur_line,
                lexer->cur_col);

            return;
        }
    }

    token->kind = single_kind;
    *is_single_char_token = true;
}

void handle_escape_sequence(Lexer *lexer, NMString *lexeme, Diagnostic **diagnostic) {
    switch (*lexer->cur_char) {
        case '\'':
            nmstring_append(lexeme, '\'');
            break;
        case '"':
            nmstring_append(lexeme, '"');
            break;
        case '?':
            nmstring_append(lexeme, '\?');
            break;
        case '\\':
            nmstring_append(lexeme, '\\');
            break;
        case 'a':
            nmstring_append(lexeme, '\a');
            break;
        case 'b':
            nmstring_append(lexeme, '\b');
            break;
        case 'f':
            nmstring_append(lexeme, '\f');
            break;
        case 'n':
            nmstring_append(lexeme, '\n');
            break;
        case 'r':
            nmstring_append(lexeme, '\r');
            break;
        case 't':
            nmstring_append(lexeme, '\t');
            break;
        case 'v':
            nmstring_append(lexeme, '\v');
            break;
        default:
            *diagnostic = diagnostic_for_single_char(
                DIAG_WARNING,
                lexer->file,
                "Unknown escape sequence",
                lexer->cur_line,
                lexer->cur_col);
    }
}

void lex_string(Lexer *lexer, LexicalToken **token, bool is_wide_string, Diagnostic **diagnostic) {
    size_t start_line = lexer->cur_line;
    size_t start_col = lexer->cur_col;
    NMString *lexeme = nmstring_new_from_char(*lexer->cur_char);
    LEXER_ADVANCE(1);

    if (is_wide_string) {
        nmstring_append(lexeme, *lexer->cur_char);
        LEXER_ADVANCE(1);
    }

    while (*lexer->cur_char && *lexer->cur_char != '"' && *lexer->cur_char != '\n') {
        if (*lexer->cur_char == '\\') {
            LEXER_ADVANCE(1);
            handle_escape_sequence(lexer, lexeme, diagnostic);
            LEXER_ADVANCE(1);
            continue;
        }
        nmstring_append(lexeme, *lexer->cur_char);
        LEXER_ADVANCE(1);
    }

    bool has_error = false;
    if (!lexer->cur_char) {
        has_error = true;

        *diagnostic = diagnostic_for_single_char(DIAG_ERROR, lexer->file, "Unexpected EOF", lexer->cur_line, lexer->cur_col);
    } else if (*lexer->cur_char != '"') {
        has_error = true;

        *diagnostic = diagnostic_for_single_char(DIAG_ERROR, lexer->file, "Unterminated string", lexer->cur_line, lexer->cur_col);
    }

    if (has_error) {
        LEXER_ADVANCE(-1);
        free(*token);
        nmstring_free(lexeme);
        *token = NULL;
    } else {
        nmstring_append(lexeme, *lexer->cur_char);
        (*token)->kind = LEX_STRING;
        (*token)->lexeme = lexeme;
        (*token)->span = span_new(lexer->file_path, start_line, start_col, lexer->cur_line, lexer->cur_col);
    }
}

void lex_float_or_decimal(Lexer *lexer, LexicalToken **token, Diagnostic **diagnostic) {
    NMString *lexeme = nmstring_new_from_char(*lexer->cur_char);
    const size_t start_line = lexer->cur_line;
    const size_t start_col = lexer->cur_col;

    LEXER_ADVANCE(1);

    while (*lexer->cur_char
        && (*lexer->cur_char == '.'|| isnumber(*lexer->cur_char))) {
        nmstring_append(lexeme, *lexer->cur_char);

        LEXER_ADVANCE(1);
    }

    LEXER_ADVANCE(-1);

    size_t dot_count = nmstring_count(lexeme, '.');
    if (dot_count > 1) {
        Span sp = {
            .file_path = lexer->file_path,
            .line_start = start_line,
            .linepos_start = start_col,
            .line_end = lexer->cur_line,
            .linepos_end = lexer->cur_line,
        };
        *diagnostic = diagnostic_for_span(
            DIAG_ERROR,
            "Invalid floating constant",
            lexer->file,
            &sp
        );

        nmstring_free(lexeme);

        return;
    }

    if (dot_count == 0) {
        (*token)->kind = LEX_INT;
        (*token)->lexeme = lexeme;
        (*token)->span = span_new(
            lexer->file_path,
            start_line,
            start_col,
            lexer->cur_line,
            lexer->cur_col);

        return;
    }

    LEXER_ADVANCE(1);

    if (tolower(*lexer->cur_char) == 'e') {
        nmstring_append(lexeme, *lexer->cur_char);
        LEXER_ADVANCE(1);

        if (lexer->cur_char[0] == '+' || lexer->cur_char[0] == '-') {
            nmstring_append(lexeme, *lexer->cur_char);
            LEXER_ADVANCE(1);
        }

        while (*lexer->cur_char && isnumber(*lexer->cur_char)) {
            nmstring_append(lexeme, *lexer->cur_char);
            LEXER_ADVANCE(1);
        }
    }

    (*token)->kind = LEX_FLOAT;
    (*token)->lexeme = lexeme;
    (*token)->span = span_new(
        lexer->file_path,
        start_line,
        start_col,
        lexer->cur_line,
        lexer->cur_col);
}

void lex_hex(Lexer *lexer, LexicalToken **token, Diagnostic **diagnostic) {
    NMString *lexeme = nmstring_new_from_str("0x");
    const size_t start_line = lexer->cur_line;
    const size_t start_col = lexer->cur_col;

    // Eat 0x
    LEXER_ADVANCE(2);

    while (*lexer->cur_char && (isnumber(*lexer->cur_char)
        || ('a' <= tolower(*lexer->cur_char) && tolower(*lexer->cur_char <= 'f')))) {
        nmstring_append(lexeme, *lexer->cur_char);
        LEXER_ADVANCE(1);
    }
    LEXER_ADVANCE(-1);

    (*token)->kind = LEX_NUMBER_HEX;
    (*token)->lexeme = lexeme;
    (*token)->span = span_new(
        lexer->file_path,
        start_line,
        start_col,
        lexer->cur_line,
        lexer->cur_col);
}

void lex_octal(Lexer *lexer, LexicalToken **token, Diagnostic **diagnostic) {
    NMString *lexeme = nmstring_new_from_char(*lexer->cur_char);
    const size_t start_line = lexer->cur_line;
    const size_t start_col = lexer->cur_col;

    LEXER_ADVANCE(1);

    while (*lexer->cur_char && isnumber(*lexer->cur_char)) {
        if (*lexer->cur_char > '7') {
            nmstring_free(lexeme);
            *diagnostic = diagnostic_for_single_char(
                DIAG_ERROR,
                lexer->file,
                "Invalid digit inside octal number",
                lexer->cur_line,
                lexer->cur_col);

            return;
        }
        nmstring_append(lexeme, *lexer->cur_char);
        LEXER_ADVANCE(1);
    }
    LEXER_ADVANCE(-1);

    (*token)->kind = LEX_NUMBER_OCT;
    (*token)->lexeme = lexeme;
    (*token)->span = span_new(
        lexer->file_path,
        start_line,
        start_col,
        lexer->cur_line,
        lexer->cur_col);
}

void lex_number(Lexer *lexer, LexicalToken **token, Diagnostic **diagnostic) {
    const char cur_char = *(lexer->cur_char);

    if (cur_char != '0') {
        lex_float_or_decimal(lexer, token, diagnostic);
    } else if (*(lexer->cur_char + 1) == 'x') {
        lex_hex(lexer, token, diagnostic);
    } else {
        lex_octal(lexer, token, diagnostic);
    }
}

void lex_identifier(Lexer *lexer, LexicalToken *token) {
    size_t start_line = lexer->cur_line;
    size_t start_col = lexer->cur_col;
    NMString *lexeme = nmstring_new_from_char(*lexer->cur_char);
    LEXER_ADVANCE(1);

    while (*lexer->cur_char && (isalnum(*lexer->cur_char) || *lexer->cur_char == '_')) {
        nmstring_append(lexeme, *lexer->cur_char);
        LEXER_ADVANCE(1);
    }

    LEXER_ADVANCE(-1);

    token->lexeme = lexeme;

    if (NM_EQ_C(lexeme, "auto")) token->kind = LEX_AUTO;
    else if (NM_EQ_C(lexeme, "double")) token->kind = LEX_DOUBLE;
    else if (NM_EQ_C(lexeme, "int")) token->kind = LEX_INT;
    else if (NM_EQ_C(lexeme, "struct")) token->kind = LEX_STRUCT;
    else if (NM_EQ_C(lexeme, "break")) token->kind = LEX_BREAK;
    else if (NM_EQ_C(lexeme, "else")) token->kind = LEX_ELSE;
    else if (NM_EQ_C(lexeme, "long")) token->kind = LEX_LONG;
    else if (NM_EQ_C(lexeme, "switch")) token->kind = LEX_SWITCH;
    else if (NM_EQ_C(lexeme, "case")) token->kind = LEX_CASE;
    else if (NM_EQ_C(lexeme, "enum")) token->kind = LEX_ENUM;
    else if (NM_EQ_C(lexeme, "typedef")) token->kind = LEX_TYPEDEF;
    else if (NM_EQ_C(lexeme, "char")) token->kind = LEX_CHAR;
    else if (NM_EQ_C(lexeme, "extern")) token->kind = LEX_EXTERN;
    else if (NM_EQ_C(lexeme, "return")) token->kind = LEX_RETURN;
    else if (NM_EQ_C(lexeme, "union")) token->kind = LEX_UNION;
    else if (NM_EQ_C(lexeme, "const")) token->kind = LEX_CONST;
    else if (NM_EQ_C(lexeme, "float")) token->kind = LEX_FLOAT;
    else if (NM_EQ_C(lexeme, "short")) token->kind = LEX_SHORT;
    else if (NM_EQ_C(lexeme, "unsigned")) token->kind = LEX_UNSIGNED;
    else if (NM_EQ_C(lexeme, "continue")) token->kind = LEX_CONTINUE;
    else if (NM_EQ_C(lexeme, "for")) token->kind = LEX_FOR;
    else if (NM_EQ_C(lexeme, "signed")) token->kind = LEX_SIGNED;
    else if (NM_EQ_C(lexeme, "void")) token->kind = LEX_VOID;
    else if (NM_EQ_C(lexeme, "default")) token->kind = LEX_DEFAULT;
    else if (NM_EQ_C(lexeme, "goto")) token->kind = LEX_GOTO;
    else if (NM_EQ_C(lexeme, "sizeof")) token->kind = LEX_SIZEOF;
    else if (NM_EQ_C(lexeme, "volatile")) token->kind = LEX_VOLATILE;
    else if (NM_EQ_C(lexeme, "do")) token->kind = LEX_DO;
    else if (NM_EQ_C(lexeme, "if")) token->kind = LEX_IF;
    else if (NM_EQ_C(lexeme, "static")) token->kind = LEX_STATIC;
    else if (NM_EQ_C(lexeme, "while")) token->kind = LEX_WHILE;
    else token->kind = LEX_IDENT;

    token->span = span_new(lexer->file_path, start_line, start_col, lexer->cur_line, lexer->cur_col);
}

LexicalToken *lex_next(Lexer *lexer, Diagnostic **diagnostic) {
    LexicalToken *token = malloc(sizeof(LexicalToken));
    NOT_NULL(token, "Failed to allocate data for lexical token");

    bool is_single_char_token = false;

#define SINGLE_CHAR_TOKEN(tok, tok_kind) case tok: {\
    token->kind = tok_kind; \
    is_single_char_token = true; \
    break; \
}

#define TWO_CHAR_TOKEN(tok, single_kind, other_chars, ...) case tok: {\
        const LexKind kinds[] = { __VA_ARGS__ }; \
        lex_upto_two_char_token( \
            lexer, \
            token, \
            single_kind, \
            other_chars, \
            kinds, \
            sizeof(kinds) / sizeof(LexKind), \
            &is_single_char_token \
        ); \
        break; \
    }

    const char next_char = *(lexer->cur_char + 1);
    switch (*lexer->cur_char) {
        case '\n': {
            lexer->cur_line++;
            lexer->cur_col = 0;
        }
        case '\r':
        case '\t':
        case ' ':
        case '\f':
        case '\v':
            return maybe_handle_space(lexer, token, diagnostic);

        // Punctuators
        SINGLE_CHAR_TOKEN('[', LEX_LBRACK)
        SINGLE_CHAR_TOKEN(']', LEX_RBRACK)
        SINGLE_CHAR_TOKEN('{', LEX_LBRACE)
        SINGLE_CHAR_TOKEN('}', LEX_RBRACE)
        SINGLE_CHAR_TOKEN('(', LEX_LPAREN)
        SINGLE_CHAR_TOKEN(')', LEX_RPAREN)
        SINGLE_CHAR_TOKEN(',', LEX_COMMA)
        SINGLE_CHAR_TOKEN(':', LEX_COLON)
        SINGLE_CHAR_TOKEN(';', LEX_SEMICOLON)
        SINGLE_CHAR_TOKEN('~', LEX_BINARY_NOT)
        SINGLE_CHAR_TOKEN('?', LEX_QUESTION)
        SINGLE_CHAR_TOKEN('\\', LEX_BACKSLASH)

        TWO_CHAR_TOKEN('-', LEX_MINUS, ">-=", LEX_ARROW, LEX_DEC, LEX_MINUS_ASSIGN)
        TWO_CHAR_TOKEN('+', LEX_PLUS, "+=", LEX_INC, LEX_PLUS_ASSIGN)
        TWO_CHAR_TOKEN('&', LEX_AMP, "&=", LEX_AND, LEX_AND_ASSIGN)
        TWO_CHAR_TOKEN('%', LEX_MOD, "=", LEX_MOD_ASSIGN)
        TWO_CHAR_TOKEN('^', LEX_XOR, "=", LEX_XOR_ASSIGN)
        TWO_CHAR_TOKEN('!', LEX_NOT, "=", LEX_NOT_EQUALS)
        TWO_CHAR_TOKEN('=', LEX_ASSIGN, "=", LEX_EQUAL)
        TWO_CHAR_TOKEN('|', LEX_BINARY_OR, "|=", LEX_OR, LEX_OR_ASSIGN)
        TWO_CHAR_TOKEN('*', LEX_STAR, "=", LEX_MUL_ASSIGN)
        TWO_CHAR_TOKEN('#', LEX_HASH, "#", LEX_DHASH);

        case '/': {
            size_t start_line = lexer->cur_line;
            size_t start_col = lexer->cur_col;

            if (next_char == '=') {
                is_single_char_token = false;
                LEXER_ADVANCE(1);

                token->kind = LEX_DIV_ASSIGN;
                token->lexeme = nmstring_new_from_str("/=");
            } else if (next_char == '/') {
                is_single_char_token = false;
                LEXER_ADVANCE(1);

                NMString *lexeme = nmstring_new_from_str("/");
                while (*lexer->cur_char && *lexer->cur_char != '\n') {
                    nmstring_append(lexeme, *lexer->cur_char);
                    LEXER_ADVANCE(1);
                }
                LEXER_ADVANCE(-1);

                token->kind = LEX_COMMENT;
                token->lexeme = lexeme;
            } else if (next_char == '*') {
                is_single_char_token = false;
                LEXER_ADVANCE(2);

                NMString *lexeme = nmstring_new_from_str("/*");
                while (*lexer->cur_char) {
                    nmstring_append(lexeme, *lexer->cur_char);

                    if (*lexer->cur_char == '\n') {
                        lexer->cur_line++;
                        lexer->cur_col = 1;
                    }

                    if (*lexer->cur_char == '*' && *(lexer->cur_char + 1) == '/') {
                        LEXER_ADVANCE(1);
                        nmstring_append(lexeme, *lexer->cur_char);
                        break;
                    }

                    LEXER_ADVANCE(1);
                }

                token->kind = LEX_COMMENT;
                token->lexeme = lexeme;

            } else {
                token->kind = LEX_SLASH;
                is_single_char_token = true;
            }

            if (!is_single_char_token) {
                token->span = span_new(
                    lexer->file_path,
                    start_line,
                    start_col,
                    lexer->cur_line,
                    lexer->cur_col
                );
            }
            break;
        }
        case '.': {
            if (next_char == '.') {
                if (*(lexer->cur_char + 2) == '.') {
                    LEXER_ADVANCE(2);

                    token->kind = LEX_THREE_DOTS;
                    token->lexeme = nmstring_new_from_str("...");
                }
            } else {
                token->kind = LEX_DOT;
                is_single_char_token = true;
            }

            if (!is_single_char_token) {
                token->span = span_new(lexer->file_path,
                    lexer->cur_line,
                    lexer->cur_col - 2,
                    lexer->cur_line,
                    lexer->cur_col);
            }

            break;
        }
        case '>':
            if (next_char == '>') {
                const char next_next_token = *(lexer->cur_char + 2);
                if (next_next_token == '=') {
                    LEXER_ADVANCE(2);

                    token->kind = LEX_RSHIFT_ASSIGN;
                    token->lexeme = nmstring_new_from_str(">>=");
                } else {
                    LEXER_ADVANCE(1);

                    token->kind = LEX_RSHIFT;
                    token->lexeme = nmstring_new_from_str(">>");
                }
            } else if (next_char == '=') {
                is_single_char_token = false;
                LEXER_ADVANCE(1);

                token->kind = LEX_GREATER_EQUAL;
                token->lexeme = nmstring_new_from_str(">=");
            } else {
                token->kind = LEX_GREATER;
                is_single_char_token = true;
            }

            break;
        case '<':
            if (next_char == '<') {
                const char next_next_token = *(lexer->cur_char + 2);
                if (next_next_token == '=') {
                    LEXER_ADVANCE(2);

                    token->kind = LEX_LSHIFT_ASSIGN;
                    token->lexeme = nmstring_new_from_str(">>=");
                } else {
                    LEXER_ADVANCE(1);

                    token->kind = LEX_LSHIFT;
                    token->lexeme = nmstring_new_from_str(">>");
                }
            } else if (next_char == '=') {
                is_single_char_token = false;
                LEXER_ADVANCE(1);

                token->kind = LEX_LESS_EQUAL;
                token->lexeme = nmstring_new_from_str("<=");
            } else {
                token->kind = LEX_LESS;
                is_single_char_token = true;
            }

            break;
        case 0:
            token->kind = LEX_EOF;
            break;
        default: {
            const char cur_char = *lexer->cur_char;
            bool is_wide_string = cur_char == 'L' && *(lexer->cur_char + 1) == '"';
            if (cur_char == '"' || is_wide_string) {
                lex_string(lexer, &token, is_wide_string, diagnostic);
            } else if (cur_char == '\'') {
                NMString *lexeme = nmstring_new_from_char('\'');
                LEXER_ADVANCE(1);

                if (*lexer->cur_char == '\\') {
                    LEXER_ADVANCE(1);
                    handle_escape_sequence(lexer, lexeme, diagnostic);
                    LEXER_ADVANCE(1);
                } else {
                    nmstring_append(lexeme, *lexer->cur_char);
                    nmstring_append(lexeme, '\'');

                    LEXER_ADVANCE(1);
                }

                if (*lexer->cur_char != '\'') {
                    Span sp = {
                        .file_path = lexer->file_path,
                        .line_start = lexer->cur_line,
                        .linepos_start = lexer->cur_col - 2,
                        .line_end = lexer->cur_line,
                        .linepos_end = lexer->cur_col,
                    };

                    *diagnostic = diagnostic_for_span(
                        DIAG_ERROR,
                        "Unterminated character literal",
                        lexer->file,
                        &sp);
                    free(token);
                    nmstring_free(lexeme);
                    token = NULL;
                }
                if (token) {
                    token->kind = LEX_CHAR_LIT;
                    token->lexeme = lexeme;
                    token->span = span_new(
                        lexer->file_path,
                        lexer->cur_line,
                        lexer->cur_col - 2,
                        lexer->cur_line,
                        lexer->cur_col);
                }
            } else if (isalpha(cur_char) || cur_char == '_') {
                lex_identifier(lexer, token);
            } else if (isnumber(cur_char)) {
                lex_number(lexer, &token, diagnostic);
            } else {
                *diagnostic = diagnostic_for_single_char(
                    DIAG_ERROR,
                    lexer->file,
                    "Unknown character",
                    lexer->cur_line,
                    lexer->cur_col);
                free(token);
                token = NULL;
            }
        }
    }
#undef SINGLE_CHAR_TOKEN
#undef TWO_CHAR_TOKEN

    if (is_single_char_token) {
        token->span = span_for_single_char(lexer->file_path, lexer->cur_line, lexer->cur_col);
        token->lexeme = nmstring_new_from_char(*lexer->cur_char);
    }

    LEXER_ADVANCE(1);

    return token;
}

void debug_lexical_token(LexicalToken *token) {
    fprintf(stderr, "LexicalToken {\n");
    fprintf(stderr, "\tkind: %d,\n", token->kind);
    if (token->lexeme) fprintf(stderr, "\tlexeme: \"%s\",\n", S(token->lexeme));
    else fprintf(stderr, "\tlexeme: (null),\n");
    fprintf(stderr, "\tspan: %s:%zu:%zu-%zu:%zu\n",
        token->span.file_path,
        token->span.line_start,
        token->span.linepos_start,
        token->span.line_end,
        token->span.linepos_end);
    fprintf(stderr, "}\n");
}
