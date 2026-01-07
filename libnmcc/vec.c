//
// Created by MD Gaziur Rahman Noor on 7/1/26.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nmcc/nmvec.h>

#include "nmcc/nmmust.h"

size_t NMVEC_GROWTH_FACTOR = 2;

NMVec *nmvec_new(size_t item_size) {
    NMVec *vec = calloc(1, sizeof(NMVec));
    NOT_NULL(vec, "failed to allocate NMVec");

    vec->item_size = item_size;

    return vec;
}

NMVec *nmvec_with_capacity(size_t item_size, size_t capacity) {
    NMVec *vec = calloc(1, sizeof(NMVec));
    NOT_NULL(vec, "failed to allocate NMVec");

    vec->data = calloc(capacity, item_size);
    NOT_NULL(vec->data, "failed to allocate NMVec data");
    vec->item_size = item_size;
    vec->capacity = capacity;

    return vec;
}

void nmvec_free(NMVec *this) {
    free(this->data);
    free(this);
}

size_t nmvec_size(const NMVec *this) {
    return this->size;
}

void nmvec_grow(NMVec *this) {
    if (this->capacity == 0) {
        this->capacity = 1;
    } else {
        this->capacity *= NMVEC_GROWTH_FACTOR;
    }

    this->data = realloc(this->data, this->capacity * this->item_size);
    NOT_NULL(this->data, "failed to reallocate NMVec data");
}

void nmvec_push(NMVec *this, void *data) {
    if (this->size == this->capacity) {
        nmvec_grow(this);
    }

    memcpy(this->data + (this->size * this->item_size), data, this->item_size);
    this->size++;
}

void *nmvec_pop(NMVec *this) {
    if (this->size == 0) {
        return NULL;
    }

    this->size--;
    return this->data + (this->size * this->item_size);
}

void *nmvec_get_buf(NMVec *this) {
    return this->data;
}

void *nmvec_at(NMVec *this, size_t idx) {
    return this->data + idx * this->item_size;
}
