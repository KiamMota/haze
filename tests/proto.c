#include "HazeLog.h"
#include "proto/RawBuffer.h"
#include "proto/Request.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---- logging simples e estável ---- */
#define LOG(fmt, ...)                                                          \
  do {                                                                         \
    time_t _now = time(NULL);                                                  \
    struct tm _tm;                                                             \
    localtime_s(&_tm, &_now); /* Windows */                                    \
    char _buf[32];                                                             \
    strftime(_buf, sizeof(_buf), "%H:%M:%S", &_tm);                            \
    fprintf(stdout, "[%s] " fmt "\n", _buf, ##__VA_ARGS__);                    \
    fflush(stdout);                                                            \
  } while (0)

/* se localtime_s não existir (MinGW antigo), use: */
#if defined(__MINGW32__) && !defined(_MSC_VER)
#undef LOG
#define LOG(fmt, ...)                                                          \
  do {                                                                         \
    time_t _now = time(NULL);                                                  \
    struct tm *_t = localtime(&_now);                                          \
    char _buf[32];                                                             \
    strftime(_buf, sizeof(_buf), "%H:%M:%S", _t);                              \
    fprintf(stdout, "[%s] " fmt "\n", _buf, ##__VA_ARGS__);                    \
    fflush(stdout);                                                            \
  } while (0)
#endif

/* helpers */
static void expect_true(bool cond, const char *msg) {
  if (!cond) {
    LOG("ASSERT_FAIL: %s", msg);
    fprintf(stderr, "ASSERT FAILED: %s\n", msg);
    abort();
  }
}
static void expect_false(bool cond, const char *msg) {
  expect_true(!cond, msg);
}
static void expect_eq_int(int a, int b, const char *msg) {
  if (a != b) {
    LOG("ASSERT_FAIL: %s (got %d, expected %d)", msg, a, b);
    fprintf(stderr, "ASSERT FAILED: %s (got %d, expected %d)\n", msg, a, b);
    abort();
  }
}
static void expect_eq_str(const char *a, const char *b, const char *msg) {
  if ((a == NULL && b != NULL) || (a != NULL && b == NULL) ||
      (a && b && strcmp(a, b) != 0)) {
    LOG("ASSERT_FAIL: %s (got '%s', expected '%s')", msg, a ? a : "(null)",
        b ? b : "(null)");
    fprintf(stderr, "ASSERT FAILED: %s (got '%s', expected '%s')\n", msg,
            a ? a : "(null)", b ? b : "(null)");
    abort();
  }
}

/* ============================================================ */
static void test_request_new_free(void) {
  LOG(">>> test_request_new_free START");
  printf("[test] RequestNew / RequestFree\n");

  LOG("RequestNew()...");
  Request *rq = RequestNew();
  LOG("RequestNew -> %p", (void *)rq);
  assert(rq != NULL);

  LOG("RequestParamCount(rq)...");
  int c = RequestParamCount(rq);
  LOG("count = %d", c);
  assert(c == 0);

  LOG("RequestMethod(rq)...");
  const char *m = RequestMethod(rq);
  LOG("method = %s", m ? m : "(null)");
  assert(m == NULL || m[0] == '\0');

  LOG("RequestFree(&rq)...");
  RequestFree(&rq);
  LOG("after free rq = %p", (void *)rq);
  assert(rq == NULL);

  LOG("double free / NULL free...");
  RequestFree(&rq);
  RequestFree(NULL);
  LOG("<<< test_request_new_free OK");
}

/* ============================================================ */
static void test_request_set_method(void) {
  LOG(">>> test_request_set_method START");
  printf("[test] RequestSetMethod\n");

  Request *rq = RequestNew();
  assert(rq != NULL);

  LOG("SetMethod 'haze.ping'...");
  RequestSetMethod(rq, "haze.ping");
  expect_eq_str(RequestMethod(rq), "haze.ping", "method after set");

  LOG("SetMethod 'haze.echo'...");
  RequestSetMethod(rq, "haze.echo");
  expect_eq_str(RequestMethod(rq), "haze.echo", "method overwrite");

  LOG("SetMethod ''...");
  RequestSetMethod(rq, "");
  expect_eq_str(RequestMethod(rq), "", "empty method");

  RequestFree(&rq);
  LOG("<<< test_request_set_method OK");
}

/* ============================================================ */
static void test_param_factories(void) {
  LOG(">>> test_param_factories START");
  printf("[test] RequestParamInit*\n");

  LOG("InitStr(\"hello\")...");
  RequestParam *p_str = RequestParamInitStr("hello");
  LOG("p_str = %p", (void *)p_str);
  assert(p_str != NULL);
  assert(RequestParamTypeGet(p_str) == PARAM_STR);
  assert(p_str->size == 5);
  assert(p_str->value.str_value != NULL);
  assert(strncmp(p_str->value.str_value, "hello", 5) == 0);

  LOG("InitInt(-42)...");
  RequestParam *p_int = RequestParamInitInt(-42);
  LOG("p_int = %p", (void *)p_int);
  assert(p_int != NULL);
  assert(RequestParamTypeGet(p_int) == PARAM_INT);
  expect_eq_int(p_int->value.int_value, -42, "int param value");

  LOG("InitBool(true)...");
  RequestParam *p_bool = RequestParamInitBool(true);
  LOG("p_bool = %p", (void *)p_bool);
  assert(p_bool != NULL);
  assert(RequestParamTypeGet(p_bool) == PARAM_BOOL);
  assert(p_bool->value.bool_value == true);

  LOG("InitBool(false)...");
  RequestParam *p_bool_f = RequestParamInitBool(false);
  LOG("p_bool_f = %p", (void *)p_bool_f);
  assert(p_bool_f != NULL);
  assert(RequestParamTypeGet(p_bool_f) == PARAM_BOOL);
  expect_false(p_bool_f->value.bool_value, "bool param false value");

  LOG("InitStr(\"\")...");
  RequestParam *p_empty = RequestParamInitStr("");
  LOG("p_empty = %p", (void *)p_empty);
  assert(p_empty != NULL);
  assert(RequestParamTypeGet(p_empty) == PARAM_STR);
  assert(p_empty->size == 0);

  LOG("free params...");
  free(p_str);
  free(p_int);
  free(p_bool);
  free(p_bool_f);
  free(p_empty);
  LOG("<<< test_param_factories OK");
}

/* ============================================================ */
static void test_param_append_get_count(void) {
  LOG(">>> test_param_append_get_count START");
  printf("[test] RequestParamAppend / Get / Count\n");

  Request *rq = RequestNew();
  LOG("rq = %p", (void *)rq);
  assert(rq != NULL);
  assert(RequestParamCount(rq) == 0);

  RequestParam *p0 = RequestParamInitInt(10);
  RequestParam *p1 = RequestParamInitStr("world");
  RequestParam *p2 = RequestParamInitBool(true);
  LOG("p0=%p p1=%p p2=%p", (void *)p0, (void *)p1, (void *)p2);

  LOG("Append p0 @0...");
  assert(RequestParamAppend(rq, p0, 0) == true);
  LOG("count after 0 = %d", RequestParamCount(rq));
  assert(RequestParamCount(rq) == 1);

  LOG("Append p1 @1...");
  assert(RequestParamAppend(rq, p1, 1) == true);
  LOG("count after 1 = %d", RequestParamCount(rq));
  assert(RequestParamCount(rq) == 2);

  LOG("Append p2 @2...");
  assert(RequestParamAppend(rq, p2, 2) == true);
  LOG("count after 2 = %d", RequestParamCount(rq));
  assert(RequestParamCount(rq) == 3);

  LOG("Get(0)...");
  RequestParam *g0 = RequestParamGet(rq, 0);
  LOG("g0 = %p", (void *)g0);
  assert(g0 != NULL);
  expect_eq_int(g0->value.int_value, 10, "g0 value");

  LOG("Get(1)...");
  RequestParam *g1 = RequestParamGet(rq, 1);
  LOG("g1 = %p size=%u", (void *)g1, g1 ? (unsigned)g1->size : 0);
  assert(g1 != NULL && g1->size == 5 &&
         strncmp(g1->value.str_value, "world", 5) == 0);

  LOG("Get(2)...");
  RequestParam *g2 = RequestParamGet(rq, 2);
  LOG("g2 = %p", (void *)g2);
  assert(g2 != NULL && g2->value.bool_value == true);

  /* out of bounds */
  LOG("Get(3) [OOB]...");
  RequestParam *g3 = RequestParamGet(rq, 3);
  LOG("Get(3) -> %p", (void *)g3);
  assert(g3 == NULL);

  LOG("Get(999) [OOB]...");
  RequestParam *g999 = RequestParamGet(rq, 999);
  LOG("Get(999) -> %p", (void *)g999);
  assert(g999 == NULL);

  LOG("Get(NULL, 0)...");
  assert(RequestParamGet(NULL, 0) == NULL);

  /* type checks — um log por chamada */
  LOG("IsType(INT, 0)...");
  bool t0 = RequestParamIsType(rq, PARAM_INT, 0);
  LOG("IsType(INT, 0) -> %d", (int)t0);
  assert(t0 == true);

  LOG("IsType(STR, 1)...");
  bool t1 = RequestParamIsType(rq, PARAM_STR, 1);
  LOG("IsType(STR, 1) -> %d", (int)t1);
  assert(t1 == true);

  LOG("IsType(BOOL, 2)...");
  bool t2 = RequestParamIsType(rq, PARAM_BOOL, 2);
  LOG("IsType(BOOL, 2) -> %d", (int)t2);
  assert(t2 == true);

  LOG("IsType(STR, 0) [expect false]...");
  bool t3 = RequestParamIsType(rq, PARAM_STR, 0);
  LOG("IsType(STR, 0) -> %d", (int)t3);
  assert(t3 == false);

  LOG("IsType(INT, 99) ");
  bool t99 = RequestParamIsType(rq, PARAM_INT, 99);
  LOG("IsType(INT, 99) -> %d", (int)t99);
  assert(t99 == false);

  LOG("IsType(NULL, ...)...");
  bool tn = RequestParamIsType(NULL, PARAM_INT, 0);
  LOG("IsType(NULL) -> %d", (int)tn);
  assert(tn == false);

  LOG("RequestFree...");
  RequestFree(&rq);
  LOG("<<< test_param_append_get_count OK");
}

/* ============================================================ */
static void test_param_sparse_append(void) {
  LOG(">>> test_param_sparse_append START");
  printf("[test] sparse / overwrite append\n");

  Request *rq = RequestNew();
  assert(rq != NULL);

  RequestParam *p0 = RequestParamInitInt(1);
  RequestParam *p2 = RequestParamInitInt(3);

  LOG("Append p0 @0...");
  assert(RequestParamAppend(rq, p0, 0) == true);

  LOG("Append p2 @2 (sparse)...");
  bool ok = RequestParamAppend(rq, p2, 2);
  LOG("sparse append result = %d", (int)ok);

  if (ok) {
    RequestParam *g2 = RequestParamGet(rq, 2);
    LOG("Get(2) after sparse = %p", (void *)g2);
    if (g2) {
      expect_eq_int(g2->value.int_value, 3, "sparse g2 value");
    }
  } else {
    printf("  (info) append em índice não-contíguo rejeitado\n");
  }

  RequestFree(&rq);
  LOG("<<< test_param_sparse_append OK");
}

/* ============================================================ */
static void test_marshal_unmarshal(void) {
  LOG(">>> test_marshal_unmarshal START");
  printf("[test] RequestMarshal / RequestUnmarshal\n");

  Request *rq = RequestNew();
  assert(rq != NULL);

  RequestSetMethod(rq, "haze.add");
  assert(RequestParamAppend(rq, RequestParamInitInt(7), 0));
  assert(RequestParamAppend(rq, RequestParamInitInt(35), 1));
  assert(RequestParamAppend(rq, RequestParamInitStr("ok"), 2));

  LOG("Marshal...");
  RawBuffer *buf = RequestMarshal(rq);
  LOG("buf = %p len = %zu", (void *)buf, buf ? (size_t)buf->len : 0);
  assert(buf != NULL && buf->data != NULL && buf->len > 0);

  LOG("Unmarshal...");
  Request *rq2 = RequestUnmarshal(buf);
  LOG("rq2 = %p", (void *)rq2);
  assert(rq2 != NULL);

  expect_eq_str(RequestMethod(rq2), "haze.add", "method after unmarshal");
  LOG("param count after unmarshal = %d", RequestParamCount(rq2));
  expect_eq_int(RequestParamCount(rq2), 3, "unmarshaled param count");

  RequestParam *a = RequestParamGet(rq2, 0);
  RequestParam *b = RequestParamGet(rq2, 1);
  RequestParam *c = RequestParamGet(rq2, 2);
  LOG("a=%p b=%p c=%p", (void *)a, (void *)b, (void *)c);

  assert(a && a->type == PARAM_INT);
  expect_eq_int(a->value.int_value, 7, "param a");
  assert(b && b->type == PARAM_INT);
  expect_eq_int(b->value.int_value, 35, "param b");
  assert(c && c->type == PARAM_STR && c->size == 2);
  assert(strncmp(c->value.str_value, "ok", 2) == 0);

  RequestFree(&rq);
  RequestFree(&rq2);
  LOG("<<< test_marshal_unmarshal OK");
}

/* ============================================================ */
static void test_null_and_edges(void) {
  LOG(">>> test_null_and_edges START");
  printf("[test] null & edge cases\n");

  expect_eq_int(RequestParamCount(NULL), 0, "null count");
  assert(RequestParamGet(NULL, 0) == NULL);
  expect_false(RequestParamIsType(NULL, PARAM_INT, 0), "null type check");

  Request *rq = RequestNew();
  assert(rq != NULL);

  LOG("Append NULL param...");
  expect_false(RequestParamAppend(rq, NULL, 0), "append null param");

  LOG("SetMethod(NULL)...");
  RequestSetMethod(rq, NULL);

  RequestFree(&rq);
  LOG("<<< test_null_and_edges OK");
}

/* ============================================================ */
static void test_many_params(void) {
  LOG(">>> test_many_params START");
  printf("[test] many params\n");

  Request *rq = RequestNew();
  assert(rq != NULL);

  const int N = 64;
  LOG("appending %d params...", N);
  for (int i = 0; i < N; i++) {
    RequestParam *p = RequestParamInitInt(i * 3);
    assert(RequestParamAppend(rq, p, (uint32_t)i) == true);
  }
  expect_eq_int(RequestParamCount(rq), N, "many params count");

  LOG("validating %d params...", N);
  for (int i = 0; i < N; i++) {
    RequestParam *p = RequestParamGet(rq, (uint32_t)i);
    assert(p != NULL);
    assert(p->type == PARAM_INT);
    expect_eq_int(p->value.int_value, i * 3, "stress param value");
  }

  RequestFree(&rq);
  LOG("<<< test_many_params OK");
}
#ifdef _WIN32
#include <windows.h>
static double now_sec(void) {
  static LARGE_INTEGER freq = {0};
  LARGE_INTEGER c;
  if (!freq.QuadPart)
    QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&c);
  return (double)c.QuadPart / (double)freq.QuadPart;
}
#else
static double now_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}
#endif
/* ============================================================ */
int main(void) {
  const int N = 100000;

  /* warmup (evita custo de cold cache / first alloc) */
  for (int i = 0; i < 1000; i++) {
    Request *rq = RequestNew();
    RequestSetMethod(rq, "haze.ping");
    RequestParamAppend(rq, RequestParamInitInt(i), 0);
    RawBuffer *buf = RequestMarshal(rq);
    Request *rq2 = RequestUnmarshal(buf);
    RequestFree(&rq);
    RequestFree(&rq2);
    /* RawBufferFree(&buf);  // descomente se existir */
  }

  double t0 = now_sec();
  for (int i = 0; i < N; i++) {
    Request *rq = RequestNew();
    RequestSetMethod(rq, "haze.ping");
    RequestParamAppend(rq, RequestParamInitInt(i), 0);
    RawBuffer *buf = RequestMarshal(rq);
    Request *rq2 = RequestUnmarshal(buf);
    RequestFree(&rq);
    RequestFree(&rq2);
    /* RawBufferFree(&buf); */
  }
  double t1 = now_sec();

  double sec = t1 - t0;
  double ops = (sec > 0.0) ? (N / sec) : 0.0;
  double us_per_op = (sec * 1e6) / N;

  printf("N            = %d\n", N);
  printf("elapsed      = %.6f s  (%.3f ms)\n", sec, sec * 1000.0);
  printf("throughput   = %.0f marshal+unmarshal/s\n", ops);
  printf("latency avg  = %.3f µs/op\n", us_per_op);

  /* ---- unit tests depois ---- */
  LOG("=== Haze proto intensive tests START ===");
  printf("=== haze proto intensive tests ===\n");
  HazeLogDebug("starting intensive request tests");

  test_request_new_free();
  test_request_set_method();
  test_param_factories();
  test_param_append_get_count();
  test_param_sparse_append();
  test_marshal_unmarshal();
  test_null_and_edges();
  test_many_params();

  LOG("=== ALL TESTS PASSED ===");
  printf("=== ALL TESTS PASSED ===\n");
  return 0;
}
