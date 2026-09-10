#include "Running.h"
#include "Paths.h"

#include "mpack/mpack-node.h"
#include "mpack/mpack-writer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RUNNING_DEFAULT_PORT 7192

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <process.h>

typedef struct {
    HANDLE handle;
} RunningLock;

static int RunningLockOpen(const char *path, RunningLock *lock)
{
    char lock_path[4096];

    snprintf(lock_path, sizeof(lock_path), "%s.lock", path);

    lock->handle = CreateFileA(
        lock_path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (lock->handle == INVALID_HANDLE_VALUE)
        return -1;

    OVERLAPPED overlapped = {0};

    if (!LockFileEx(
            lock->handle,
            LOCKFILE_EXCLUSIVE_LOCK,
            0,
            MAXDWORD,
            MAXDWORD,
            &overlapped))
    {
        CloseHandle(lock->handle);
        lock->handle = INVALID_HANDLE_VALUE;
        return -1;
    }

    return 0;
}

static void RunningLockClose(RunningLock *lock)
{
    if (!lock || lock->handle == INVALID_HANDLE_VALUE)
        return;

    OVERLAPPED overlapped = {0};

    UnlockFileEx(
        lock->handle,
        0,
        MAXDWORD,
        MAXDWORD,
        &overlapped
    );

    CloseHandle(lock->handle);
    lock->handle = INVALID_HANDLE_VALUE;
}

static int RunningSyncFile(FILE *file)
{
    int fd = _fileno(file);

    if (fd < 0)
        return -1;

    fflush(file);

    HANDLE handle = (HANDLE)_get_osfhandle(fd);

    if (handle == INVALID_HANDLE_VALUE)
        return -1;

    return FlushFileBuffers(handle) ? 0 : -1;
}

static int RunningReplaceFile(
    const char *temp_path,
    const char *path
)
{
    if (!MoveFileExA(
            temp_path,
            path,
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        return -1;

    return 0;
}

static unsigned int RunningPid(void)
{
    return (unsigned int)_getpid();
}

#else

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
    int fd;
} RunningLock;

static int RunningLockOpen(const char *path, RunningLock *lock)
{
    char lock_path[4096];

    snprintf(lock_path, sizeof(lock_path), "%s.lock", path);

    lock->fd = open(
        lock_path,
        O_RDWR | O_CREAT,
        0644
    );

    if (lock->fd < 0)
        return -1;

    if (flock(lock->fd, LOCK_EX) != 0) {
        close(lock->fd);
        lock->fd = -1;
        return -1;
    }

    return 0;
}

static void RunningLockClose(RunningLock *lock)
{
    if (!lock || lock->fd < 0)
        return;

    flock(lock->fd, LOCK_UN);
    close(lock->fd);
    lock->fd = -1;
}

static int RunningSyncFile(FILE *file)
{
    int fd = fileno(file);

    if (fd < 0)
        return -1;

    if (fflush(file) != 0)
        return -1;

    return fsync(fd);
}

static int RunningReplaceFile(
    const char *temp_path,
    const char *path
)
{
    return rename(temp_path, path);
}

static unsigned int RunningPid(void)
{
    return (unsigned int)getpid();
}

#endif

static void WriteRunning(
    mpack_writer_t *writer,
    const Running *r
)
{
    mpack_start_map(writer, 5);

    mpack_write_cstr(writer, "pid");
    mpack_write_u64(writer, r->pid);

    mpack_write_cstr(writer, "session_name");
    mpack_write_cstr(writer, r->session_name);

    mpack_write_cstr(writer, "port");
    mpack_write_u16(writer, r->port);

    mpack_write_cstr(writer, "status");
    mpack_write_int(writer, r->status);

    mpack_write_cstr(writer, "created_at");
    mpack_write_i32(writer, r->created_at);

    mpack_finish_map(writer);
}

Running *RunningNew(
    const char *session_name,
    unsigned short port,
    Status st
)
{
    if (!session_name)
        return NULL;

    Running *r = malloc(sizeof(Running));

    if (!r)
        return NULL;

    r->pid = (uint64_t)RunningPid();
    r->session_name = session_name;
    r->port = port;
    r->status = st;
    r->created_at = (int)time(NULL);

    return r;
}

Running *RunningFree(Running **r)
{
    if (!r || !*r)
        return NULL;

    free(*r);
    *r = NULL;

    return NULL;
}

static void CopyRunning(
    mpack_writer_t *writer,
    mpack_node_t node
)
{
    mpack_node_t pid =
        mpack_node_map_cstr(node, "pid");

    mpack_node_t session =
        mpack_node_map_cstr(node, "session_name");

    mpack_node_t port =
        mpack_node_map_cstr(node, "port");

    mpack_node_t status =
        mpack_node_map_cstr(node, "status");

    mpack_node_t created =
        mpack_node_map_cstr(node, "created_at");

    mpack_start_map(writer, 5);

    mpack_write_cstr(writer, "pid");
    mpack_write_u64(
        writer,
        mpack_node_u64(pid)
    );

    mpack_write_cstr(writer, "session_name");
    // Correção: string no mpack_node_str não tem terminador nulo, use mpack_write_str
    mpack_write_str(
        writer,
        mpack_node_str(session),
        (uint32_t)mpack_node_strlen(session)
    );

    mpack_write_cstr(writer, "port");
    mpack_write_u16(
        writer,
        mpack_node_u16(port)
    );

    mpack_write_cstr(writer, "status");
    mpack_write_int(
        writer,
        mpack_node_int(status)
    );

    mpack_write_cstr(writer, "created_at");
    mpack_write_i32(
        writer,
        mpack_node_i32(created)
    );

    mpack_finish_map(writer);
}

Result RunningWriteFile(Running *r)
{
    if (!r)
        return ResultErr("Running is NULL");

    if (!r->session_name)
        return ResultErr("Session name is NULL");

    const char *path =
        PathsGet(PathsInstance, PATHS_RUNNING);

    if (!path)
        return ResultErr("Running path is NULL");

    RunningLock lock = {0};

#ifdef _WIN32
    lock.handle = INVALID_HANDLE_VALUE;
#else
    lock.fd = -1;
#endif

    if (RunningLockOpen(path, &lock) != 0)
        return ResultErr("Failed to lock running file");

    mpack_tree_t tree;
    mpack_node_t root;

    bool exists = PathsExists(PathsInstance, PATHS_RUNNING);

    // Correção: Trata arquivos vazios gerados por erro anterior
    if (exists) {
        FILE *check_file = fopen(path, "rb");
        if (check_file) {
            fseek(check_file, 0, SEEK_END);
            if (ftell(check_file) == 0) {
                exists = false;
            }
            fclose(check_file);
        } else {
            exists = false;
        }
    }

    size_t count = 0;

    if (exists) {
        mpack_tree_init_filename(
            &tree,
            path,
            0
        );

        if (mpack_tree_error(&tree) != mpack_ok) {
            mpack_tree_destroy(&tree);
            RunningLockClose(&lock);
            return ResultErr("Failed to read running file");
        }

        root = mpack_tree_root(&tree);

        if (mpack_node_type(root) != mpack_type_array) {
            mpack_tree_destroy(&tree);
            RunningLockClose(&lock);
            return ResultErr("Invalid running file");
        }

        count = mpack_node_array_length(root);
    }

    char temp_path[4096];

#ifdef _WIN32
    snprintf(
        temp_path,
        sizeof(temp_path),
        "%s.tmp.%u",
        path,
        RunningPid()
    );
#else
    snprintf(
        temp_path,
        sizeof(temp_path),
        "%s.tmp.%u",
        path,
        RunningPid()
    );
#endif

    FILE *file = fopen(temp_path, "wb");

    if (!file) {
        if (exists)
            mpack_tree_destroy(&tree);

        RunningLockClose(&lock);

        return ResultErr("Failed to create temporary running file");
    }

    mpack_writer_t writer;

    mpack_writer_init_stdfile(
        &writer,
        file,
        true
    );

    mpack_start_array(
        &writer,
        count + 1
    );

    if (exists) {
        for (size_t i = 0; i < count; i++) {
            mpack_node_t node =
                mpack_node_array_at(root, i);

            if (mpack_node_type(node) != mpack_type_map) {
                mpack_writer_destroy(&writer);
                mpack_tree_destroy(&tree);
                remove(temp_path);
                RunningLockClose(&lock);

                return ResultErr(
                    "Invalid Running entry"
                );
            }

            CopyRunning(
                &writer,
                node
            );
        }

        mpack_tree_destroy(&tree);
    }

    WriteRunning(
        &writer,
        r
    );

    mpack_finish_array(&writer);

    if (mpack_writer_error(&writer) != mpack_ok) {
        mpack_writer_destroy(&writer);
        remove(temp_path);
        RunningLockClose(&lock);

        return ResultErr(
            "Failed to write running file"
        );
    }

    // Correção: Esvazia o buffer do mpack para o FILE * antes do sync no SO
    mpack_writer_flush_message(&writer);

    if (RunningSyncFile(file) != 0) {
        mpack_writer_destroy(&writer);
        remove(temp_path);
        RunningLockClose(&lock);

        return ResultErr(
            "Failed to flush running file"
        );
    }

    mpack_writer_destroy(&writer);

    if (RunningReplaceFile(temp_path, path) != 0) {
        remove(temp_path);
        RunningLockClose(&lock);

        return ResultErr(
            "Failed to replace running file"
        );
    }

    RunningLockClose(&lock);

    return ResultOk();
}
unsigned short RunningGetLastPort(void)
{
    const char *path =
        PathsGet(PathsInstance, PATHS_RUNNING);

    if (!path ||
        !PathsExists(
            PathsInstance,
            PATHS_RUNNING))
        return RUNNING_DEFAULT_PORT;

    RunningLock lock = {0};

#ifdef _WIN32
    lock.handle = INVALID_HANDLE_VALUE;
#else
    lock.fd = -1;
#endif

    if (RunningLockOpen(path, &lock) != 0)
        return RUNNING_DEFAULT_PORT;

    mpack_tree_t tree;

    mpack_tree_init_filename(
        &tree,
        path,
        0
    );

    if (mpack_tree_error(&tree) != mpack_ok) {
        mpack_tree_destroy(&tree);
        RunningLockClose(&lock);
        return RUNNING_DEFAULT_PORT;
    }

    mpack_node_t root =
        mpack_tree_root(&tree);

    if (mpack_node_type(root) != mpack_type_array) {
        mpack_tree_destroy(&tree);
        RunningLockClose(&lock);
        return RUNNING_DEFAULT_PORT;
    }

    size_t count =
        mpack_node_array_length(root);

    unsigned short max_port =
        RUNNING_DEFAULT_PORT;

    for (size_t i = 0; i < count; i++) {
        mpack_node_t node =
            mpack_node_array_at(root, i);

        if (mpack_node_type(node) != mpack_type_map)
            continue;

        mpack_node_t port_node =
            mpack_node_map_cstr(node, "port");

        if (mpack_node_type(port_node) != mpack_type_uint)
            continue;

        unsigned short port =
            mpack_node_u16(port_node);

        if (port > max_port)
            max_port = port;
    }

    mpack_tree_destroy(&tree);
    RunningLockClose(&lock);

    return max_port;
}
