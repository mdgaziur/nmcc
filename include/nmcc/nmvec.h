//
// Created by MD Gaziur Rahman Noor on 7/1/26.
//

#ifndef NMVEC_H
#define NMVEC_H

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
    size_t item_size;
} NMVec;

extern size_t NMVEC_GROWTH_FACTOR;

NMVec *nmvec_new(size_t item_size);
NMVec *nmvec_with_capacity(size_t item_size, size_t capacity);
void nmvec_free(NMVec *this);

size_t nmvec_size(const NMVec *this);
void nmvec_push(NMVec *this, void *data);
void *nmvec_pop(NMVec *this);
void *nmvec_get_buf(NMVec *this);
void *nmvec_at(NMVec *this, size_t idx);

#endif //NMVEC_H
