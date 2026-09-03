#ifndef PROJHEADER_H
#define PROJHEADER_H

#include "RawBuffer.h"
#include <stdint.h>
typedef struct {
  uint64_t SchemaVersion;
  uint64_t HazeVersion;
  const char* ProjectName;
  const char* Author;
  uint64_t CreatedAt;
  const char* ProjectPath;
} ProjectHeader;

RawBuffer* ProjectHeaderCreate(ProjectHeader* header);
ProjectHeader ProjectHeaderGet(RawBuffer* buf);
uint64_t ProjectHeaderGetSchemaVersion(const ProjectHeader* proj);
uint64_t ProjectHeaderGetHazeVersion(const ProjectHeader* proj);
const char* ProjectHeaderGetProjetName(const ProjectHeader* proj);
const char* ProjectHeaderGetAuthor(const ProjectHeader* proj);
uint64_t ProjectHeaderGetCreatedAt(const ProjectHeader* proj);
const char* ProjectHeaderGetProjectPath(const ProjectHeader* proj);

#endif
