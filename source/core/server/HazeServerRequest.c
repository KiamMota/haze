#include "HazeServerRequest.h"
#include "mpack/mpack.h"

#include <stdlib.h>
#include <string.h>

HazeServerRequest *HazeServerRequestNew(void)
{
    HazeServerRequest *request =
        calloc(1, sizeof(HazeServerRequest));

    if (!request)
        return NULL;

    request->type = HAZE_RPC_REQUEST;

    return request;
}
HazeServerRequest *HazeServerRequestUnmarshal(
    const void *data,
    size_t len
)
{
    if (!data || len == 0)
        return NULL;

    mpack_reader_t reader;

    mpack_reader_init_data(
        &reader,
        (const char *)data,
        len
    );

    HazeServerRequest *request = NULL;

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

    HazeServerRequestFree(&request);

    return NULL;
}

void HazeServerRequestSetMethod(
    HazeServerRequest *request,
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

bool HazeServerRequestSetParameters(
    HazeServerRequest *request,
    const void *data,
    size_t len
)
{
    if (!request)
        return false;

    free(request->parameters);

    request->parameters = NULL;
    request->parameters_len = 0;

    if (!data || len == 0)
        return true;

    request->parameters = malloc(len);

    if (!request->parameters)
        return false;

    memcpy(request->parameters, data, len);
    request->parameters_len = len;

    return true;
}

void *HazeServerRequestGetRaw(
    HazeServerRequest *request,
    size_t *len
)
{
    if (!request)
        return NULL;

    if (len)
        *len = request->parameters_len;

    return request->parameters;
}

void HazeServerRequestFree(
    HazeServerRequest **request
)
{
    if (!request || !*request)
        return;

    free((*request)->method);
    free((*request)->parameters);
    free(*request);

    *request = NULL;
}
