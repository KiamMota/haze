#include "File.h"
#include "Path.h"
#include "RawBuffer.h"
#include "Result.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) ||      \
    defined(__OpenBSD__) || defined(__NetBSD__)
#include <sys/stat.h>
#endif

FileStat FileStatInit(const Path *path) {
  FileStat result = {0};

  if (!path || PathIsNullOrEmpty(path))
    return result;

#if defined(_WIN32) || defined(_WIN64)
  WIN32_FILE_ATTRIBUTE_DATA data;

  if (!GetFileAttributesExA(path->path, GetFileExInfoStandard, &data))
    return result;

  result.size =
      ((uint64_t)data.nFileSizeHigh << 32) | (uint64_t)data.nFileSizeLow;

  result.is_directory = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

  result.is_file = !result.is_directory;

  // Conversão básica de FILETIME para time_t (opcional/aproximada para Windows
  // se necessário) Para manter simples, focamos nos campos principais.

#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) ||      \
    defined(__OpenBSD__) || defined(__NetBSD__)
  struct stat info;

  if (stat(path->path, &info) != 0)
    return result;

  result.size = (uint64_t)info.st_size;
  result.is_file = S_ISREG(info.st_mode);
  result.is_directory = S_ISDIR(info.st_mode);

  result.modified_at = info.st_mtime;
  result.accessed_at = info.st_atime;
#if defined(__APPLE__)
  result.created_at = info.st_birthtime;
#else
  result.created_at = info.st_ctime;
#endif

#endif

  return result;
}

File *FileOpen(const Path *path, FileMode mode) {
  if (!path || !path->path)
    return NULL;

  const char *mode_str;

  switch (mode) {
  case FILE_MODE_READ:
    mode_str = "rb";
    break;
  case FILE_MODE_WRITE:
    mode_str = "wb";
    break;
  case FILE_MODE_APPEND:
    mode_str = "ab";
    break;
  case FILE_MODE_READ_UPDATE:
    mode_str = "rb+";
    break;
  case FILE_MODE_WRITE_UPDATE:
    mode_str = "wb+";
    break;
  case FILE_MODE_APPEND_UPDATE:
    mode_str = "ab+";
    break;
  default:
    return NULL;
  }

  FILE *handle = fopen(path->path, mode_str);
  if (!handle)
    return NULL;

  File *file = malloc(sizeof(File));
  if (!file) {
    fclose(handle);
    return NULL;
  }

  file->handle = handle;
  file->stat = FileStatInit(path);
  return file;
}

void FileClose(File **file) {
  if (!file || !*file)
    return;

  if ((*file)->handle) {
    fclose((*file)->handle);
  }

  free(*file);
  *file = NULL;
}

RawBuffer *FileRead(const File *file, int offset) {
  if (!file || !file->handle)
    return NULL;

  if (fseek(file->handle, offset, SEEK_SET) != 0)
    return NULL;

  long start = ftell(file->handle);
  if (start < 0)
    return NULL;

  if (fseek(file->handle, 0, SEEK_END) != 0)
    return NULL;

  long end = ftell(file->handle);
  if (end < 0 || end < start)
    return NULL;

  size_t size = (size_t)(end - start);

  if (fseek(file->handle, start, SEEK_SET) != 0)
    return NULL;

  RawBuffer *buffer = RawBufferNew(NULL, size);
  if (!buffer)
    return NULL;

  size_t read_bytes = fread(buffer->data, 1, size, file->handle);
  if (read_bytes != size) {
    RawBufferFree(&buffer);
    return NULL;
  }

  buffer->len = read_bytes;
  return buffer;
}

char *FileReadStr(const File *file, int offset) {
  if (!file || !file->handle)
    return NULL;

  if (fseek(file->handle, offset, SEEK_SET) != 0)
    return NULL;

  long start = ftell(file->handle);
  if (start < 0)
    return NULL;

  if (fseek(file->handle, 0, SEEK_END) != 0)
    return NULL;

  long end = ftell(file->handle);
  if (end < 0 || end < start)
    return NULL;

  size_t size = (size_t)(end - start);

  if (fseek(file->handle, start, SEEK_SET) != 0)
    return NULL;

  char *buffer = malloc(size + 1);
  if (!buffer)
    return NULL;

  size_t read_bytes = fread(buffer, 1, size, file->handle);
  buffer[read_bytes] = '\0';

  return buffer;
}

int FileWrite(const File *file, RawBuffer *buffer) {
  if (!FileIsNullOrEmpty(file))
    return -1;
  size_t written =
      fwrite(RawBufferData(buffer), 1, RawBufferLen(buffer), file->handle);

  return written == RawBufferLen(buffer);
}
int FileWriteStr(const File *file, const char *str) {
  if (FileIsNullOrEmpty(file) || str == NULL)
    return 0;

  size_t length = strlen(str);

  return fwrite(str, 1, length, file->handle) == length;
}

bool FileIsNullOrEmpty(const File *f) { return f == NULL || f->handle == NULL; }
int FileSeek(File *file, long offset) {
  if (!file || !file->handle)
    return -1;

  return fseek(file->handle, offset, SEEK_SET);
}

int FileTell(File *file, long *position) {
  if (!file || !file->handle || !position)
    return -1;

  long pos = ftell(file->handle);
  if (pos < 0)
    return -1;

  *position = pos;
  return 0;
}

bool FileExists(const Path *path) {
  if (!path || PathIsNullOrEmpty(path))
    return false;

  FILE *f = fopen(path->path, "rb");
  if (!f) {
    FileStat st = FileStatInit(path);
    return st.is_file || st.is_directory;
  }

  fclose(f);
  return true;
}

bool FileIsDir(File *file) {
  if (!file)
    return false;
  return file->stat.is_directory;
}

Result FileDelete(const Path *path) {
  if (!path || PathIsNullOrEmpty(path))
    return ResultErr("invalid path");

  if (remove(path->path) != 0)
    return ResultErr(strerror(errno));

  return ResultOk();
}

Result FileStatGet(const Path *path, FileStat *stat) {
  if (!path || !stat)
    return ResultErr("invalid argument");

  FileStat st = FileStatInit(path);
  if (!st.is_file && !st.is_directory && st.size == 0) {
    if (!FileExists(path))
      return ResultErr("file does not exist");
  }

  *stat = st;
  return ResultOk();
}
