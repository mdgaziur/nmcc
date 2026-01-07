//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#include <stdarg.h>
#include <stdio.h>

void nmcc_perror(const char *fmt, ...) {
  char *data = NULL;
  va_list ap;
  va_start(ap, fmt);
  vasprintf(&data, fmt, ap);
  perror(data);
}
