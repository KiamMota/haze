#include "HazeServer.h"
#include "HazeLog.h"
#include "HazeServerDispatch.h"
#include "RawBuffer.h"
#include "proto/Request.h"
#include "proto/Response.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

/* ---------------------------------------------------------- */
/* Conexão individual                                         */
/* ---------------------------------------------------------- */

typedef struct {
  uv_tcp_t handle;
  
  // Buffer persistente para lidar com framing TCP
  char *buffer;
  size_t buffer_len;
  size_t buffer_cap;
} HazeConn;

typedef struct {
  uv_write_t req;
  void *data_to_free;
  uv_stream_t *stream;
} haze_write_req_t;

static void haze_on_close(uv_handle_t *handle) {
  if (handle->data) {
    HazeConn *conn = (HazeConn *)handle->data; 
    handle->data = NULL; 
    
    if (conn->buffer) {
        free(conn->buffer);
    }
    free(conn);         
  }
}

static void haze_on_alloc(uv_handle_t *handle, size_t suggested,
                          uv_buf_t *buf) {
  (void)handle;
  buf->base = malloc(suggested);
  buf->len = buf->base ? (unsigned int)suggested : 0;
}

static void haze_on_write_done(uv_write_t *req, int status) {
  haze_write_req_t *wr = (haze_write_req_t *)req;

  if (status < 0) {
    HazeLogWarn("Write error: %s", uv_strerror(status));
    if (wr->stream && !uv_is_closing((uv_handle_t *)wr->stream)) {
      uv_close((uv_handle_t *)wr->stream, haze_on_close);
    }
  }
  
  if (wr->data_to_free) {
    free(wr->data_to_free);
  }
  
  free(wr);
}

void haze_send(uv_stream_t *stream, const void *data, size_t len) {
    haze_write_req_t *req = malloc(sizeof(*req));
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

    uv_buf_t buf = uv_buf_init(
        (char *)data_copy,
        (unsigned int)len
    );

    int rc = uv_write(
        (uv_write_t *)req,
        stream,
        &buf,
        1,
        haze_on_write_done
    );

    if (rc < 0) {
        free(req->data_to_free);
        free(req);
    }
}

static void haze_on_read(uv_stream_t *stream, ssize_t nread,
                         const uv_buf_t *buf) {
  HazeConn *conn = (HazeConn *)stream->data;

  if (nread < 0) {
    if (nread != UV_EOF) {
        HazeLogWarn("Read error: %s", uv_err_name((int)nread));
    }
    if (buf->base) free(buf->base);
    uv_close((uv_handle_t *)stream, haze_on_close);
    return;
  }

  if (nread == 0) {
    if (buf->base) free(buf->base);
    return;
  }

  // 1. Acumula os novos bytes no buffer da conexão
  size_t new_len = conn->buffer_len + nread;
  if (new_len > conn->buffer_cap) {
    size_t new_cap = conn->buffer_cap == 0 ? 1024 : conn->buffer_cap * 2;
    while (new_cap < new_len) new_cap *= 2;
    
    char *new_buf = realloc(conn->buffer, new_cap);
    if (!new_buf) {
        HazeLogError("Out of memory reallocating connection buffer");
        if (buf->base) free(buf->base);
        uv_close((uv_handle_t *)stream, haze_on_close);
        return;
    }
    conn->buffer = new_buf;
    conn->buffer_cap = new_cap;
  }
  memcpy(conn->buffer + conn->buffer_len, buf->base, nread);
  conn->buffer_len = new_len;
  if (buf->base) free(buf->base);

  // 2. Tenta fazer o parse iterativo de mensagens no buffer
  while (conn->buffer_len > 0) {
    RawBuffer recv = RawBufferInit(conn->buffer, conn->buffer_len);
    Request *request = RequestUnmarshal(&recv);

    // Mensagem incompleta, aguarda mais bytes do socket
    if (!request) {
      if (conn->buffer_len > 4 * 1024 * 1024) { // Limite de segurança: 4MB
         HazeLogWarn("Buffer capacity exceeded 4MB, potential malformed stream. Closing.");
         uv_close((uv_handle_t *)stream, haze_on_close);
      }
      break; 
    }

    Response *response = HazeServerDispatch(request);
    if (response) {
      RawBuffer *sendResponse = ResponseMarshal(response);
      if (sendResponse) {
        haze_send(stream, RawBufferData(sendResponse), RawBufferLen(sendResponse));
        RawBufferFree(&sendResponse);
      }
      ResponseFree(&response);
    }
    RequestFree(&request);
    
    size_t consumed = recv.len; 

    if (consumed == 0 || consumed > conn->buffer_len) {
        HazeLogError("Parser state invalid. Closing connection.");
        uv_close((uv_handle_t *)stream, haze_on_close);
        break;
    }

    // 3. Desliza a memória restante para o começo do buffer
    conn->buffer_len -= consumed;
    if (conn->buffer_len > 0) {
        memmove(conn->buffer, conn->buffer + consumed, conn->buffer_len);
    }
  }
}

static void haze_on_connect(uv_stream_t *server, int status) {
  if (status < 0) {
    HazeLogError("connect error: %s", uv_strerror(status));
    return;
  }

  HazeConn *conn = calloc(1, sizeof(HazeConn));
  if (!conn) return;

  int init_ret = uv_tcp_init(server->loop, &conn->handle);
  if (init_ret != 0) {
    free(conn);
    return;
  }

  conn->handle.data = conn;

  int accept_ret = uv_accept(server, (uv_stream_t *)&conn->handle);
  if (accept_ret != 0) {
    uv_close((uv_handle_t *)&conn->handle, haze_on_close);
    return;
  }

  // ATIVAÇÃO DO NODELAY: Desativa o Algoritmo de Nagle para evitar o delay de 30ms em conexões persistentes
  uv_tcp_nodelay(&conn->handle, 1);

  int read_ret = uv_read_start((uv_stream_t *)&conn->handle, haze_on_alloc, haze_on_read);
  if (read_ret != 0) {
    uv_close((uv_handle_t *)&conn->handle, haze_on_close);
    return;
  }
}

/* ---------------------------------------------------------- */
/* API pública                                                */
/* ---------------------------------------------------------- */

HazeServer *HazeServerNew(const char *addr, uint16_t port) {
  HazeServer *s = calloc(1, sizeof(HazeServer));
  if (!s) return NULL;

  s->loop = uv_loop_new();
  s->port = port;
  s->addr = strdup(addr ? addr : "127.0.0.1");

  uv_tcp_init(s->loop, &s->tcp);

  return s;
}

int HazeServerStart(HazeServer *s) {
  if (!s) return UV_EINVAL;

  struct sockaddr_in bind_addr;
  uv_ip4_addr(s->addr, s->port, &bind_addr);

  int r = uv_tcp_bind(&s->tcp, (const struct sockaddr *)&bind_addr, 0);
  if (r != 0) return r;

  r = uv_listen((uv_stream_t *)&s->tcp, 512, haze_on_connect);
  return r;
}

void HazeServerRun(HazeServer *s) {
  if (!s) return;
  uv_run(s->loop, UV_RUN_DEFAULT);
}

void HazeServerStop(HazeServer *s) {
  if (!s) return;
  uv_stop(s->loop);
}

void HazeServerFree(HazeServer **s_ptr) {
  if (!s_ptr || !*s_ptr) return;
  HazeServer *s = *s_ptr;

  if (s->loop) {
    uv_loop_close(s->loop);
    free(s->loop);
  }
  
  if (s->addr) {
      free((void*)s->addr);
  }

  free(s);
  *s_ptr = NULL;
}

uint16_t HazeServerPort(HazeServer *s) {
  if (!s) return 0;
  return s->port;
}

const char *HazeServerAddress(HazeServer *s) {
  if (!s) return NULL;
  return s->addr;
}
