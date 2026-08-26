#include "Request.h"
#include "core/proto/RawBuffer.h"
#include "mpack/mpack.h"

#include <stdlib.h>
#include <string.h>

Request *HazeServerRequestNew(void)
{
    Request *request =
        calloc(1, sizeof(Request));

    if (!request)
        return NULL;

    request->type = HAZE_RPC_REQUEST;

    return request;
}
Request *RequestUnmarshal(RawBuffer *b)
{
  if (!RawBufferLen(b))  {
    return NULL;
  }

    mpack_reader_t reader;

    mpack_reader_init_data(
        &reader, RawBufferData(b), RawBufferLen(b)
    );

    Request *request = NULL;

    /* [type, msgid, method, params] */
    uint32_t count = mpack_expect_array(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    if (count != 4)
        goto fail;

    request = HazeServerRequestNew();

    if (!request)
        goto fail;

    /*
     * type
     */
    uint8_t type = mpack_expect_u8(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    if (type != HAZE_RPC_REQUEST)
        goto fail;

    request->type = (HazeServerRPCType)type;

    /*
     * msgid
     */
    request->msgid = mpack_expect_u32(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    /*
     * method
     */
    size_t method_len = mpack_expect_str(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    request->method = malloc(method_len + 1);

    if (!request->method)
        goto fail;

    mpack_read_bytes(
        &reader,
        request->method,
        method_len
    );

    request->method[method_len] = '\0';

    mpack_done_str(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    /*
     * params
     *
     * Mantemos o objeto MessagePack bruto.
     * Isso permite que o parser de parâmetros seja
     * executado posteriormente.
     */

    mpack_tag_t params_tag = mpack_read_tag(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    /*
     * Aqui não devemos transformar params em BIN.
     * O protocolo diz que params é um objeto MessagePack,
     * especificamente um array.
     */

    if (mpack_tag_type(&params_tag) != mpack_type_array)
      goto fail;

    /*
     * Neste ponto, os parâmetros ainda pertencem ao
     * reader. Para guardar os bytes crus, precisamos
     * de uma estratégia de captura do objeto original.
     *
     * O MPack reader não fornece diretamente "retornar
     * os bytes do objeto já lido".
     */

    mpack_done_array(&reader);

    if (mpack_reader_error(&reader) != mpack_ok)
        goto fail;

    mpack_reader_destroy(&reader);

    return request;

fail:

    mpack_reader_destroy(&reader);

    RequestFree(&request);

    return NULL;
}

void HazeServerRequestSetMethod(
    Request *request,
    const char *method
)
{
    if (!request)
        return;

    free(request->method);
    request->method = NULL;

    if (!method)
        return;

    request->method = malloc(strlen(method) + 1);

    if (!request->method)
        return;

    strcpy(request->method, method);
}

RawBuffer *RequestGetRaw(Request *request) {
    if (!request) return NULL;

    size_t method_len = request->method ? strlen(request->method) + 1 : 0;
    size_t params_len = request->parameters ? RawBufferLen(request->parameters) : 0;
    size_t total_size = sizeof(request->type) + sizeof(request->msgid) + method_len + params_len;

    void *raw_data = malloc(total_size);
    if (!raw_data) return NULL;

    uint8_t *ptr = (uint8_t *)raw_data;
    
    memcpy(ptr, &request->type, sizeof(request->type));
    ptr += sizeof(request->type);

    memcpy(ptr, &request->msgid, sizeof(request->msgid));
    ptr += sizeof(request->msgid);

    if (method_len > 0) {
        memcpy(ptr, request->method, method_len);
        ptr += method_len;
    }

    if (params_len > 0) {
        memcpy(ptr, RawBufferData(request->parameters), params_len);
    }

    return RawBufferNew(raw_data, total_size);
}


bool RequestSetParameters(Request *request, RawBuffer *b) {
    if (!request || !b || RawBufferLen(b) == 0) {
        return false;
    }

    if (!request->parameters) {
        request->parameters = RawBufferNew(RawBufferData(b), RawBufferLen(b));
        return request->parameters != NULL;
    }

    return RawBufferSetData(request->parameters, RawBufferData(b), RawBufferLen(b));
}



void RequestFree(
    Request **request
)
{
    if (!request || !*request)
        return;

    free((*request)->method);
    free((*request)->parameters);
    free(*request);

    *request = NULL;
}
