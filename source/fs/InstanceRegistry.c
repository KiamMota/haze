#include "InstanceRegistry.h"
#include "HazeMacros.h"
#include "RawBuffer.h"
#include "Result.h"
#include "fs/Paths.h"
#include "mpack/mpack-writer.h"
#include "session/Session.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

static int haze_process_id(void) {
#ifdef _WIN32
  return (int)GetCurrentProcessId();
#else
  return (int)getpid();
#endif
}

InstanceReg *InstanceRegInstance = NULL;

static RawBuffer *_InstanceRegistryWrite(InstanceReg *reg) {
  unsigned char buffer[255];

  mpack_writer_t writer;
  mpack_writer_init(&writer, buffer, sizeof(buffer));
  mpack_write_u64(&writer, reg->pid);
  mpack_write_u16(&writer, reg->port);
  mpack_write_cstr(&writer, reg->session_name);
  mpack_write_u64(&writer, reg->created_at);
  mpack_finish_map(&writer);

  if (mpack_writer_error(&writer) != mpack_ok) {
    mpack_writer_destroy(&writer);
    return NULL;
  }

  size_t len = mpack_writer_buffer_used(&writer);

  RawBuffer *result = RawBufferNew(buffer, len);

  mpack_writer_destroy(&writer);

  return result;
}

InstanceReg *InstanceRegistryNew(Session *inst, Paths *pt,
                                 unsigned short port) {
  InstanceReg *iReg = malloc(sizeof(InstanceReg));
  if (!iReg)
    return NULL;

  iReg->pid = haze_process_id();
  iReg->session_name = strdup(SessionGetName(inst));
  iReg->port = port;
  iReg->created_at = SessionGetCreatedAt(inst);

  char port_str[16];
  snprintf(port_str, sizeof(port_str), "%hu", port);

  iReg->path = PathsJoin(PathsGet(pt, PATHS_INSTANCES), port_str, NULL);

  iReg->handle = fopen(iReg->path, "wb");

  if (!iReg->handle) {
    free(iReg);
    return NULL;
  }

  RawBuffer *content = _InstanceRegistryWrite(iReg);
  if (!content) {
    fclose(iReg->handle);
    free(iReg->session_name);
    free(iReg);
    return NULL;
  }
  fwrite(RawBufferData(content), 1, RawBufferLen(content), iReg->handle);
  fflush(iReg->handle);
  RawBufferFree(&content);
  return iReg;
}

void InstanceRegistryFree(InstanceReg **iReg_ptr) {
  if (!iReg_ptr || !*iReg_ptr) return;

  InstanceReg *iReg = *iReg_ptr;

  if (iReg->handle) {
    fclose(iReg->handle);
    iReg->handle = NULL;
  }

  if (iReg->session_name) {
    free(iReg->session_name);
    iReg->session_name = NULL;
  }

  if (iReg->path) {
    free((void *)iReg->path);
    iReg->path = NULL;
  }

  free(iReg);
  *iReg_ptr = NULL;
}

Result InstanceRegistryRemove(InstanceReg *r) {
  if (!r)
    return ResultErr("deallocated struct");

  if (r->handle) {
    if (fclose(r->handle) != 0)
      return ResultErr("failed to close registry file");

    r->handle = NULL;
  }

  if (r->path) {
    if (remove(r->path) != 0)
      return ResultErr("failed to remove registry file");
  }

  return ResultOk();
}

unsigned short InstanceRegistryGetLastPort(void) {
  const char *dirPath = PathsGet(PathsInstance, PATHS_INSTANCES);
  if (dirPath == NULL) {
    return 7191; // Retorna 7191 para que o main() some +1 e vire 7192
  }

  unsigned short maxPort = 0;

#ifdef _WIN32
  WIN32_FIND_DATAA findData;
  char searchPath[MAX_PATH];
  snprintf(searchPath, sizeof(searchPath), "%s\\*", dirPath);

  HANDLE hFind = FindFirstFileA(searchPath, &findData);
  if (hFind == INVALID_HANDLE_VALUE) {
    return 7191; // Alterado de 0 para 7191
  }

  do {
    if (strcmp(findData.cFileName, ".") != 0 &&
        strcmp(findData.cFileName, "..") != 0) {
      unsigned long portVal = strtoul(findData.cFileName, NULL, 10);
      if (portVal > 0 && portVal <= 65535) {
        if (portVal > maxPort) {
          maxPort = (unsigned short)portVal;
        }
      }
    }
  } while (FindNextFileA(hFind, &findData) != 0);
  FindClose(hFind);

#else
  DIR *dir = opendir(dirPath);
  if (dir == NULL) {
    return 7191; // Alterado de 0 para 7191
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      unsigned long portVal = strtoul(entry->d_name, NULL, 10);
      if (portVal > 0 && portVal <= 65535) {
        if (portVal > maxPort) {
          maxPort = (unsigned short)portVal;
        }
      }
    }
  }
  closedir(dir);
#endif

  // Se continuar 0, devolve 7191. Senão, devolve a maior porta encontrada.
  return (maxPort == 0) ? 7191 : maxPort;
}
