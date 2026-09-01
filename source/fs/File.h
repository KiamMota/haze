#ifndef FILE_H
#define FILE_H

#include "Result.h"
#include <stdio.h>
typedef struct File File;

Result FileOpen(File **file, const char *path, const char *mode);
Result FileClose(File **file);

Result FileRead(File *file, void *buffer, size_t size);
Result FileWrite(File *file, const void *buffer, size_t size);

Result FileSeek(File *file, long offset);
Result FileTell(File *file, long *position);

bool FileExists(const char *path);
Result FileDelete(const char *path);

#endif
