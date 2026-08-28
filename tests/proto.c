#include "proto/Request.h"
#include "HazeLog.h"
#include "proto/RawBuffer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* helpers */
static void expect_true(bool cond, const char *msg) {
  if (!cond) {
    fprintf(stderr, "ASSERT FAILED: %s\n", msg);
    abort();
  }
}

static void expect_false(bool cond, const char *msg) {
  expect_true(!cond, msg);
}

static void expect_eq_int(int a, int b, const char *msg) {
  if (a != b) {
    fprintf(stderr, "ASSERT FAILED: %s (got %d, expected %d)\n", msg, a, b);
    abort();
  }
}

static void expect_eq_str(const char *a, const char *b, const char *msg) {
  if ((a == NULL && b != NULL) || (a != NULL && b == NULL) ||
      (a && b && strcmp(a, b) != 0)) {
    fprintf(stderr, "ASSERT FAILED: %s (got '%s', expected '%s')\n",
            msg, a ? a : "(null)", b ? b : "(null)");
    abort();
  }
}

/* ============================================================
 * 1. Lifecycle básico
 * ============================================================ */
static void test_request_new_free(void) {
  printf("[test] RequestNew / RequestFree\n");

  Request *rq = RequestNew();
  assert(rq != NULL);
  assert(RequestParamCount(rq) == 0);
  assert(RequestMethod(rq) == NULL || RequestMethod(rq)[0] == '\0');

  RequestFree(&rq);
  assert(rq == NULL);

  /* double free / null free não deve crashar */
  RequestFree(&rq);
  RequestFree(NULL);
}

/* ============================================================
 * 2. Method
 * ============================================================ */
static void test_request_set_method(void) {
  printf("[test] RequestSetMethod\n");

  Request *rq = RequestNew();
  assert(rq != NULL);

  RequestSetMethod(rq, "haze.ping");
  expect_eq_str(RequestMethod(rq), "haze.ping", "method after set");

  RequestSetMethod(rq, "haze.echo");
  expect_eq_str(RequestMethod(rq), "haze.echo", "method overwrite");

  RequestSetMethod(rq, "");
  expect_eq_str(RequestMethod(rq), "", "empty method");

  RequestFree(&rq);
}

/* ============================================================
 * 3. Param factories
 * ============================================================ */
static void test_param_factories(void) {
  printf("[test] RequestParamInit*\n");

  RequestParam *p_str = RequestParamInitStr("hello");
  assert(p_str != NULL);
  assert(RequestParamTypeGet(p_str) == PARAM_STR);
  assert(p_str->size == 5);
  assert(p_str->value.str_value != NULL);
  assert(strncmp(p_str->value.str_value, "hello", 5) == 0);

  RequestParam *p_int = RequestParamInitInt(-42);
  assert(p_int != NULL);
  assert(RequestParamTypeGet(p_int) == PARAM_INT);
  assert(p_int->value.int_value == -42);

  RequestParam *p_bool = RequestParamInitBool(true);
  assert(p_bool != NULL);
  assert(RequestParamTypeGet(p_bool) == PARAM_BOOL);
  assert(p_bool->value.bool_value == true);

  RequestParam *p_bool_f = RequestParamInitBool(false);
  assert(p_bool_f != NULL);
  assert(RequestParamTypeGet(p_bool_f) == PARAM_BOOL);
  assert(p_bool_f->value.bool_value == false);

  /* empty string */
  RequestParam *p_empty = RequestParamInitStr("");
  assert(p_empty != NULL);
  assert(RequestParamTypeGet(p_empty) == PARAM_STR);
  assert(p_empty->size == 0);

  free(p_str);
  free(p_int);
  free(p_bool);
  free(p_bool_f);
  free(p_empty);
}

/* ============================================================
 * 4. Append / Get / Count
 * ============================================================ */
static void test_param_append_get_count(void) {
  printf("[test] RequestParamAppend / Get / Count\n");

  Request *rq = RequestNew();
  assert(rq != NULL);
  assert(RequestParamCount(rq) == 0);

  /* append em ordem */
  RequestParam *p0 = RequestParamInitInt(10);
  RequestParam *p1 = RequestParamInitStr("world");
  RequestParam *p2 = RequestParamInitBool(true);

  assert(RequestParamAppend(rq, p0, 0) == true);
  assert(RequestParamCount(rq) == 1);

  assert(RequestParamAppend(rq, p1, 1) == true);
  assert(RequestParamCount(rq) == 2);

  assert(RequestParamAppend(rq, p2, 2) == true);
  assert(RequestParamCount(rq) == 3);

  /* get */
  RequestParam *g0 = RequestParamGet(rq, 0);
  RequestParam *g1 = RequestParamGet(rq, 1);
  RequestParam *g2 = RequestParamGet(rq, 2);

  assert(g0 != NULL && g0->value.int_value == 10);
  assert(g1 != NULL && g1->size == 5 && strncmp(g1->value.str_value, "world", 5) == 0);
  assert(g2 != NULL && g2->value.bool_value == true);

  /* out of bounds */
  assert(RequestParamGet(rq, 3) == NULL);
  assert(RequestParamGet(rq, 999) == NULL);
  assert(RequestParamGet(NULL, 0) == NULL);

  /* type checks */
  assert(RequestParamIsType(rq, PARAM_INT, 0) == true);
  assert(RequestParamIsType(rq, PARAM_STR, 1) == true);
  assert(RequestParamIsType(rq, PARAM_BOOL, 2) == true);
  assert(RequestParamIsType(rq, PARAM_STR, 0) == false);
  assert(RequestParamIsType(rq, PARAM_INT, 99) == false);
  assert(RequestParamIsType(NULL, PARAM_INT, 0) == false);

  RequestFree(&rq);
}

/* ============================================================
 * 5. Append com índice “buraco” / reordenação
 * ============================================================ */
static void test_param_sparse_append(void) {
  printf("[test] sparse / overwrite append\n");

  Request *rq = RequestNew();
  assert(rq != NULL);

  RequestParam *p0 = RequestParamInitInt(1);
  RequestParam *p2 = RequestParamInitInt(3);

  /* append no índice 0 e depois no 2 (pula 1) — depende da implementação */
  assert(RequestParamAppend(rq, p0, 0) == true);

  /* se a API exigir contíguo, isso pode falhar; se permitir sparse, ok */
  bool ok = RequestParamAppend(rq, p2, 2);
  if (ok) {
    assert(RequestParamCount(rq) >= 1);
    RequestParam *g2 = RequestParamGet(rq, 2);
    if (g2) {
      assert(g2->value.int_value == 3);
    }
  } else {
    /* implementação exige contíguo — comportamento aceitável */
    printf("  (info) append em índice não-contíguo rejeitado\n");
  }

  RequestFree(&rq);
}

/* ============================================================
 * 6. Marshal / Unmarshal roundtrip
 * ============================================================ */
static void test_marshal_unmarshal(void) {
  printf("[test] RequestMarshal / RequestUnmarshal\n");

  Request *rq = RequestNew();
  assert(rq != NULL);

  RequestSetMethod(rq, "haze.add");
  assert(RequestParamAppend(rq, RequestParamInitInt(7), 0));
  assert(RequestParamAppend(rq, RequestParamInitInt(35), 1));
  assert(RequestParamAppend(rq, RequestParamInitStr("ok"), 2));

  RawBuffer *buf = RequestMarshal(rq);
  assert(buf != NULL);
  assert(buf->data != NULL);
  assert(buf->len > 0);

  Request *rq2 = RequestUnmarshal(buf);
  assert(rq2 != NULL);

  expect_eq_str(RequestMethod(rq2), "haze.add", "method after unmarshal");
  printf("pararms : %d\n", RequestParamCount(rq2));
  assert(RequestParamCount(rq2) == 3);

  RequestParam *a = RequestParamGet(rq2, 0);
  RequestParam *b = RequestParamGet(rq2, 1);
  RequestParam *c = RequestParamGet(rq2, 2);

  assert(a && a->type == PARAM_INT && a->value.int_value == 7);
  assert(b && b->type == PARAM_INT && b->value.int_value == 35);
  assert(c && c->type == PARAM_STR && c->size == 2);
  assert(strncmp(c->value.str_value, "ok", 2) == 0);

  /* cleanup */
  RequestFree(&rq);
  RequestFree(&rq2);
  /* se RawBuffer tiver free próprio: */
  /* RawBufferFree(&buf); */
}

/* ============================================================
 * 7. Null / edge cases
 * ============================================================ */
static void test_null_and_edges(void) {
  printf("[test] null & edge cases\n");

  assert(RequestParamCount(NULL) == 0);
  assert(RequestParamGet(NULL, 0) == NULL);
  assert(RequestParamIsType(NULL, PARAM_INT, 0) == false);

  Request *rq = RequestNew();
  assert(rq != NULL);

  /* append null param */
  assert(RequestParamAppend(rq, NULL, 0) == false);

  /* method null */
  RequestSetMethod(rq, NULL); /* não deve crashar */

  RequestFree(&rq);
}

/* ============================================================
 * 8. Stress leve — muitos params
 * ============================================================ */
static void test_many_params(void) {
  printf("[test] many params\n");

  Request *rq = RequestNew();
  assert(rq != NULL);

  const int N = 64;
  for (int i = 0; i < N; i++) {
    RequestParam *p = RequestParamInitInt(i * 3);
    assert(RequestParamAppend(rq, p, (uint32_t)i) == true);
  }

  assert(RequestParamCount(rq) == N);

  for (int i = 0; i < N; i++) {
    RequestParam *p = RequestParamGet(rq, (uint32_t)i);
    assert(p != NULL);
    assert(p->type == PARAM_INT);
    assert(p->value.int_value == i * 3);
  }

  RequestFree(&rq);
}

/* ============================================================
 * main
 * ============================================================ */
int main(void) {
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

  printf("=== ALL TESTS PASSED ===\n");
  return 0;
}
