#include "HazeServer.h"
#include "modules/HazeLog.h"
#include "server/HazeServerMiddleware.h"
#include "server/HazeServerResponse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* ---------------------------------------------------------- */
/* Conexão individual                                          */
/* ---------------------------------------------------------- */

typedef struct {
  uv_tcp_t handle;
  uv_buf_t buf;
} HazeConn;

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
  if (status < 0)
    HazeLogWarn("Write error: %s", uv_strerror(status));
  free(req);
}

void haze_send(uv_stream_t *stream, const void *data, size_t len) {
  uv_write_t *req = malloc(sizeof(uv_write_t));
  if (!req)
    return;

  uv_buf_t buf = uv_buf_init((char *)data, (unsigned int)len);

  HazeServerResponse *res = HazeServerResponseNew();

  uv_write(req, stream, &buf, 1, haze_on_write_done);
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

  if (HazeServerMiddlewareIsHttp(buf->base, nread) == HAZE_MW_REJECT) {
    HazeLogDebug("%s", "Received HTTP, rejected.");
    goto close;
  }
  haze_send(stream, buf->base, nread);

  free(buf->base);
  return;

close:
  free(buf->base);
  uv_close((uv_handle_t *)stream, haze_on_close);
}

static void haze_on_connect(uv_stream_t *server, int status) {
  if (status < 0) {
    fprintf(stderr, "[haze] connect error: %s\n", uv_strerror(status));
    fflush(stdout);
    return;
  }

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

  s->addr = strdup(addr ? addr : "127.0.0.1");
  s->port = port;
  s->loop = uv_default_loop();

  uv_tcp_init(s->loop, &s->tcp);
  s->tcp.data = s;

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

void HazeServerFree(HazeServer **s) {
  if (!s || !*s)
    return;
  HazeServerStop(*s);
  uv_loop_close((*s)->loop);
  free((*s)->addr);
  free(*s);
  *s = NULL;
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
