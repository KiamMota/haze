#ifndef FILE_H
#define FILE_H

#include "Path.h"
#include "RawBuffer.h"
#include "Result.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

typedef enum {
  FILE_MODE_READ,
  FILE_MODE_WRITE,
  FILE_MODE_APPEND,
  FILE_MODE_READ_UPDATE,
  FILE_MODE_WRITE_UPDATE,
  FILE_MODE_APPEND_UPDATE
} FileMode;

typedef struct {
  uint64_t size;
  time_t created_at;
  time_t modified_at;
  time_t accessed_at;

  bool is_file;
  bool is_directory;

} FileStat;
typedef struct {
  FILE *handle;
  FileStat stat;

} File;

FileStat FileStatInit(const Path *path);
File *FileOpen(const Path *path, FileMode mode);
void FileClose(File **file);
RawBuffer* FileRead(const File* file, int offset);
int FileWrite(const File *file, RawBuffer* buffer);
char* FileReadStr(const File *file, int offset);
int FileWriteStr(const File* file, const char* str);
int FileSeek(File *file, long offset);
int FileTell(File *file, long *position);

bool FileIsNullOrEmpty(const File* f);
bool FileExists(const Path *path);
bool FileIsDir(File* file);
Result FileDelete(const Path *path);
Result FileStatGet(const Path *path, FileStat *stat);

#endif
