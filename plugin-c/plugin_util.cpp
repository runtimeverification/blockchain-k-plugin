#include "plugin_util.h"

extern "C" {
string *allocString(size_t len) {
  struct string *result =
      (struct string *)kore_alloc_token(len + sizeof(string));
  init_with_len(result, len);
  return result;
}

string *hexEncode(unsigned char *digest, size_t len) {
  uint64_t hexLen = len * 2;
  char byte[3];
  struct string *result = allocString(hexLen);
  for (size_t i = 0, j = 0; i < len; i++, j += 2) {
    snprintf(byte, sizeof(byte), "%02x", digest[i]);
    result->data[j] = byte[0];
    result->data[j + 1] = byte[1];
  }
  return result;
}

string *raw(unsigned char *digest, size_t len) {
  struct string *result = allocString(len);
  memcpy(result->data, digest, len);
  return result;
}
}
