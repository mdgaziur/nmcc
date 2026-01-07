//
// Created by MD Gaziur Rahman Noor on 24/12/25.
//

#include <_stdio.h>
#include <nmcc/nmfmt.h>
#include <stdarg.h>

#include "../test/preprocessor_include/print_hello.h"

NMString *fmt(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  char *res;
  vasprintf(&res, fmt, args);
  va_end(args);
}
