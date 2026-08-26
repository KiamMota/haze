#include "HazeServer.h"
#include "HazeLog.h"
#include "HazeServerDispatch.h"
#include "HazeServerMiddleware.h"
#include "core/proto/MessagePackRPC.h"
#include "core/proto/RawBuffer.h"
#include "core/proto/Request.h"
#include "core/proto/Response.h"
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
} haze_write_req_t;

static void haze_on_close(uv_handle_t *handle) {
  if (handle->data) {
    free(handle->data);
    handle->data = NULL;
  }
}

static void haze_on_alloc(uv_handle_t *handle, size_t suggested,
                          uv_buf_t *buf) {
  (void)handle;
  buf->base = malloc(suggested);
  buf->len = buf->base ? (unsigned int)suggested : 0;
}
static void haze_on_write_done(uv_write_t *req, int status) {
  if (status < 0) {
    HazeLogWarn("Write error: %s", uv_strerror(status));
  }
  haze_write_req_t *wr = (haze_write_req_t *)req;
  if (wr->data_to_free) {
    free(wr->data_to_free);
  }
  free(wr);
}

void haze_send(uv_stream_t *stream, const void *data, size_t len) {
  haze_write_req_t *req = malloc(sizeof(haze_write_req_t));
  if (!req)
    return;

  // Duplica o buffer para garantir a validade dos dados durante o envio
  // assíncrono
  void *data_copy = malloc(len);
  if (!data_copy) {
    free(req);
    return;
  }
  memcpy(data_copy, data, len);

  req->data_to_free = data_copy;
  uv_buf_t buf = uv_buf_init((char *)data_copy, (unsigned int)len);

  uv_write((uv_write_t *)req, stream, &buf, 1, haze_on_write_done);
}

static void haze_on_read(uv_stream_t *stream, ssize_t nread,
                         const uv_buf_t *buf) {
  if (nread < 0) {
    if (nread != UV_EOF)
      HazeLogWarn("Read error: %s", uv_strerror((int)nread));

    goto close;
  }

  if (nread == 0) {
    free(buf->base);
    return;
  }

  HazeLogDebug("Received %zd bytes (MsgPack)", nread);

  RawBuffer *recv = RawBufferNew(buf->base, nread);
  if (!recv)
    goto cleanup;
  HazeLogDebug("libuv first bytes: %02X %02X %02X %02X",
               (unsigned char)buf->base[0], (unsigned char)buf->base[1],
               (unsigned char)buf->base[2], (unsigned char)buf->base[3]);
  /*
   * Transport layer:
   * RawBuffer -> Request
   */
  HazeLogDebug("Deserializing incoming request...");

  Request *request = RequestUnmarshal(recv);

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

    goto cleanup;
  }

  HazeLogDebug("Request deserialized: ID=%u, Method='%s'", request->msgid,
               request->method ? request->method : "NULL");

  /*
   * Dispatcher layer:
   * Request -> Response
   */
  Response *response = HazeServerDispatch(request);

  if (!response) {
    HazeLogError("Dispatcher returned NULL");
    goto cleanup;
  }

  RawBuffer *send_buffer = ResponseMarshal(response);

  if (!send_buffer) {
    HazeLogError("Failed to serialize response");
    ResponseFree(&response);
    goto cleanup;
  }

  haze_send(stream, RawBufferData(send_buffer), RawBufferLen(send_buffer));

  RawBufferFree(&send_buffer);
  ResponseFree(&response);

cleanup:
  RawBufferFree(&recv);
  free(buf->base);
  return;

close:
  free(buf->base);
  uv_close((uv_handle_t *)stream, haze_on_close);
}

static void haze_on_connect(uv_stream_t *server, int status) {
  if (status < 0) {
    HazeLogError("[haze] connect error: %s\n", uv_strerror(status));
    fflush(stdout);
    return;
  }

  HazeLogDebug("%s", "New Connection\n");
  HazeConn *conn = calloc(1, sizeof(HazeConn));
  if (!conn)
    return;

  uv_tcp_init(server->loop, &conn->handle);
  conn->handle.data = conn;

  if (uv_accept(server, (uv_stream_t *)&conn->handle) == 0) {
    uv_read_start((uv_stream_t *)&conn->handle, haze_on_alloc, haze_on_read);
  } else {
    uv_close((uv_handle_t *)&conn->handle, haze_on_close);
  }
}

/* ---------------------------------------------------------- */
/* API pública                                                 */
/* ---------------------------------------------------------- */

HazeServer *HazeServerNew(const char *addr, uint16_t port) {
  HazeServer *s = calloc(1, sizeof(HazeServer));
  if (!s)
    return NULL;

  // Cria um loop dedicado para esta instância
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

  // Fecha o loop dedicado
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
