//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#include "nmcc/preprocess1.h"

#include "nmcc/nmutils.h"
#include <ctype.h>
#include <libgen.h>
#include <nmcc/nmdebug.h>
#include <nmcc/nmdiagnostics.h>
#include <nmcc/nmfile.h>
#include <nmcc/nmmust.h>
#include <nmcc/nmstring.h>
#include <string.h>

#define MAX_INCLUDE_DEPTH 128

typedef struct IncludeStackItem {
  NMString *path;
  struct IncludeStackItem *next;
} IncludeStackItem;

typedef struct {
  NMString *name;

  bool is_function_like;
  NMString **args;
  size_t n_args;

  NMString *body;
} MacroDefine;

typedef struct {
  bool should_emit;
  bool append_line_to_last_macro_def;

  NMString *include_directories;
  size_t n_include_directories;

  MacroDefine **macros;
  size_t n_macros;
} PreprocessorState;

PreprocessorState preprocessor_state = {
    .should_emit = true,
    .append_line_to_last_macro_def = false,
    .n_include_directories = 0,
    .include_directories = NULL,
};

size_t current_include_depth = 0;

bool should_emit_code() { return preprocessor_state.should_emit; }

NMString *eat_ident(const char **line_ptr) {
  while (**line_ptr && isspace(**line_ptr))
    (*line_ptr)++;
  if (!*line_ptr) {
    ERR("Unexpected eol!");
    exit(EXIT_FAILURE);
  }

  NMString *ident = nmstring_new();
  while (**line_ptr && (isalpha(**line_ptr) || **line_ptr == '_'))
    nmstring_append(ident, *((*line_ptr)++));

  if (ident->size < 1) {
    free(ident);
    return NULL;
  }

  return ident;
}

void skip_whitespace(const char **line_ptr, bool error_on_eol) {
  while (**line_ptr && isspace(**line_ptr))
    (*line_ptr)++;
  if (error_on_eol && !**line_ptr) {
    ERR("Unexpected eol");
    exit(EXIT_FAILURE);
  }
}

NMString *collect_define_macro_body(const char **line_ptr,
                                    bool *has_extra_line) {
  skip_whitespace(line_ptr, false);
  NMString *body = nmstring_new();
  NOT_NULL(body, "Failed to allocate memory for body");
  while (**line_ptr && **line_ptr != '\n') {
    nmstring_append(body, **line_ptr);
    (*line_ptr)++;
  }

  *has_extra_line = false;
  char *last_back_slash = NULL;
  {
    size_t i;
    for (i = 0; i < body->size; i++) {
      if (S(body)[i] == '\\') {
        *has_extra_line = true;
        last_back_slash = &body->buf[i];
      } else if (!isspace(S(body)[i])) {
        *has_extra_line = false;
        last_back_slash = NULL;
      }
    }
  }

  if (last_back_slash) {
    *last_back_slash = '\n';
  }

  return body;
}

void preprocess_define(const char *line_ptr) {
  NMString *name = eat_ident(&line_ptr);
  if (!name) {
    ERR("Expected ident");
    exit(EXIT_FAILURE);
  }

  NMDEBUG("Macro name: `%s`\n", S(name));

  bool is_function_like = *line_ptr == '(';
  line_ptr++;
  NMString **args = NULL;
  size_t n_args = 0;
  if (is_function_like) {
    while (*line_ptr && *line_ptr != ')') {
      args = realloc(args, ++n_args * sizeof(NMString *));
      NOT_NULL(args, "Failed to allocate memory for #define args");

      args[n_args - 1] = eat_ident(&line_ptr);
      if (!args[n_args - 1]) {
        ERR("Unexpected eof");
        exit(EXIT_FAILURE);
      }

      skip_whitespace(&line_ptr, true);

      if (*line_ptr != ',' && *line_ptr != ')') {
        ERR("Expected `,` or `)` after parameter");
        exit(EXIT_FAILURE);
      }

      if (*line_ptr == ')') {
        break;
      }

      line_ptr++;
    }

    if (*line_ptr != ')') {
      ERR("Expected `)` after parameter list");
      exit(EXIT_FAILURE);
    }

    line_ptr++;
  }

#ifdef DEBUG
  {
    size_t i;
    for (i = 0; i < n_args; i++) {
      NMDEBUG("ARG: `%s`\n", S(args[i]));
    }
  }
#endif

  bool has_extra_line;
  NMString *body = collect_define_macro_body(&line_ptr, &has_extra_line);

  MacroDefine *macro_defn = malloc(sizeof(MacroDefine));
  NOT_NULL(macro_defn, "Failed to allocate memory for macro definition");

  macro_defn->name = name;
  macro_defn->body = body;
  macro_defn->is_function_like = is_function_like;
  macro_defn->args = args;
  macro_defn->n_args = n_args;

  NMDEBUG("Macro body: `%s`\n", S(body));

  preprocessor_state.macros =
      realloc(preprocessor_state.macros,
              ++preprocessor_state.n_macros * sizeof(MacroDefine *));
  NOT_NULL(preprocessor_state.macros,
           "Failed to allocate memory for macro definition");
  preprocessor_state.macros[preprocessor_state.n_macros - 1] = macro_defn;
  preprocessor_state.append_line_to_last_macro_def = has_extra_line;
}

void preprocess_include(const char *line_ptr, const char *file_path,
                        NMString *preprocessed_code) {
  if (current_include_depth > MAX_INCLUDE_DEPTH) {
    ERR("Max include depth limit(128) exceeded");
    exit(EXIT_FAILURE);
  }
  current_include_depth++;
  NMDEBUG("Preprocessing `include` directive\n");

  while (*line_ptr && isspace((unsigned char)*line_ptr))
    line_ptr++;

  bool is_system = *line_ptr == '<';
  if (*line_ptr != '<' && *line_ptr != '"') {
    ERR("#include must contain a file path wrapped in either `<>` or `\"\"`");
    exit(EXIT_FAILURE);
  }

  line_ptr++;
  char end_char = is_system ? '>' : '"';
  NMString *path = nmstring_new();

  while (*line_ptr && *line_ptr != end_char)
    nmstring_append(path, *line_ptr++);

  NMDEBUG("Found path: `%s`\n", S(path));

  bool has_found = false;
  if (!is_system) {
    NMString *search_path = get_dirname(file_path);
    nmstring_append(search_path, '/');
    nmstring_append_nmstring(search_path, path);

    NMDEBUG("Attempting to find `%s` in `%s`\n", S(path), S(search_path));
    NMFile *f = nmfile_open(S(search_path));
    if (!f->has_error) {
      preprocess1_code(f, preprocessed_code);
      has_found = true;
    }

    nmstring_free(search_path);
  }

  size_t i;
  for (i = 0; i < preprocessor_state.n_include_directories && !has_found; i++) {
    NMString *search_path =
        nmstring_new_from_str(preprocessor_state.include_directories[i].buf);
    nmstring_append(search_path, '/');
    nmstring_append_nmstring(search_path, path);

    NMDEBUG("Attempting to find `%s` in `%s`\n", S(path), S(search_path));
    NMFile *f = nmfile_open(S(search_path));
    if (!f->has_error) {
      preprocess1_code(f, preprocessed_code);
      has_found = true;
    }

    nmstring_free(search_path);
  }

  if (!has_found) {
    ERR("Failed to #include path `%s`(`%s`)", S(path), file_path);
  }

  nmstring_free(path);
  current_include_depth--;
}

// NOTE: this takes the ownership of `path`
void preprocess1_add_include_directory(NMString *path) {
  preprocessor_state.include_directories = realloc(
      preprocessor_state.include_directories,
      (preprocessor_state.n_include_directories + 1) * sizeof(NMString));
  NOT_NULL(preprocessor_state.include_directories,
           "Failed to reallocate buffer to hold include_dirs");

  preprocessor_state
      .include_directories[preprocessor_state.n_include_directories] = *path;
  preprocessor_state.n_include_directories++;
}

void preprocess1_code(NMFile *src, NMString *preprocessed_code) {
  FILE *src_file = nmfile_inner(src);
  NOT_NULL(src_file, "Cannot read from source file");

#ifdef DEBUG
  NMDEBUG("Preprocessing `%s`\n", src->path);
  size_t i;
  for (i = 0; i < preprocessor_state.n_include_directories; i++) {
    NMDEBUG("Include directory: `%s`\n",
            S(&preprocessor_state.include_directories[i]));
  }
#endif
  char *line = NULL;
  size_t len = 0;
  size_t line_num = 0;

  while (getline(&line, &len, src_file) != -1) {
    line_num++;
    const char *ptr = line;
    if (preprocessor_state.append_line_to_last_macro_def) {
      bool has_extra_line;
      NMString *body = collect_define_macro_body(&ptr, &has_extra_line);

      nmstring_append_nmstring(
          preprocessor_state.macros[preprocessor_state.n_macros - 1]->body,
          body);
      preprocessor_state.append_line_to_last_macro_def = has_extra_line;
    } else if (*ptr == '#') {
      ptr++;
      skip_whitespace(&ptr, true);

      NMString *directive = nmstring_new();
      while (*ptr && (isalpha(*ptr) || *ptr == '_'))
        nmstring_append(directive, *ptr++);

      NMDEBUG("Found preprocessing directive: `%s`\n", S(directive));
      NMDEBUG("Line: %s", line);

      if (!strcmp(S(directive), "define")) {
        preprocess_define(ptr);
      } else if (!strcmp(S(directive), "include")) {
        preprocess_include(ptr, src->path, preprocessed_code);
      } else {
        fprintf(stderr, "[Warn] Unknown directive `%s`\n", S(directive));
      }

      nmstring_free(directive);
    } else if (should_emit_code()) {
      size_t i;
      NMString *cur_ident = nmstring_new();
      bool inside_string = false;
      for (i = 0; i < strlen(line); i++) {
        if (line[i] == '"') {
          inside_string = !inside_string;
        }

        if (inside_string) {
          nmstring_append(preprocessed_code, line[i]);
          continue;
        }
        if (isalpha(line[i]) || line[i] == '_') {
          nmstring_append(cur_ident, line[i]);
        } else {
          size_t j;
          bool is_macro = false;
          for (j = 0; j < preprocessor_state.n_macros; j++) {
            if (!strcmp(S(preprocessor_state.macros[j]->name), S(cur_ident))) {
              const MacroDefine *macro_defn = preprocessor_state.macros[j];
              if (macro_defn->is_function_like) {
                const char *ptr = line + i;
                while (*ptr && isspace(*ptr))
                  ptr++;
                if (!*ptr || *ptr != '(') {
                  continue;
                }

                NMString *res_body = nmstring_new_from_str(S(macro_defn->body));
                size_t cur_arg;
                for (cur_arg = 0; cur_arg < macro_defn->n_args; cur_arg++) {
                  NMString *arg_val = nmstring_new();
                  while (*ptr && *ptr != ',') {
                    nmstring_append(arg_val, *ptr++);
                  }

                  if (!*ptr) {
                    ERR("Unexpected EOF");
                    exit(EXIT_FAILURE);
                  }
                  if (*ptr != ',') {
                    ERR("Expected a ',', or ')' after macro argument");
                    exit(EXIT_FAILURE);
                  }

                  nmstring_replace(res_body, S(macro_defn->args[cur_arg]),
                                   S(arg_val));
                }

                if (!*ptr || *ptr != ')') {
                  ERR("Expected a ')' after macro call");
                  exit(EXIT_FAILURE);
                }

                i += ptr - line;
              } else {
                nmstring_append_nmstring(preprocessed_code, macro_defn->body);
                is_macro = true;
              }
            }
          }

          if (!is_macro)
            nmstring_append_nmstring(preprocessed_code, cur_ident);
          nmstring_append(preprocessed_code, line[i]);
          nmstring_clear(cur_ident);
        }
      }
    }
  }

  free(line);
}
