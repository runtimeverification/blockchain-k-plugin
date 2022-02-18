#include <cryptopp/blake2.h>
#include <cryptopp/xed25519.h>
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

bool hook_KRYPTO_ed25519verify(struct string *key, struct string *message, struct string *sig) {
  if(len(key) != ed25519Verifier::PUBLIC_KEYLENGTH || len(sig) != ed25519Verifier::SIGNATURE_LENGTH) {
    return false;
  }

  ed25519Verifier v((unsigned char *)&key->data[0]);

  return v.VerifyMessage((unsigned char *)&message->data[0], len(message), (unsigned char *)&sig->data[0], len(sig));
}

}
