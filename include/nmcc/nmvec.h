//
// Created by MD Gaziur Rahman Noor on 7/1/26.
//

#ifndef NMVEC_H
#define NMVEC_H

#include <ctype.h>

#define V(vec, type, idx) (*(type *)nmvec_at(vec, idx))

/*
  Dynamically allocated vector that can hold opaque items of fixed size.
  It is recommended to use the `V` macro to access items in the vector as
  accessing items may not be as straightforward as it seems. Check `nmvec_at`
  for more info.
*/
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
/*
  Grows the capacity of the vector `this` by the growth factor defined
  in `NMVEC_GROWTH_FACTOR` if necessary. And after that, it copies the
  data from given address `data` to the end of the vector `this`. It is
  necessary that the caller free the data if it was dynamically allocated.

  One confusion that may arise from this function is that if the type of
  the data held by this vector is `T*`, the caller may simply pass the data as
  is. This will cause undefined behavior as instead of `T*` getting copied,
  data of pointer size will get copied from the given data.

  The correct way to do this is to pass the address of `T*` data, i.e. `&data`.
  Example:
  int *data = malloc(sizeof(int));
  *data = 42;
  nmvec_push(vec, data);  // Incorrect (undefined behavior)
  nmvec_push(vec, &data); // Correct
 */
void nmvec_push(NMVec *this, void *data);
void *nmvec_pop(NMVec *this);
void *nmvec_get_buf(NMVec *this);
/*
  Returns a pointer to the item at index `idx` in the vector `this`. Note that
  the returned pointer is of type `void *` and needs to be cast to the
  appropriate type before use. It is recommended to use the `V` macro for this
  purpose.

  Return value type is the pointer to the type of item stored in the vector.
  For example:
  - If the item is of type `T`, the return type is `T *`.
  - If the item is of type `T*`, the return type is `T**`.
  ...and so on.

  It is important to keep note of this as misuse may lead to undefined behavior
  and type confusion.
*/
void *nmvec_at(NMVec *this, size_t idx);

#endif // NMVEC_H
