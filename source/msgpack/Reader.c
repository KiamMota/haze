#include "Reader.h"
#include "HazeMacros.h"
#include "RawBuffer.h"
#include <stdlib.h>

Reader *ReaderNew(RawBuffer *bf) {
  if (!bf) {
    return NULL;
  }

  Reader *rd = malloc(sizeof(Reader));
  rd->buff = RawBufferNew(RawBufferData(bf), RawBufferLen(bf));
  if (!rd->buff) {
    return NULL;
  }
  rd->ptr = rd->buff->data;
  return rd;
}

int ReaderInit(Reader *r, RawBuffer *buff) {
  if (!buff)
    return 1;
  if (RawBufferLen(buff) <= 1)
    return 1;

  Reader rd = {0};
  rd.buff = RawBufferDup(buff);
  rd.ptr = rd.buff->data;

  *r = rd;
  return 0;
}

void ReaderFree(Reader **r) {
  PTR_FREE_ASSERT(r)
  RawBufferFree(&(*r)->buff);
  free((*r)->ptr);
  free(*r);
  *r = NULL;
}

bool ReaderExpect(Reader *r, MsgPackTag tag) {

}
