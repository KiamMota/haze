#include "Header.h"
#include "RawBuffer.h"
#include "mpack/mpack-expect.h"
#include "mpack/mpack-reader.h"
#include "RawBuffer.h"
#include "RawBuffer.h"
#include "mpack/mpack-writer.h"
#include <stdio.h>
#include <string.h>

RawBuffer* ProjectHeaderCreate(ProjectHeader* header) {
  if (!header) return NULL;

  RawBuffer* headerBuff = RawBufferNew(NULL, 0);

  mpack_writer_t writer;
  mpack_writer_init_growable(&writer, NULL, NULL);
  mpack_write_u64(&writer, header->SchemaVersion);
  mpack_write_u64(&writer, header->HazeVersion);
  mpack_write_cstr(&writer, header->ProjectName);
  mpack_write_cstr(&writer, header->Author);
  // mpack_writeb -> CreatedAt
  mpack_write_cstr(&writer, header->ProjectPath);
}

ProjectHeader ProjectHeaderGet(RawBuffer *buf) {
  if (RawBufferIsNullOrEmpty(buf))
    return (ProjectHeader){0};

  mpack_reader_t reader;
  mpack_reader_init_data(&reader, RawBufferData(buf), RawBufferLen(buf));

  ProjectHeader header = {0};

  if (mpack_reader_error(&reader))
    return (ProjectHeader){0};

  header.SchemaVersion = mpack_expect_u64(&reader);
  header.HazeVersion = mpack_expect_u64(&reader);
  RawBuffer* project_name = RawBufferNew(NULL, 0);
  mpack_expect_cstr(&reader, (char*)RawBufferData(project_name), RawBufferLen(project_name));
  header.ProjectName = strdup(RawBufferData(project_name));
  RawBufferFree(&project_name);
}
