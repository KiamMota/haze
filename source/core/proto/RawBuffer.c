#include "RawBuffer.h"
#include "HazeMacros.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

RawBuffer *RawBufferNew(void *data, size_t len) {
  RawBuffer *rw_buffr = malloc(sizeof(RawBuffer));
  rw_buffr->data = malloc(sizeof(len));
  rw_buffr->len = len;
  return rw_buffr;
}

void RawBufferFree(RawBuffer **b) {
  PTR_FREE_ASSERT(b);
  free((*b)->data);
  (*b)->len = 0;
  free(*b);
  *b = NULL;
}

const void *RawBufferData(RawBuffer *b) {
  if (!b)
    return NULL;
  return b->data;
}
size_t RawBufferLen(RawBuffer *b) {
  if (!b)
    return 0;
  return b->len;
}

bool RawBufferSetData(RawBuffer *b, void *data, size_t len) {
  if (!b) return false;
  void *new_data = realloc(b->data, len);
  if (!new_data && len > 0) {
    return false; 
  }

  // Agora que o espaço está garantido, copiamos os bytes do dado de origem
  if (data && len > 0) {
    memcpy(new_data, data, len);
  }

  b->data = new_data;
  b->len = len;
  return true;
}

bool RawBufferClear(RawBuffer *b) {
  if (!b || !b->data || b->len == 0) {
    return false;
  }

  memset(b->data, 0, b->len);
  return true;
}
