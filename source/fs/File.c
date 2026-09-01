#include "File.h"
#include "Result.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct File {
    FILE *handle;
};

Result FileOpen(File **file, const char *path, const char *mode) {
    if (!file || !path || !mode)
        return ResultErr("invalid argument");

    FILE *handle = fopen(path, mode);

    if (!handle)
        return ResultErr(strerror(errno));

    File *result = malloc(sizeof(File));

    if (!result) {
        int error = errno;
        fclose(handle);
        return ResultErr(strerror(error));
    }

    result->handle = handle;
    *file = result;

    return ResultOk();
}

Result FileClose(File **file) {
    if (!file || !*file)
        return ResultErr("invalid file");

    if (fclose((*file)->handle) != 0)
        return ResultErr(strerror(errno));

    free(*file);
    *file = NULL;

    return ResultOk();
}

Result FileRead(File *file, void *buffer, size_t size) {
    if (!file || !file->handle || !buffer)
        return ResultErr("invalid argument");

    size_t read = fread(buffer, 1, size, file->handle);

    if (read != size && ferror(file->handle))
        return ResultErr(strerror(errno));

    return ResultOk();
}

Result FileWrite(File *file, const void *buffer, size_t size) {
    if (!file || !file->handle || !buffer)
        return ResultErr("invalid argument");

    size_t written = fwrite(buffer, 1, size, file->handle);

    if (written != size)
        return ResultErr(strerror(errno));

    return ResultOk();
}

Result FileSeek(File *file, long offset) {
    if (!file || !file->handle)
        return ResultErr("invalid file");

    if (fseek(file->handle, offset, SEEK_SET) != 0)
        return ResultErr(strerror(errno));

    return ResultOk();
}

Result FileTell(File *file, long *position) {
    if (!file || !file->handle || !position)
        return ResultErr("invalid argument");

    long result = ftell(file->handle);

    if (result < 0)
        return ResultErr(strerror(errno));

    *position = result;

    return ResultOk();
}

bool FileExists(const char *path) {
    if (!path)
        return false;

    FILE *file = fopen(path, "r");

    if (!file)
        return false;

    fclose(file);
    return true;
}

Result FileDelete(const char *path) {
    if (!path)
        return ResultErr("invalid path");

    if (remove(path) != 0)
        return ResultErr(strerror(errno));

    return ResultOk();
}
