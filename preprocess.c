//
// Created by MD Gaziur Rahman Noor on 7/1/26.
//

#include <nmcc/preprocess.h>

#include "nmcc/nmdebug.h"
#include "nmcc/nmlex.h"
#include "nmcc/nmmust.h"

#include <stdlib.h>

typedef struct {
  NMString *include_directories;
  size_t n_include_directories;
} PreprocessorState;

PreprocessorState preprocessor_state = {
    .include_directories = NULL,
    .n_include_directories = 0,
};

void preprocess_add_include_directory(NMString *path) {
  preprocessor_state.include_directories = realloc(
      preprocessor_state.include_directories,
      (preprocessor_state.n_include_directories + 1) * sizeof(NMString));
  NOT_NULL(preprocessor_state.include_directories,
           "Failed to allocate buffer to store include directories");

  preprocessor_state
      .include_directories[preprocessor_state.n_include_directories] = *path;

  preprocessor_state.n_include_directories++;
}

NMFile *try_resolve_include(NMString *path) {
  NMFile *src_file = nmfile_open(S(path));
  if (!src_file->has_error) {
    return src_file;
  }

  nmfile_close(src_file);

  size_t i;
  for (i = 0; i < preprocessor_state.n_include_directories; i++) {
    NMString *cur_path =
        nmstring_new_from_str(S(&preprocessor_state.include_directories[i]));
    nmstring_append(cur_path, '/');
    nmstring_append_nmstring(cur_path, path);

    src_file = nmfile_open(S(cur_path));
    if (!src_file->has_error) {
      return src_file;
    }
    nmfile_close(src_file);
  }

  return NULL;
}

NMString *construct_line_directive(const char *file_path, size_t line_num) {
#define LINE_NUM_MAX_SZ 21
  char line_num_str[LINE_NUM_MAX_SZ];
  NMString *line_directive = nmstring_new_from_str("#line ");
  snprintf(line_num_str, LINE_NUM_MAX_SZ, "%zu", line_num);
  nmstring_append_buf(line_directive, line_num_str);
  nmstring_append_buf(line_directive, " \"");
  nmstring_append_buf(line_directive, file_path);
  nmstring_append_buf(line_directive, "\"\n");
#undef LINE_NUM_MAX_SZ
  return line_directive;
}

void skip_whitespace(Lexer *lexer, NMVec *preprocessor_diagnostics) {
  LexicalToken *cmp_token = lex_next(lexer, preprocessor_diagnostics);
  while (cmp_token->kind == LEX_SPACE || cmp_token->kind == LEX_TAB) {
    cmp_token = lex_next(lexer, preprocessor_diagnostics);
  }
}

void handle_token(Lexer *lexer, LexicalToken *token,
                  NMString *preprocessed_code,
                  NMVec *preprocessor_diagnostics) {
  Span start_span = token->span;
  switch (token->kind) {
    case LEX_HASH:
      LexicalToken *ident = lex_next(lexer, preprocessor_diagnostics);
      if (ident->kind == LEX_IDENT) {
        if (NM_EQ_C(ident->lexeme, "include")) {
          NMString *include_path = nmstring_new();
          NMFile *inc;
          NMString *inc_content;
          NMString *line_directive_start;
          NMString *line_directive_continue;

          skip_whitespace(lexer, preprocessor_diagnostics);
          LexicalToken *cmp_token = lex_next(lexer, preprocessor_diagnostics);
          if (!NM_EQ_C(cmp_token->lexeme, "<")) {
            Diagnostic *d = diagnostic_for_span(DIAG_ERROR, "unexpected token",
                                                lexer->file, &cmp_token->span);
            nmvec_push(preprocessor_diagnostics, &d);
            nmstring_append_nmstring(preprocessed_code, cmp_token->lexeme);
            lexical_token_free(cmp_token);
            goto end;
          }

          LexicalToken *include_token =
              lex_next(lexer, preprocessor_diagnostics);
          while (include_token->kind != LEX_EOF &&
                 include_token->kind != LEX_NEWLINE &&
                 include_token->kind != LEX_GREATER) {
            nmstring_append_nmstring(include_path, include_token->lexeme);
            lexical_token_free(include_token);
            include_token = lex_next(lexer, preprocessor_diagnostics);
          }

          if (include_token->kind != LEX_GREATER) {
            Diagnostic *d = diagnostic_for_span(
                DIAG_ERROR, "expected `>`", lexer->file, &include_token->span);
            nmvec_push(preprocessor_diagnostics, &d);
            nmstring_append_nmstring(preprocessed_code, include_token->lexeme);
            goto end;
          }

          inc = try_resolve_include(include_path);
          if (!inc) {
            Span s = span_merge(&start_span, &include_token->span);
            lexical_token_free(include_token);
            Diagnostic *d = diagnostic_for_span(
                DIAG_ERROR, "Failed to open file to include", lexer->file, &s);
            nmvec_push(preprocessor_diagnostics, &d);
            goto end;
          }
          lexical_token_free(include_token);

          inc_content = nmfile_read_to_string(inc);

          line_directive_start = construct_line_directive(S(include_path), 1);
          line_directive_continue =
              construct_line_directive(lexer->file_path, lexer->cur_line + 1);

          nmstring_append_nmstring(preprocessed_code, line_directive_start);
          nmstring_append_nmstring(preprocessed_code, inc_content);
          nmstring_append_nmstring(preprocessed_code, line_directive_continue);

          nmstring_free(line_directive_start);
          nmstring_free(line_directive_continue);
          nmfile_close(inc);
          nmstring_free(inc_content);

        end:
          nmstring_free(include_path);
        } else {
          // TODO
        }
      } else {
        nmstring_append_nmstring(preprocessed_code, token->lexeme);
        nmstring_append_nmstring(preprocessed_code, ident->lexeme);

        lexical_token_free(token);
      }
      break;
    case LEX_IDENT:
      // maybe handle #define subst
      break;
    case LEX_EOF:
      break;
    default:
      nmstring_append_nmstring(preprocessed_code, token->lexeme);
  }
}

bool preprocess_code(NMFile *src_code, NMString *preprocessed_code) {
  Lexer *lexer = lexer_new(src_code, true);

  NMVec *lexer_diagnostics = nmvec_new(sizeof(Diagnostic *));
  NMVec *preprocessor_diagnostics = nmvec_new(sizeof(Diagnostic *));
  LexicalToken *token = lex_next(lexer, lexer_diagnostics);

  while (true) {
    if (token) {
      LexKind token_kind = token->kind;

      handle_token(lexer, token, preprocessed_code, preprocessor_diagnostics);

      lexical_token_free(token);
      if (token_kind == LEX_EOF)
        break;
    }

    token = lex_next(lexer, lexer_diagnostics);
  }

  {
    size_t i;
    for (i = 0; i < lexer_diagnostics->size; i++) {
      print_diagnostic(V(lexer_diagnostics, Diagnostic *, i));
    }
  }

  {
    size_t i;
    for (i = 0; i < preprocessor_diagnostics->size; i++) {
      print_diagnostic(V(preprocessor_diagnostics, Diagnostic *, i));
    }
  }

  bool has_error =
      lexer_diagnostics->size > 0 || preprocessor_diagnostics->size > 0;

  while (lexer_diagnostics->size > 0)
    diagnostic_free(*(Diagnostic **)nmvec_pop(lexer_diagnostics));
  nmvec_free(lexer_diagnostics);

  while (preprocessor_diagnostics->size > 0)
    diagnostic_free(*(Diagnostic **)nmvec_pop(preprocessor_diagnostics));
  nmvec_free(preprocessor_diagnostics);
  lexer_free(lexer);

  return has_error;
}
