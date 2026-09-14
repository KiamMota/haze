#ifndef MSGPACK_READER_H
#define MSGPACK_READER_H

#include "RawBuffer.h"
#include "msgpack/Tag.h"
#include <stdbool.h>

typedef struct {
  void* ptr;
  RawBuffer *buff;
} Reader;

Reader *ReaderNew(RawBuffer *buff);
int ReaderInit(Reader* rd, RawBuffer *buff);
void ReaderFree(Reader **r);
bool ReaderExpect(Reader *r, MsgPackTag tag);
MsgPackTag ReaderGetTag(Reader *r);

#endif
