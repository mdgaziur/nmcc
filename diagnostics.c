//
// Created by MD Gaziur Rahman Noor on 24/12/25.
//

#include <stdlib.h>
#include <nmcc/nmdiagnostics.h>

#include "nmcc/nmcolors.h"
#include "nmcc/nmdebug.h"
#include "nmcc/nmmust.h"

#define P(...) fprintf(stderr, __VA_ARGS__);

void print_diagnostic(Diagnostic *diagnostic) {
    if (diagnostic->kind == DIAG_WARNING) {
        P(DIAG_CWARN "warning: " DIAG_CINFO "%s\n" ANSI_RESET, S(diagnostic->msg));
    } else if (diagnostic->kind == DIAG_ERROR) {
        P(DIAG_CERROR "warning: " DIAG_CINFO "%s\n" ANSI_RESET, S(diagnostic->msg));
    }

    P(DIAG_CINFO "--> %s:%zu:%zu\n" ANSI_RESET,
        diagnostic->file->path,
        diagnostic->span.line_start,
        diagnostic->span.linepos_start + 1);
}

Diagnostic *allocate_diagnostic() {
    /*
     * We use calloc here to automatically set `n_corrections` and `n_extra` to 0.
     */
    Diagnostic *d = calloc(1, sizeof(Diagnostic));
    NOT_NULL(d, "Failed to allocate memory for Diagnostic");

    return d;
}

Diagnostic *diagnostic_for_span(DiagKind diag_kind, const char *msg, NMFile *file, Span *span) {
    Diagnostic *d = allocate_diagnostic();

    d->kind = diag_kind;
    d->file = file;
    d->span = *span;
    d->msg = nmstring_new_from_str(msg);

    return d;
}

Diagnostic *diagnostic_for_single_char(DiagKind diag_kind, NMFile *file, const char *msg, size_t line, size_t col) {
    /*
     * We use calloc here to automatically set `n_corrections` and `n_extra` to 0.
     */
    Diagnostic *d = allocate_diagnostic();

    d->kind = diag_kind;
    d->file = file;
    d->span.file_path = file->path;
    d->span.line_start = line;
    d->span.linepos_start = col;
    d->span.line_end = line;
    d->span.linepos_end = col;
    d->msg = nmstring_new_from_str(msg);

    return d;
}
