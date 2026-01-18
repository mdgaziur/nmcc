//
// Created by MD Gaziur Rahman Noor on 1/1/26.
//

#include "nmcc/nmspan.h"

Span span_for_single_char(const char *file_path, size_t line_start,
                          size_t linepos) {
  Span sp = {
      .file_path = file_path,
      .line_start = line_start,
      .line_end = line_start,
      .linepos_start = linepos,
      .linepos_end = linepos,
  };

  return sp;
}

Span span_new(const char *file_path, size_t line_start, size_t linepos_start,
              size_t line_end, size_t linepos_end) {
  Span sp = {
      .file_path = file_path,
      .line_start = line_start,
      .line_end = line_end,
      .linepos_start = linepos_start,
      .linepos_end = linepos_end,
  };

  return sp;
}

Span span_merge(Span *s1, Span *s2) {
  Span new_span = {
      .file_path = s1->file_path,
      .line_start = s1->line_start,
      .line_end = s2->line_end,
      .linepos_start = s1->linepos_start,
      .linepos_end = s2->linepos_end,
  };

  return new_span;
}
