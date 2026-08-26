#include "Client.h"
#include "proto/Request.h"
#include "proto/Response.h"
#include <uv.h>
#include <stdlib.h>
#include <string.h>

struct HazeClient {
    uv_loop_t *loop;
    uv_tcp_t socket;
    uint32_t current_msgid;
    bool is_connected;
};

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

typedef struct {
    Response *response;
    bool completed;
} CallContext;

static void on_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    CallContext *ctx = (CallContext *)stream->data;

    if (nread > 0) {
        /* Encapsula os dados lidos em um RawBuffer temporário */
        RawBuffer *response_raw = RawBufferNew(buf->base, (size_t)nread);
        if (response_raw) {
            ctx->response = ResponseUnmarshal(response_raw);
            RawBufferFree(&response_raw);
        }
        ctx->completed = true;
        uv_read_stop(stream);
    } else if (nread < 0) {
        ctx->completed = true;
        uv_read_stop(stream);
    }

    if (buf->base) {
        free(buf->base);
    }
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    (void)handle;
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_write(uv_write_t *req, int status) {
    write_req_t *wr = (write_req_t *)req;
    free(wr->buf.base);
    free(wr);

    if (status == 0) {
        uv_read_start(req->handle, alloc_buffer, on_read);
    }
}

static void on_connect(uv_connect_t *req, int status) {
    HazeClient *client = (HazeClient *)req->data;
    if (status == 0) {
        client->is_connected = true;
    }
    free(req);
}

HazeClient *HazeClientNew(const char *host, int port) {
    if (!host || port <= 0) return NULL;

    HazeClient *client = malloc(sizeof(HazeClient));
    if (!client) return NULL;

    client->loop = uv_loop_new();
    if (!client->loop) {
        free(client);
        return NULL;
    }

    uv_tcp_init(client->loop, &client->socket);
    client->current_msgid = 1;
    client->is_connected = false;

    struct sockaddr_in dest;
    if (uv_ip4_addr(host, port, &dest) != 0) {
        uv_loop_close(client->loop);
        free(client->loop);
        free(client);
        return NULL;
    }

    uv_connect_t *connect_req = malloc(sizeof(uv_connect_t));
    if (!connect_req) {
        uv_loop_close(client->loop);
        free(client->loop);
        free(client);
        return NULL;
    }
    connect_req->data = client;

    uv_tcp_connect(connect_req, &client->socket, (const struct sockaddr *)&dest, on_connect);
    
    uv_run(client->loop, UV_RUN_DEFAULT);

    if (!client->is_connected) {
        HazeClientFree(&client);
        return NULL;
    }

    return client;
}

void HazeClientFree(HazeClient **client) {
    if (!client || !*client) return;

    HazeClient *c = *client;

    if (!uv_is_closing((uv_handle_t *)&c->socket)) {
        uv_close((uv_handle_t *)&c->socket, NULL);
        uv_run(c->loop, UV_RUN_DEFAULT);
    }

    uv_loop_close(c->loop);
    free(c->loop);
    free(c);

    *client = NULL;
}

Response *HazeClientCall(HazeClient *client, const char *method, RawBuffer *params) {
    if (!client || !method || !client->is_connected) return NULL;

    /* 1. Prepara a estrutura da requisição */
    Request *req = HazeServerRequestNew();
    if (!req) return NULL;

    req->type = HAZE_RPC_REQUEST;
    req->msgid = client->current_msgid++;

    HazeServerRequestSetMethod(req, method);
    if (params) {
        RequestSetParameters(req, params);
    }

    RawBuffer *serialized_req = RequestGetRaw(req);
    RequestFree(&req);
    if (!serialized_req) return NULL;

    /* 2. Extrai dados via métodos oficiais da API RawBuffer */
    size_t req_len = RawBufferLen(serialized_req);
    const void *req_data = RawBufferData(serialized_req);

    if (!req_data || req_len == 0) {
        RawBufferFree(&serialized_req);
        return NULL;
    }

    /* 3. Prepara a escrita com a libuv */
    write_req_t *wr = malloc(sizeof(write_req_t));
    if (!wr) {
        RawBufferFree(&serialized_req);
        return NULL;
    }

    char *data_copy = malloc(req_len);
    if (!data_copy) {
        free(wr);
        RawBufferFree(&serialized_req);
        return NULL;
    }

    memcpy(data_copy, req_data, req_len);
    wr->buf = uv_buf_init(data_copy, (unsigned int)req_len);

    RawBufferFree(&serialized_req);

    CallContext ctx = { .response = NULL, .completed = false };
    client->socket.data = &ctx;

    /* 4. Envia e aguarda a resposta */
    if (uv_write(&wr->req, (uv_stream_t *)&client->socket, &wr->buf, 1, on_write) != 0) {
        free(data_copy);
        free(wr);
        return NULL;
    }

    while (!ctx.completed) {
        uv_run(client->loop, UV_RUN_ONCE);
    }

    return ctx.response;
}
