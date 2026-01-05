//
// Created by MD Gaziur Rahman Noor on 24/12/25.
//

#ifndef NMDEBUG_H
#define NMDEBUG_H

#ifdef DEBUG
#define NMDEBUG(...) fprintf(stderr, "[Debug] " __VA_ARGS__); fflush(stdout);
#else
#define NMDEBUG(...)
#endif

#ifdef DEBUG
#define NMTODO(...) fprintf(stderr, "[Todo] " __VA_ARGS__); fflush(stdout);
#else
#define NMTODO(...) fprintf(stderr, "[Todo] " __VA_ARGS__); fflush(stdout); exit(EXIT_FAILURE);
#endif

#endif //NMDEBUG_H
