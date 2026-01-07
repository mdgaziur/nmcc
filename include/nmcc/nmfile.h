//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#ifndef NMFILE_H
#define NMFILE_H

#include <stdbool.h>
#include <stdio.h>
#include <sys/syslimits.h>

#include "nmstring.h"

typedef struct {
  FILE *f;
  char path[PATH_MAX];
  bool has_error;
} NMFile;

NMFile *nmfile_open(const char *filename);
void nmfile_reset_pos(NMFile *nmfile);
FILE *nmfile_inner(const NMFile *file);
void nmfile_exit_if_error(const NMFile *file);
size_t nmfile_get_size(NMFile *);
NMString *nmfile_read_to_string(NMFile *);
void nmfile_close(NMFile *);

#endif // NMFILE_H
