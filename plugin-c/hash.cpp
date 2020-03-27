#include <cstdint>
#include <cryptopp/keccak.h>
#include <cryptopp/ripemd.h>
#include <cryptopp/sha.h>
#include <cryptopp/sha3.h>
#include <gmp.h>
#include "blake2.h"
#include "plugin_util.h"

using namespace CryptoPP;

extern "C" {

struct string *hook_HASH_sha3raw(struct string *str) {
  SHA3_256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest)); 
}

struct string *hook_HASH_sha3(struct string *str) {
  SHA3_256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

struct string *hook_HASH_keccak256raw(struct string *str) {
  Keccak_256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest)); 
}

struct string *hook_HASH_keccak256(struct string *str) {
  Keccak_256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

struct string *hook_HASH_sha256raw(struct string *str) {
  SHA256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest)); 
}

struct string *hook_HASH_sha256(struct string *str) {
  SHA256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

struct string *hook_HASH_ripemd160raw(struct string *str) {
  RIPEMD160 h;
  unsigned char digest[20];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest)); 
}

struct string *hook_HASH_ripemd160(struct string *str) {
  RIPEMD160 h;
  unsigned char digest[20];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

struct string *hook_HASH_blake2compress(struct string *params) {
    if (len(params) != 213) {
        return hexEncode(nullptr, 0);
    }
    unsigned char *data = (unsigned char *)params->data;
    uint32_t  rounds = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    uint64_t *h      = (uint64_t *)&data[4];
    uint64_t *m      = (uint64_t *)&data[68];
    uint64_t *t      = (uint64_t *)&data[196];
    unsigned char f  = data[212];

    if (f > 1) return hexEncode(nullptr, 0);

    blake2b_compress(h, m, t, f, rounds);

    return hexEncode((unsigned char *)h, 64);
}

}
