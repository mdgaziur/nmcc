//
// Created by MD Gaziur Rahman Noor on 7/1/26.
//

#include <nmcc/preprocess.h>

#include "nmcc/nmdebug.h"
#include "nmcc/nmlex.h"
#include "nmcc/nmmust.h"
#include <nmcc/nmutils.h>

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

NMFile *try_resolve_include(NMString *parent_path, NMString *path,
                            bool supports_current_dir) {
  NMFile *src_file;
  if (supports_current_dir) {
    NMString *final_path = nmstring_new_from_str(S(parent_path));
    nmstring_append(final_path, '/');
    nmstring_append_nmstring(final_path, path);
    src_file = nmfile_open(S(final_path));
    nmstring_free(final_path);
    if (!src_file->has_error) {
      return src_file;
    }

    nmfile_close(src_file);
  }

  size_t i;
  for (i = 0; i < preprocessor_state.n_include_directories; i++) {
    NMString *cur_path =
        nmstring_new_from_str(S(&preprocessor_state.include_directories[i]));
    nmstring_append(cur_path, '/');
    nmstring_append_nmstring(cur_path, path);

    src_file = nmfile_open(S(cur_path));
    nmstring_free(cur_path);
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

LexicalToken *skip_whitespace(Lexer *lexer, NMVec *preprocessor_diagnostics) {
  LexicalToken *cmp_token = lex_next(lexer, preprocessor_diagnostics);
  while (cmp_token->kind == LEX_SPACE || cmp_token->kind == LEX_TAB) {
    lexical_token_free(cmp_token);
    cmp_token = lex_next(lexer, preprocessor_diagnostics);
  }
  return cmp_token;
}

void handle_token(Lexer *lexer, LexicalToken *token,
                  NMString *preprocessed_code,
                  NMVec *preprocessor_diagnostics) {
  switch (token->kind) {
    case LEX_HASH: {
      LexicalToken *ident = lex_next(lexer, preprocessor_diagnostics);
      if (ident->kind == LEX_IDENT) {
        if (NM_EQ_C(ident->lexeme, "include")) {
          NMString *include_path = nmstring_new();
          NMFile *inc;
          NMString *inc_content;
          NMString *line_directive_start;
          NMString *line_directive_continue;
          Span final_span;
          NMString *parent_path = get_dirname(lexer->file_path);
          bool supports_current_dir = false;

          LexicalToken *cmp_token =
              skip_whitespace(lexer, preprocessor_diagnostics);
          Span start_span = cmp_token->span;
          if (cmp_token->kind == LEX_LESS) {
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
              Diagnostic *d =
                  diagnostic_for_span(DIAG_ERROR, "expected `>`", lexer->file,
                                      &include_token->span);
              nmvec_push(preprocessor_diagnostics, &d);
              nmstring_append_nmstring(preprocessed_code,
                                       include_token->lexeme);
              goto end;
            }

            final_span = span_merge(&start_span, &include_token->span);
            lexical_token_free(include_token);
          } else if (cmp_token->kind == LEX_STRING) {
            supports_current_dir = true;
            NMString *include_path_from_token =
                nmstring_new_from_str(S(cmp_token->lexeme));
            nmstring_replace(include_path_from_token, "\"", "");
            nmstring_append_nmstring(include_path, include_path_from_token);
            nmstring_free(include_path_from_token);
          } else {
            Diagnostic *d = diagnostic_for_span(DIAG_ERROR, "unexpected token",
                                                lexer->file, &cmp_token->span);
            nmvec_push(preprocessor_diagnostics, &d);
            nmstring_append_nmstring(preprocessed_code, cmp_token->lexeme);
            lexical_token_free(cmp_token);
            goto end;
          }

          inc = try_resolve_include(parent_path, include_path,
                                    supports_current_dir);
          if (!inc) {
            Diagnostic *d = diagnostic_for_span(
                DIAG_ERROR, "Failed to open file to include", lexer->file,
                &final_span);
            nmvec_push(preprocessor_diagnostics, &d);
            goto end;
          }

          inc_content = nmfile_read_to_string(inc);

          line_directive_start = construct_line_directive(S(include_path), 1);
          line_directive_continue =
              construct_line_directive(lexer->file_path, lexer->cur_line + 1);

          nmstring_append_nmstring(preprocessed_code, line_directive_start);
          nmstring_append_nmstring(preprocessed_code, inc_content);
          if (S(preprocessed_code)[preprocessed_code->size - 1] != '\n') {
            nmstring_append(preprocessed_code, '\n');
          }
          nmstring_append_nmstring(preprocessed_code, line_directive_continue);

          nmstring_free(line_directive_start);
          nmstring_free(line_directive_continue);
          nmfile_close(inc);
          nmstring_free(inc_content);
        end:
          lexical_token_free(cmp_token);
          nmstring_free(parent_path);
          nmstring_free(include_path);
        } else {
          // TODO
        }
      } else {
        nmstring_append_nmstring(preprocessed_code, token->lexeme);
        nmstring_append_nmstring(preprocessed_code, ident->lexeme);

        lexical_token_free(token);
      }

      lexical_token_free(ident);
      break;
    }
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
