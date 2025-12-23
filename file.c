//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#include <nmfile.h>
#include <stdlib.h>
#include <string.h>

#include "nmerror.h"

NMFile *nmfile_open(const char *filename) {
    NMFile *file;
    file = (NMFile *)malloc(sizeof(NMFile));
    if (!file) {
        return NULL;
    }

    file->filename = malloc(strlen(filename) + 1);
    if (!file->filename) {
        free(file);
        return NULL;
    }
    strcpy(file->filename, filename);

    file->f = fopen(filename, "r");
    file->has_error = !file->f;

    return file;
}

void nmfile_exit_if_error(const NMFile *file) {
    if (!file->has_error) {
        return;
    }

    nmcc_perror("Failed to open file `%s`", file->filename);
}

NMString *nmfile_read_to_string(NMFile *file) {
    size_t sz = nmfile_get_size(file);
    char *buffer = malloc(sz + 1);
    if (!buffer) {
        return NULL;
    }

    fread(buffer, sz, 1, file->f);
    buffer[sz] = '\0';
    NMString *str = nmstring_new_from_str(buffer);
    free(buffer);
    return str;
}

size_t nmfile_get_size(NMFile *file) {
    size_t sz;
    fseek(file->f, 0, SEEK_END);
    sz = ftell(file->f);
    fseek(file->f, 0, SEEK_SET);

    return sz;
}

void nmfile_close(NMFile *file) {
    free(file->filename);
    fclose(file->f);
    free(file);
}
