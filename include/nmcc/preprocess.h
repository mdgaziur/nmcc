//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#ifndef PREPROCESS_H
#define PREPROCESS_H
#include "nmfile.h"
#include "nmstring.h"

/*
  This function takes ownership of `path` and uses the path string to add
  an include directory for the preprocessor to search for include files.
*/
void preprocess_add_include_directory(NMString *path);
bool preprocess_code(NMFile *src_code, NMString *preprocessed_code);

#endif // PREPROCESS_H
