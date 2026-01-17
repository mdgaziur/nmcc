//
// Created by MD Gaziur Rahman Noor on 23/12/25.
//

#include <nmcc/nmmust.h>
#include <nmcc/nmstring.h>
#include <nmcc/nmutils.h>
#include <stdio.h>
#include <string.h>

static NMString *nmstring_create_with_capacity(size_t);
static void nmstring_extend_buf(NMString *this, size_t extend_by);

NMString *nmstring_new() {
  NMString *this = nmstring_create_with_capacity(1);
  this->buf[0] = '\0';
  this->size = 0;

  return this;
}

NMString *nmstring_new_from_char(const char c) {
  NMString *nmstring = nmstring_create_with_capacity(1);
  nmstring->buf[0] = c;
  nmstring->buf[1] = '\0';
  nmstring->size = 1;
  return nmstring;
}

NMString *nmstring_new_from_str(const char *s) {
  const size_t src_size = strlen(s);
  NMString *nmstring = nmstring_create_with_capacity(src_size);
  strcpy(nmstring->buf, s);
  nmstring->size = src_size;

  return nmstring;
}

NMString *nmstring_copy(const NMString *src) {
  NMString *nmstring = nmstring_create_with_capacity(src->capacity);
  strcpy(nmstring->buf, src->buf);
  nmstring->size = src->size;

  return nmstring;
}

size_t nmstring_length(const NMString *this) { return this->size; }

size_t nmstring_count(const NMString *this, const char c) {
  size_t count = 0;
  const char *cur = this->buf;
  while (*cur)
    count += *cur++ == c;
  return count;
}

const char *nmstring_get_inner(const NMString *this) { return this->buf; }

void nmstring_append(NMString *this, const char c) {
  if (this->size + 1 >= this->capacity) {
    nmstring_extend_buf(this, 1);
  }

  this->buf[this->size++] = c;
  this->buf[this->size] = '\0';
}

void nmstring_append_buf(NMString *this, const char *buf) {
  const size_t target_len = strlen(buf);
  if (this->size + target_len >= this->capacity) {
    nmstring_extend_buf(this, target_len);
  }
  strcpy(this->buf + this->size, buf);
  this->size += target_len;
  this->buf[this->size] = '\0';
}

void nmstring_append_nmstring(NMString *this, const NMString *src) {
  if (this->size + src->size >= this->capacity) {
    nmstring_extend_buf(this, src->size);
  }

  strcpy(this->buf + this->size, src->buf);
  this->size += src->size;
  this->buf[this->size] = '\0';
}

void nmstring_free(NMString *this) {
  free(this->buf);
  free(this);
}

void nmstring_clear(NMString *this) {
  this->buf[0] = '\0';
  this->size = 0;
}

void nmstring_replace(NMString *this, const char *what, const char *with) {
  /*
    We use a temporary NMString to create our result string. After the
    replace operation is done, we replace the members of `this` with the
    members of this. Of course, we free any previously allocated memory.
   */
  NMString *res = nmstring_new();
  const char *src_buf = this->buf;
  const char *match;
  size_t what_len = strlen(what);

  // Iterate through all occurrences of `what` in `this`
  while ((match = strstr(src_buf, what)) != NULL) {
    // Append everything before the match
    while (src_buf < match) {
      nmstring_append(res, *src_buf++);
    }

    // Append `with`
    nmstring_append_buf(res, with);

    // Advance the source buffer pointer
    src_buf += what_len;
  }

  // Append the remaining part of the source buffer
  nmstring_append_buf(res, src_buf);

  // Swap `this` with `res`
  const NMString temp = *this;
  *this = *res;
  *res = temp;

  nmstring_free(res);
}

static NMString *nmstring_create_with_capacity(const size_t capacity) {
  NMString *nmstring = malloc(sizeof(NMString));
  NOT_NULL(nmstring, "Failed to allocate memory for NMString struct");

  nmstring->buf = malloc((capacity + 1) * sizeof(char));
  NOT_NULL(nmstring->buf, "Failed to allocate memory for NMString buffer");
  nmstring->capacity = capacity;

  return nmstring;
}

/*
  Extends the internal buffer in such a way that the new capacity is
  aligned to the nearest power of two of current capacity plus `extend_by`.
*/
static void nmstring_extend_buf(NMString *this, const size_t extend_by) {
  this->capacity = align_up_to_power_of_two(this->capacity + extend_by);
  this->buf = realloc(this->buf, this->capacity + 1);
  NOT_NULL(this->buf, "Failed to extend memory for NMString buffer");
}
