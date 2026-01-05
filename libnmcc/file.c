//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#include <nmcc/nmfile.h>
#include <nmcc/nmerror.h>
#include <stdlib.h>
#include <string.h>

NMFile *nmfile_open(const char *filename) {
    NMFile *file;
    file = (NMFile *)malloc(sizeof(NMFile));
    if (!file) {
        return NULL;
    }

    file->f = fopen(filename, "r");
    file->has_error = !file->f;

    if (!file->has_error) {
        realpath(filename, file->path);
    }

    return file;
}

void nmfile_reset_pos(NMFile *nmfile) {
    fseek(nmfile->f, 0, SEEK_SET);
}

FILE *nmfile_inner(const NMFile *file) {
    return file->f;
}

void nmfile_exit_if_error(const NMFile *file) {
    if (!file->has_error) {
        return;
    }

    nmcc_perror("Failed to open file `%s`", file->path);
    exit(EXIT_FAILURE);
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

    nmfile_reset_pos(file);
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
    fclose(file->f);
    free(file);
}
