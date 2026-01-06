//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#include <nmcc/nmerror.h>
#include <nmcc/nmstring.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nmcc/nmmust.h"

static NMString *nmstring_create_with_size(size_t);

NMString *nmstring_new() {
    return nmstring_create_with_size(0);
}

NMString *nmstring_new_from_char(char c) {
    NMString *nmstring = nmstring_create_with_size(1);
    if (nmstring) nmstring->buf[0] = c;
    return nmstring;
}

NMString *nmstring_new_from_str(const char *s) {
    const size_t src_size = strlen(s);
    NMString *nmstring = nmstring_create_with_size(src_size);
    if (nmstring) {
        strcpy(nmstring->buf, s);
        nmstring->size = src_size;
    }

    return nmstring;
}

NMString *nmstring_copy(const NMString *src) {
    NMString *nmstring = nmstring_create_with_size(src->size);
    if (nmstring) {
        strcpy(nmstring->buf, src->buf);
        nmstring->size = src->size;
    }

    return nmstring;
}

size_t nmstring_length(const NMString *this) {
    return this->size;
}

size_t nmstring_count(const NMString *this, char c) {
    size_t count = 0;
    const char *cur = this->buf;
    while (*cur) count += *cur++ == c;
    return count;
}

const char *nmstring_get_inner(const NMString *this) {
    return this->buf;
}

void nmstring_append(NMString *this, const char c) {
    char *old_buf = this->buf;
    this->buf = realloc(this->buf, this->size + 1 + 1);
    if (!this->buf) {
        perror("NMString reallocation failure");
        free(old_buf);
        FATAL();
    }
    this->buf[this->size++] = c;
    this->buf[this->size] = '\0';
}

void nmstring_append_buf(NMString *this, const char *buf) {
    const size_t target_len = strlen(buf);
    char *old_buf = this->buf;
    this->buf = realloc(this->buf, this->size + target_len + 1);
    if (!this->buf) {
        perror("NMString reallocation failure");
        free(old_buf);
        FATAL();
    }
    strcpy(this->buf + this->size, buf);
    this->size = this->size + target_len;
    this->buf[this->size] = '\0';
}

void nmstring_append_nmstring(NMString *this, NMString *src) {
    const size_t src_size = strlen(src->buf);
    char *old_buf = this->buf;
    this->buf = realloc(this->buf, this->size + src_size + 1);
    if (!this->buf) {
        perror("NMString reallocation failure");
        free(old_buf);
        FATAL();
    }
    strcpy(this->buf + this->size, src->buf);
    this->size = this->size + src_size;
    this->buf[this->size] = '\0';
}

void nmstring_free(NMString *this) {
    free(this->buf);
    free(this);
}

void nmstring_clear(NMString *this) {
    free(this->buf);
    this->buf = malloc(1 * sizeof(char));
    NOT_NULL(this->buf, "Failed to allocate NMString internal buffer after clearing!");
    this->size = 0;
}

void nmstring_replace(NMString *this, const char *what, const char *with) {
    NMString *res = nmstring_new();

    NMString *cur = nmstring_new();
    const char *whatptr = what;
    size_t matchsize = 0;
    size_t whatsize = strlen(what);
    size_t i;
    for (i = 0; i < this->size; i++) {
        if (this->buf[i] == *whatptr) {
            nmstring_append(cur, this->buf[i]);
            whatptr++;
            matchsize++;

            if (matchsize == whatsize) {
                nmstring_append_buf(res, with);
                whatptr = what;
                matchsize = 0;
                nmstring_clear(cur);
            }
        } else {
            if (cur->size > 0) nmstring_append_nmstring(res, cur);
            nmstring_clear(cur);

            if (this->buf[i] == *what) {
                i--;
            } else {
                nmstring_append(res, this->buf[i]);
            }

            matchsize = 0;
            whatptr = what;
        }
    }

    if (cur->size > 0) nmstring_append_nmstring(res, cur);

    free(this->buf);
    this->buf = res->buf;
    this->size = res->size;
    free(res);
}

static NMString *nmstring_create_with_size(const size_t size) {
    NMString *nmstring = malloc(sizeof(NMString));
    if (nmstring) {
        nmstring->buf = malloc((size + 1) * sizeof(char));
        if (nmstring->buf) {
            nmstring->buf[size] = '\0';
            nmstring->size = size;
        } else {
            perror("NMString allocation failure");
            free(nmstring);
            return NULL;
        }
    }

    return nmstring;
}
