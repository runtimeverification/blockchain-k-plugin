#ifndef PLUGIN_UTIL_H
#define PLUGIN_UTIL_H

#include <cstdint>
#include <cstring>
#include "runtime/alloc.h"
#include "runtime/header.h"

extern "C" {
static inline string* allocString(size_t len) {
  struct string *result = (struct string *)koreAllocToken(len + sizeof(string));
  set_len(result, len);
  return result;
}

static string *hexEncode(unsigned char *digest, size_t len) {
  uint64_t hexLen = len * 2;
  char byte[3];
  struct string *result = allocString(hexLen);
  for (size_t i = 0, j = 0; i < len; i++, j += 2) {
    sprintf(byte, "%02x", digest[i]);
    result->data[j] = byte[0];
    result->data[j+1] = byte[1];
  }
  return result;
}

static string *raw(unsigned char *digest, size_t len) {
  struct string *result = allocString(len);
  memcpy(result->data, digest, len);
  return result;
}
}

#endif
