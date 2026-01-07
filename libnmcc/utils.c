//
// Created by MD Gaziur Rahman Noor on 24/12/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nmcc/nmmust.h>
#include <nmcc/nmstring.h>

// ASSUMPTION: the path must be a path to a file!

NMString *get_dirname(const char *absolute_path) {
  const char *last_slash = strrchr(absolute_path, '/');
  char *new_path;
  size_t len;
  if (last_slash == absolute_path) {
    new_path = strdup("/");
    len = 1;
  } else {
    len = last_slash - absolute_path;
    new_path = malloc(sizeof(char) * (len + 1));
    NOT_NULL(new_path, "Failed to allocate memory for `get_dirname` path");
    memcpy(new_path, absolute_path, len);
    new_path[len] = '\0';
  }

  // FIXME: do not manually construct NMString here
  NMString *final_path = malloc(sizeof(NMString));
  NOT_NULL(final_path,
           "Failed to allocate memory for `get_dirname` path(NMString)");

  final_path->buf = new_path;
  final_path->size = len;

  return final_path;
}
