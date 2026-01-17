//
// Created by MD Gaziur Rahman Noor on 24/12/25.
//

#include <_stdio.h>
#include <nmcc/nmfmt.h>
#include <stdarg.h>

#include <stdlib.h>

NMString *fmt(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  char *out;
  vasprintf(&out, fmt, args);
  va_end(args);

  NMString *res = nmstring_new_from_str(out);
  free(out);
  return res;
}
