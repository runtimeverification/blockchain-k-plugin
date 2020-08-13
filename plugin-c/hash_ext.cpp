#include <cryptopp/blake2.h>
#include "plugin_util.h"

using namespace CryptoPP;

extern "C" {

struct string *hook_KRYPTO_blake2b256raw(struct string *str) {
  BLAKE2b h(false,32);
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest));
}

struct string *hook_KRYPTO_blake2b256(struct string *str) {
  BLAKE2b h(false,32);
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

}
