//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#ifndef PREPROCESS_H
#define PREPROCESS_H
#include "nmfile.h"
#include "nmstring.h"

void preprocess1_add_include_directory(NMString *path);
void preprocess1_code(NMFile *src_code, NMString *preprocessed_code);

#endif // PREPROCESS_H
