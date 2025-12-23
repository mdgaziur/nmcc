//
// Created by MD Gaziur Rahman Noor on 20/12/25.
//

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

typedef struct {
    char *buf;
    size_t size;
} NMString;

NMString *nmstring_new();
NMString *nmstring_new_from_str(const char *s);
NMString *nmstring_copy(const NMString *src);
size_t nmstring_length(const NMString *this);
void nmstring_append(NMString *this, const char c);
void nmstring_append_buf(NMString *this, const char *buf);
void nmstring_append_nmstring(NMString *this, NMString *src);
const char *nmstring_get_inner(const NMString *this);
void nmstring_free(NMString*);

#endif //STRING_H
