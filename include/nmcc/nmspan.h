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

Span span_for_single_char(const char *file_path, size_t line_start,
                          size_t linepos);
Span span_new(const char *file_path, size_t line_start, size_t linepos_start,
              size_t line_end, size_t linepos_end);

// Merge span where s1 < s2 and both have the same source file
Span span_merge(Span *s1, Span *s2);

#endif // NMSPAN_H
