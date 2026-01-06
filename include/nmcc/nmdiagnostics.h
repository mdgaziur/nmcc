//
// Created by MD Gaziur Rahman Noor on 24/12/25.
//

#ifndef NMDIAGNOSTICS_H
#define NMDIAGNOSTICS_H

#include "nmcolors.h"
#include "nmfile.h"
#include "nmspan.h"
#include "nmstring.h"

// TODO: make it create diagnostic once we finish implementing diagnostics handling
#define __STRINGIFY_INNER(x) #x
#define STRINGIFY(x) __STRINGIFY_INNER(x)
#define ERR(...) fprintf(stderr, DIAG_CERROR "error at line: " STRINGIFY(__LINE__) ", file: " __FILE__ ": " ANSI_RESET __VA_ARGS__); fprintf(stderr, "\n");

typedef struct {
    Span span;

    NMString *correction;
} Correction;

typedef enum {
    DIAG_EXTRA_HINT,
    DIAG_EXTRA_NOTE,
} ExtraKind;

typedef struct {
    ExtraKind kind;
    NMString *snippet;

    Span span;
} Extra;

typedef enum {
    DIAG_WARNING,
    DIAG_ERROR,
    DIAG_INTERNAL,
} DiagKind;

typedef struct {
    DiagKind kind;

    NMString *msg;
    Span span;

    NMFile *file;

    Correction *corrections;
    size_t n_corrections;

    Extra *extra;
    size_t n_extra;
} Diagnostic;

Diagnostic *diagnostic_for_span(DiagKind diag_kind, const char *msg, NMFile *file, Span *span);
Diagnostic *diagnostic_for_single_char(DiagKind diag_kind, NMFile *file, const char *msg, size_t line, size_t col);
void print_diagnostic(Diagnostic *diagnostic);
void diagnostic_free(Diagnostic *diagnostic);

#endif //NMDIAGNOSTICS_H
