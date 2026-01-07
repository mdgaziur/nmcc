//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#ifndef MUST_H
#define MUST_H

#include <stdlib.h>

#define NOT_NULL(expr, msg)                                                    \
  if (!expr) {                                                                 \
    fprintf(stderr, "NMCC PANIC(%s:%d): %s", __FILE__, __LINE__, msg);         \
    exit(EXIT_FAILURE);                                                        \
  }

#endif // MUST_H
