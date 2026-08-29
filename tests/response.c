#include "proto/Response.h"
#include "proto/RawBuffer.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void test_response_new(void) {
  Response *response = ResponseNew();

  assert(response != NULL);
  assert(ResponseMsgId(response) == 0);
  assert(ResponseError(response) == NULL);
  assert(ResponseResult(response) == NULL);

  assert(ResponseFree(&response));
  assert(response == NULL);
}

static void test_response_free(void) {
  Response *response = NULL;

  assert(ResponseFree(&response) == false);
  assert(ResponseFree(NULL) == false);

  response = ResponseNew();
  assert(response != NULL);

  assert(ResponseFree(&response) == true);
  assert(response == NULL);
}

static void test_msgid(void) {
  Response *response = ResponseNew();

  assert(response != NULL);

  assert(ResponseSetMsgId(response, 1));
  assert(ResponseMsgId(response) == 1);

  assert(ResponseSetMsgId(response, 123456));
  assert(ResponseMsgId(response) == 123456);

  assert(ResponseSetMsgId(response, UINT32_MAX));
  assert(ResponseMsgId(response) == UINT32_MAX);

  assert(ResponseSetMsgId(response, 0));
  assert(ResponseMsgId(response) == 0);

  assert(!ResponseSetMsgId(NULL, 1));

  ResponseFree(&response);
}


static void test_error(void) {
  Response *response = ResponseNew();

  assert(response != NULL);
  assert(ResponseError(response) == NULL);



  assert(ResponseError(response) != NULL);

  ResponseFree(&response);
}

static void test_create_str_result(void) {
  Response *response =
      ResponseCreateStrResult(
          42,
          "Session created successfully.");

  assert(response != NULL);
  assert(ResponseMsgId(response) == 42);
  assert(ResponseError(response) == NULL);
  assert(ResponseResult(response) != NULL);

  ResponseFree(&response);
}

static void test_create_error(void) {
  Response *response =
      ResponseCreateError(
          99,
          "malformed request");

  assert(response != NULL);
  assert(ResponseMsgId(response) == 99);
  assert(ResponseError(response) != NULL);
  assert(ResponseResult(response) == NULL);

  ResponseFree(&response);
}

static void test_marshal(void) {
  Response *response =
      ResponseCreateStrResult(
          1,
          "Session created successfully.");

  assert(response != NULL);

  RawBuffer *buffer =
      ResponseMarshal(response);

  assert(buffer != NULL);

  ResponseFree(&response);
}

static void test_roundtrip(void) {
  Response *original =
      ResponseCreateStrResult(
          123,
          "Session created successfully.");

  assert(original != NULL);

  RawBuffer *buffer =
      ResponseMarshal(original);

  assert(buffer != NULL);

  Response *decoded =
      ResponseUnmarshal(buffer);

  assert(decoded != NULL);

  assert(ResponseMsgId(decoded) ==
         ResponseMsgId(original));

  assert(ResponseError(decoded) == NULL);
  assert(ResponseResult(decoded) != NULL);

  ResponseFree(&decoded);
  ResponseFree(&original);

  assert(decoded == NULL);
  assert(original == NULL);
}

static void test_error_roundtrip(void) {
  Response *original =
      ResponseCreateError(
          777,
          "malformed request");

  assert(original != NULL);

  RawBuffer *buffer =
      ResponseMarshal(original);

  assert(buffer != NULL);

  Response *decoded =
      ResponseUnmarshal(buffer);

  assert(decoded != NULL);

  assert(ResponseMsgId(decoded) == 777);
  assert(ResponseError(decoded) != NULL);
  assert(ResponseResult(decoded) == NULL);

  ResponseFree(&decoded);
  ResponseFree(&original);
}

static void test_invalid_unmarshal(void) {
  assert(ResponseUnmarshal(NULL) == NULL);
}

int main(void) {
  printf("Running Response tests...\n");

  test_response_new();
  printf("[PASS] ResponseNew\n");

  test_response_free();
  printf("[PASS] ResponseFree\n");

  test_msgid();
  printf("[PASS] Message ID\n");


  test_error();
  printf("[PASS] Error\n");

  test_create_str_result();
  printf("[PASS] CreateStrResult\n");

  test_create_error();
  printf("[PASS] CreateError\n");

  test_marshal();
  printf("[PASS] Marshal\n");

  test_roundtrip();
  printf("[PASS] Roundtrip\n");

  test_error_roundtrip();
  printf("[PASS] Error roundtrip\n");

  test_invalid_unmarshal();
  printf("[PASS] Invalid Unmarshal\n");

  printf("\nALL RESPONSE TESTS PASSED\n");

  return 0;
}
