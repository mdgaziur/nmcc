//
// Created by MD Gaziur Rahman Noor on 1/1/26.
//

#ifndef NMSPAN_H
#define NMSPAN_H
#include <stdio.h>

typedef struct {
    const char *file_path;

    size_t line_start;
    size_t line_end;

    size_t linepos_start;
    size_t linepos_end;
} Span;

Span span_for_single_char(const char *file_path, size_t line_start, size_t linepos);
Span span_new(const char *file_path, size_t line_start, size_t linepos_start, size_t line_end, size_t linepos_end);

#endif //NMSPAN_H
