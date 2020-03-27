#include <gmp.h>
#include <secp256k1_recovery.h>
#include "plugin_util.h"

extern "C" {

// matches evm and bitcoin's value of V, which is in the range 27-28
struct string *hook_SECP256K1_ecdsaRecover(struct string *str, mpz_t v, struct string *r, struct string *s) {
  if (len(str) != 32 || len(r) != 32 || len(s) != 32) {
    return hexEncode(nullptr, 0);
  }
  unsigned char sigArr[64];
  memcpy(sigArr, r->data, 32);
  memcpy(sigArr+32, s->data, 32);
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN);
  if (!mpz_fits_ulong_p(v)) {
    return hexEncode(nullptr, 0);
  }
  unsigned long v_long = mpz_get_ui(v);
  if (v_long < 27 || v_long > 28) {
    return hexEncode(nullptr, 0);
  }
  secp256k1_ecdsa_recoverable_signature sig;
  if (!secp256k1_ecdsa_recoverable_signature_parse_compact(ctx, &sig, sigArr, v_long - 27)) {
    return hexEncode(nullptr, 0);
  }
  secp256k1_pubkey key;
  if (!secp256k1_ecdsa_recover(ctx, &key, &sig, (unsigned char *)str->data)) {
    return hexEncode(nullptr, 0);
  }
  unsigned char serialized[65];
  size_t len = sizeof(serialized);
  secp256k1_ec_pubkey_serialize(ctx, serialized, &len, &key, SECP256K1_EC_UNCOMPRESSED);
  struct string *result = allocString(64);
  memcpy(result->data, serialized+1, 64);
  return result;
}

struct string *hook_SECP256K1_ecdsaSign(struct string *mhash, struct string *prikey) {
  if (len(prikey) != 32 || len(mhash) != 32) {
    return hexEncode(nullptr, 0);
  }
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
  secp256k1_ecdsa_recoverable_signature sig;
  if (!secp256k1_ecdsa_sign_recoverable(ctx, &sig, (unsigned char *)mhash->data, (unsigned char *)prikey->data, NULL, NULL)) {
    return hexEncode(nullptr, 0);
  }
  unsigned char result[65];
  int recid;
  if (!secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, result, &recid, &sig)) {
    return hexEncode(nullptr, 0);
  }
  result[64] = recid;
  return hexEncode(result, 65);
}

struct string *hook_SECP256K1_ecdsaPubKey(struct string *prikey) {
  if (len(prikey) != 32) {
    return hexEncode(nullptr, 0);
  }
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
  secp256k1_pubkey pubkey;
  if (!secp256k1_ec_pubkey_create(ctx, &pubkey, (unsigned char *)prikey->data)) {
    return hexEncode(nullptr, 0);
  }
  unsigned char keystring[65];
  size_t outputlen = 65;
  secp256k1_ec_pubkey_serialize(ctx, keystring, &outputlen, &pubkey, SECP256K1_EC_UNCOMPRESSED);
  return hexEncode(keystring+1, outputlen-1);
}

}
