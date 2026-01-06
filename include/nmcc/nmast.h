//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#ifndef AST_H
#define AST_H

#include "nmstring.h"
#include "nmspan.h"

typedef enum {
    // Possibly multi character lexical tokens
    LEX_IDENT,
    LEX_CHAR_LIT,
    LEX_STRING,
    LEX_WSTRING,
    LEX_NUMBER_INT,
    LEX_NUMBER_FLOAT,
    LEX_NUMBER_OCT,
    LEX_NUMBER_HEX,

    LEX_NOT, // !
    LEX_NOT_EQUALS, // !=
    LEX_DQUOTE, // "
    LEX_MOD, // %
    LEX_MOD_ASSIGN, // %=
    LEX_AMP, // &
    LEX_AND_ASSIGN, // &=
    LEX_AND, // &&
    LEX_TICK, // `
    LEX_LPAREN, // (
    LEX_RPAREN, // )
    LEX_STAR, // *
    LEX_MUL_ASSIGN, // *=
    LEX_PLUS, // +
    LEX_INC, // ++
    LEX_PLUS_ASSIGN, // +=
    LEX_COMMA, // ,
    LEX_MINUS, // -
    LEX_ARROW, // ->
    LEX_DEC, // --
    LEX_MINUS_ASSIGN, // -=
    LEX_DOT, // .
    LEX_THREE_DOTS, // ...
    LEX_SLASH, // /
    LEX_DIV_ASSIGN, // /=
    LEX_COLON, // :
    LEX_SEMICOLON, // ;
    LEX_LESS, // <
    LEX_LSHIFT, // <<
    LEX_LESS_EQUAL, // <=
    LEX_LSHIFT_ASSIGN, // <<=
    LEX_ASSIGN, // =
    LEX_EQUAL, // ==
    LEX_GREATER, // >
    LEX_GREATER_EQUAL, // >=
    LEX_RSHIFT, // >>
    LEX_RSHIFT_ASSIGN, // >>=
    LEX_QUESTION, // ?
    LEX_LBRACK, // [
    LEX_HASH, // #
    LEX_DHASH, // ##
    LEX_BACKSLASH, // \

    // necessary for preprocessor
    LEX_TAB, // \t
    LEX_SPACE, // <space>
    LEX_NEWLINE, // \n
    LEX_FORMFEED, // \f
    LEX_VERTICAL_TAB, // \v
    LEX_CARRIAGE_RETURN, // \r
    LEX_COMMENT,

    LEX_RBRACK, // ]
    LEX_XOR, // ^
    LEX_XOR_ASSIGN, // ^=
    LEX_UNDERSCORE, // _
    LEX_LBRACE, // {
    LEX_BINARY_OR, // |
    LEX_OR, // ||
    LEX_OR_ASSIGN,
    LEX_RBRACE, // }
    LEX_BINARY_NOT, // ~

    LEX_AUTO,
    LEX_DOUBLE,
    LEX_INT,
    LEX_STRUCT,
    LEX_BREAK,
    LEX_ELSE,
    LEX_LONG,
    LEX_SWITCH,
    LEX_CASE,
    LEX_ENUM,
    LEX_REGISTER,
    LEX_TYPEDEF,
    LEX_CHAR,
    LEX_EXTERN,
    LEX_RETURN,
    LEX_UNION,
    LEX_CONST,
    LEX_FLOAT,
    LEX_SHORT,
    LEX_UNSIGNED,
    LEX_CONTINUE,
    LEX_FOR,
    LEX_SIGNED,
    LEX_VOID,
    LEX_DEFAULT,
    LEX_GOTO,
    LEX_SIZEOF,
    LEX_VOLATILE,
    LEX_DO,
    LEX_IF,
    LEX_STATIC,
    LEX_WHILE,

    LEX_EOF
} LexKind;

typedef struct {
    NMString *lexeme;
    LexKind kind;
    Span span;
} LexicalToken;

void lexical_token_free(LexicalToken *token);

#endif //AST_H
