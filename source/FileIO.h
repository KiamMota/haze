#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
 FILE* handle;
} File;

File* FileCreate(const char* path) {
  File* f = (File*)malloc(sizeof(File));
  f->handle = fopen(path, "wr");
  return f; 
}

#endif
