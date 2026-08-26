#include "Response.h"
#include "HazeLog.h"
#include "proto/RawBuffer.h"
#include "mpack/mpack-reader.h"
#include "mpack/mpack.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

Response *ResponseNew(void) {
  Response *response = calloc(1, sizeof(Response));

  if (!response)
    return NULL;

  response->type = HAZE_RPC_RESPONSE;
  response->msgid = 0;
  response->error = RawBufferNew(NULL, 0);
  response->result = RawBufferNew(NULL, 0);

  return response;
}
bool ResponseSetResult(Response *s, void *data, size_t len) {
  if (!s)
    return false;

  return RawBufferSetData(s->result, data, len);
}

RawBuffer *ResponseMarshal(Response *s)
{
    if (!s)
        return NULL;

    char *data = NULL;
    size_t size = 0;

    mpack_writer_t writer;
    mpack_writer_init_growable(&writer, &data, &size);

    mpack_start_array(&writer, 4);

    mpack_write_u8(&writer, HAZE_RPC_RESPONSE);
    mpack_write_u32(&writer, s->msgid);

    if (s->error && RawBufferLen(s->error) > 0) {
        mpack_write_bytes(
            &writer,
            RawBufferData(s->error),
            RawBufferLen(s->error)
        );
    } else {
        mpack_write_nil(&writer);
    }

    if (s->result && RawBufferLen(s->result) > 0) {
        mpack_write_bytes(
            &writer,
            RawBufferData(s->result),
            RawBufferLen(s->result)
        );
    } else {
        mpack_write_nil(&writer);
    }

    mpack_finish_array(&writer);

    mpack_error_t error = mpack_writer_destroy(&writer);

    if (error != mpack_ok) {
        HazeLogError(
            "ResponseMarshal failed: %s",
            mpack_error_to_string(error)
        );

        if (data)
            free(data);

        return NULL;
    }

    RawBuffer *buffer = RawBufferNew(data, size);

    free(data);

    if (!buffer)
        return NULL;

    return buffer;
}

bool ResponseFree(Response **response) {
  if (!response || !*response)
    return false;

  free(*response);
  *response = NULL;
  return true;
}

bool ResponseSetError(Response *s, MpackRPCError err) {
    if (!s)
        return false;

    return RawBufferSetData(
        s->error,
        &err,
        sizeof(err)
    );
}
bool ResponseSetMsgId(Response *s, uint32_t msgid) {
  if (!s)
    return false;
  s->msgid = msgid;
  return true;
}

Response *ResponseUnmarshal(RawBuffer *b)
{
    if (!b || RawBufferLen(b) == 0)
        return NULL;

    mpack_reader_t reader;

    mpack_reader_init_data(
        &reader,
        (const char *)RawBufferData(b),
        RawBufferLen(b)
    );

    Response *response = NULL;

    uint32_t count = mpack_expect_array(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    if (count != 4)
        goto fail;

    response = ResponseNew();

    if (!response)
        goto fail;

    /*
     * 1. type
     */
    response->type = (HazeServerRPCType)mpack_expect_u8(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    if (response->type != HAZE_RPC_RESPONSE)
        goto fail;

    /*
     * 2. msgid
     */
    response->msgid = mpack_expect_u32(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    /*
     * 3. error
     *
     * [nil] -> no error
     * [bin] -> raw error payload
     */
    mpack_tag_t error_tag = mpack_peek_tag(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    if (mpack_tag_type(&error_tag) == mpack_type_nil) {

        mpack_expect_nil(&reader);

        if (mpack_reader_error(&reader) != mpack_ok)
            goto fail;

    } else if (mpack_tag_type(&error_tag) == mpack_type_bin) {

        size_t error_len = mpack_expect_bin(&reader);

        if (mpack_reader_error(&reader) != mpack_ok)
            goto fail;

        if (error_len > 0) {
            void *error_data = malloc(error_len);

            if (!error_data)
                goto fail;

            mpack_read_bytes(
                &reader,
                error_data,
                error_len
            );

            if (mpack_reader_error(&reader) != mpack_ok) {
                free(error_data);
                goto fail;
            }

            mpack_done_bin(&reader);

            if (mpack_reader_error(&reader) != mpack_ok) {
                free(error_data);
                goto fail;
            }

            if (!RawBufferSetData(
                    response->error,
                    error_data,
                    error_len
                )) {
                free(error_data);
                goto fail;
            }

            free(error_data);
        } else {
            mpack_done_bin(&reader);

            if (mpack_reader_error(&reader) != mpack_ok)
                goto fail;
        }

    } else {
        goto fail;
    }

    /*
     * 4. result
     *
     * [nil] -> no result
     * [bin] -> raw result payload
     */
    mpack_tag_t result_tag = mpack_peek_tag(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    if (mpack_tag_type(&result_tag) == mpack_type_nil) {

        mpack_expect_nil(&reader);

        if (mpack_reader_error(&reader) != mpack_ok)
            goto fail;

    } else if (mpack_tag_type(&result_tag) == mpack_type_bin) {

        size_t result_len = mpack_expect_bin(&reader);

        if (mpack_reader_error(&reader) != mpack_ok)
            goto fail;

        if (result_len > 0) {
            void *result_data = malloc(result_len);

            if (!result_data)
                goto fail;

            mpack_read_bytes(
                &reader,
                result_data,
                result_len
            );

            if (mpack_reader_error(&reader) != mpack_ok) {
                free(result_data);
                goto fail;
            }

            mpack_done_bin(&reader);

            if (mpack_reader_error(&reader) != mpack_ok) {
                free(result_data);
                goto fail;
            }

            if (!RawBufferSetData(
                    response->result,
                    result_data,
                    result_len
                )) {
                free(result_data);
                goto fail;
            }

            free(result_data);
        } else {
            mpack_done_bin(&reader);

            if (mpack_reader_error(&reader) != mpack_ok)
                goto fail;
        }

    } else {
        goto fail;
    }

    /*
     * Finish the array.
     */
    mpack_done_array(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    mpack_reader_destroy(&reader);

    return response;

fail:
    mpack_reader_destroy(&reader);

    if (response)
        ResponseFree(&response);

    return NULL;
}

Response* ResponseCreateStrResult(uint32_t msgid, const char* res)
{
    HazeLogDebug("ResponseNew");

    Response* resp = ResponseNew();

    if (!resp) {
        HazeLogError("ResponseNew returned NULL");
        return NULL;
    }

    HazeLogDebug("ResponseSetMsgId");

    ResponseSetMsgId(resp, msgid);

    HazeLogDebug("ResponseSetResult");

    ResponseSetResult(
        resp,
        (void*)res,
        strlen(res)
    );

    HazeLogDebug("ResponseCreateStrResult done");

    return resp;
}
