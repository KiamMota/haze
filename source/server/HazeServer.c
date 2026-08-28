#include "HazeServer.h"
#include "HazeLog.h"
#include "HazeServerDispatch.h"
#include "HazeServerMiddleware.h"
#include "proto/MessagePackRPC.h"
#include "proto/RawBuffer.h"
#include "proto/Request.h"
#include "proto/Response.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

/* ---------------------------------------------------------- */
/* Conexão individual                                          */
/* ---------------------------------------------------------- */

typedef struct {
  uv_tcp_t handle;
  uv_buf_t buf;
} HazeConn;

typedef struct {
  uv_write_t req;
  void *data_to_free;
  uv_stream_t *stream;
} haze_write_req_t;

static void haze_on_close(uv_handle_t *handle) {
  if (handle->data) {
    HazeConn *conn = (HazeConn *)handle->data;
    HazeLogDebug("Connection closed: %p", (void *)conn);
    
    handle->data = NULL; // Fazemos antes, enquanto a memória é nossa
    free(conn);          // Agora sim, adeus
  }
}static void haze_on_alloc(uv_handle_t *handle, size_t suggested,
                          uv_buf_t *buf) {
  (void)handle;
  buf->base = malloc(suggested);
  buf->len = buf->base ? (unsigned int)suggested : 0;
}

static void haze_on_read(uv_stream_t *stream, ssize_t nread,
                         const uv_buf_t *buf);

static void haze_on_write_done(uv_write_t *req, int status) {
  if (status < 0) {
    HazeLogWarn("Write error: %s", uv_strerror(status));
  }
  
  haze_write_req_t *wr = (haze_write_req_t *)req;
  
  if (wr->data_to_free) {
    free(wr->data_to_free);
  }
  
  if (wr->stream && !uv_is_closing((uv_handle_t *)wr->stream)) {
    HazeLogDebug("Write done, server closing connection");
    uv_close((uv_handle_t *)wr->stream, haze_on_close);
  }
  
  free(wr);
}

void haze_send(uv_stream_t *stream, const void *data, size_t len) {
  haze_write_req_t *req = malloc(sizeof(haze_write_req_t));
  if (!req)
    return;

  void *data_copy = malloc(len);
  if (!data_copy) {
    free(req);
    return;
  }
  memcpy(data_copy, data, len);

  req->data_to_free = data_copy;
  req->stream = stream;
  
  uv_buf_t buf = uv_buf_init((char *)data_copy, (unsigned int)len);

  HazeLogDebug("Sending %zu bytes, pausing reads", len);
  // Pausa leitura para garantir que escrita completa antes de processar novo request
  uv_read_stop(stream);
  
  uv_write((uv_write_t *)req, stream, &buf, 1, haze_on_write_done);
}
static void haze_on_read(uv_stream_t *stream, ssize_t nread,
                         const uv_buf_t *buf) {
  if (nread < 0) {
    if (nread != UV_EOF) {
      HazeLogDebug("Client disconnected or error");
    }

    if (buf->base) free(buf->base);
    uv_close((uv_handle_t *)stream, haze_on_close);
    return;
  }

  if (nread == 0) {
    if (buf->base) free(buf->base);
    return;
  }

  RawBuffer recv = RawBufferInit(buf->base, nread);
  Request *request = RequestUnmarshal(&recv);
  printf("Received Request: ");
  RequestPrint(request);

  if (!request) {
    HazeLogWarn("Failed to deserialize incoming request");
    Response *error = ResponseNew();
    if (error) {
      ResponseSetMsgId(error, 0);
      ResponseSetError(error, MPACKRPC_MALFORMED_REQ);

      RawBuffer *response = ResponseMarshal(error);

      if (response) {
        haze_send(stream, RawBufferData(response), RawBufferLen(response));
        RawBufferFree(&response);
      }

      ResponseFree(&error);
    }

    // CORREÇÃO 1: Libera a memória se o request for inválido
    if (buf->base) free(buf->base);
    return;
  }

  Response *response = HazeServerDispatch(request);
  HazeLogDebug("Dispatched request");

  if (response) {
    RawBuffer *sendResponse = ResponseMarshal(response);

    if (sendResponse) {
      HazeLogDebug("Sending response bytes");
      haze_send(stream, RawBufferData(sendResponse), RawBufferLen(sendResponse));
      RawBufferFree(&sendResponse);
    }

    ResponseFree(&response);
  }

  // CORREÇÃO 2: Libera a memória após o sucesso (já processamos o request)
  if (buf->base) free(buf->base);
  return;
}

static void haze_on_connect(uv_stream_t *server, int status) {
  if (status < 0) {
    HazeLogError("connect error: %s", uv_strerror(status));
    return;
  }

  HazeLogDebug("1. New Connection");

  HazeConn *conn = calloc(1, sizeof(HazeConn));
  HazeLogDebug("2. calloc: %p", (void *)conn);

  if (!conn)
    return;

  HazeLogDebug("3. before uv_tcp_init");

  int init_ret = uv_tcp_init(server->loop, &conn->handle);

  HazeLogDebug("4. after uv_tcp_init: %d", init_ret);

  if (init_ret != 0) {
    free(conn);
    return;
  }

  conn->handle.data = conn;

  HazeLogDebug("5. before uv_accept");

  int accept_ret =
      uv_accept(server, (uv_stream_t *)&conn->handle);

  HazeLogDebug("6. after uv_accept: %d", accept_ret);

  if (accept_ret != 0) {
    uv_close((uv_handle_t *)&conn->handle, haze_on_close);
    return;
  }

  HazeLogDebug("7. before uv_read_start");

  int read_ret =
      uv_read_start(
          (uv_stream_t *)&conn->handle,
          haze_on_alloc,
          haze_on_read);

  HazeLogDebug("8. after uv_read_start: %d", read_ret);

  if (read_ret != 0) {
    uv_close((uv_handle_t *)&conn->handle, haze_on_close);
    return;
  }
}

/* ---------------------------------------------------------- */
/* API pública                                                 */
/* ---------------------------------------------------------- */

HazeServer *HazeServerNew(const char *addr, uint16_t port) {
  HazeServer *s = calloc(1, sizeof(HazeServer));
  if (!s)
    return NULL;

  s->loop = uv_loop_new();
  s->port = port;
  s->addr = addr ? strdup(addr) : "127.0.0.1";

  uv_tcp_init(s->loop, &s->tcp);

  return s;
}

int HazeServerStart(HazeServer *s) {
  if (!s)
    return UV_EINVAL;

  struct sockaddr_in bind_addr;
  uv_ip4_addr(s->addr, s->port, &bind_addr);

  int r = uv_tcp_bind(&s->tcp, (const struct sockaddr *)&bind_addr, 0);
  if (r != 0)
    return r;

  r = uv_listen((uv_stream_t *)&s->tcp, 128, haze_on_connect);
  return r;
}

void HazeServerRun(HazeServer *s) {
  if (!s)
    return;
  uv_run(s->loop, UV_RUN_DEFAULT);
}

void HazeServerStop(HazeServer *s) {
  if (!s)
    return;
  uv_stop(s->loop);
}

void HazeServerFree(HazeServer **s_ptr) {
  if (!s_ptr || !*s_ptr)
    return;
  HazeServer *s = *s_ptr;

  if (s->loop) {
    uv_loop_close(s->loop);
    free(s->loop);
  }

  free(s);
  *s_ptr = NULL;
}

uint16_t HazeServerPort(HazeServer *s) {
  if (!s)
    return 0;
  return s->port;
}

const char *HazeServerAddress(HazeServer *s) {
  if (!s)
    return NULL;
  return s->addr;
}
