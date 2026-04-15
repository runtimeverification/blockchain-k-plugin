#include "p256verify.h"
#include "plugin_util.h"

extern "C" {

// Helper: Create public key EVP_PKEY object
static EVP_PKEY *p256_pubkey_from_coords(const unsigned char *qx, const unsigned char *qy);
// Helper: Build signature from raw (r, s) components
static unsigned char *p256_sig_to_der(const unsigned char *r_bytes, const unsigned char *s_bytes, int *out_len);
// Helper: Perform ECDSA verification with pre-hashed data
static int p256_verify_prehash(EVP_PKEY *pkey, const unsigned char *sig_der, int sig_len, const unsigned char *hash, size_t hash_len);

struct string *hook_KRYPTO_p256verify(struct string *input) {
// The precompile expects exactly 160 bytes: h || r || s || qx || qy
  if (len(input) != 160) {
    return allocString(0);
  }

  // 32-bytes slices
  const unsigned char *data = (unsigned char *)input->data;
  const unsigned char *h = data;
  const unsigned char *r_bytes = data + 32;
  const unsigned char *s_bytes = data + 64;
  const unsigned char *qx = data + 96;
  const unsigned char *qy = data + 128;

  EVP_PKEY *pkey = p256_pubkey_from_coords(qx, qy);
  if (!pkey) {
    return allocString(0);
  }

  int sig_der_len = 0;
  unsigned char *sig_der = p256_sig_to_der(r_bytes, s_bytes, &sig_der_len);
  if (!sig_der) {
    EVP_PKEY_free(pkey);
    return allocString(0);
  }

  int valid = p256_verify_prehash(pkey, sig_der, sig_der_len, h, 32);

  OPENSSL_free(sig_der);
  EVP_PKEY_free(pkey);

  if (valid) {
    unsigned char result[32] = {0};
    result[31] = 1;
    return raw(result, 32);
  }
  return allocString(0);
}

static EVP_PKEY *p256_pubkey_from_coords(const unsigned char *qx, const unsigned char *qy) {
  // Build SEC1 uncompressed format: 0x04 || x || y
  unsigned char pubkey_uncompressed[65];
  pubkey_uncompressed[0] = 0x04;
  memcpy(pubkey_uncompressed + 1, qx, 32);
  memcpy(pubkey_uncompressed + 33, qy, 32);

  EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
  if (!pctx) {
    return NULL;
  }

  if (EVP_PKEY_fromdata_init(pctx) <= 0) {
    EVP_PKEY_CTX_free(pctx);
    return NULL;
  }

  OSSL_PARAM params[] = {
    OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME, (char *)"prime256v1", 0),
    OSSL_PARAM_construct_octet_string(OSSL_PKEY_PARAM_PUB_KEY, pubkey_uncompressed, 65),
    OSSL_PARAM_construct_end()
  };

  EVP_PKEY *pkey = NULL;
  if (EVP_PKEY_fromdata(pctx, &pkey, EVP_PKEY_PUBLIC_KEY, params) <= 0) {
    EVP_PKEY_CTX_free(pctx);
    return NULL;
  }

  EVP_PKEY_CTX_free(pctx);
  return pkey;
}

static unsigned char *p256_sig_to_der(const unsigned char *r_bytes, const unsigned char *s_bytes, int *out_len) {
  ECDSA_SIG *sig = ECDSA_SIG_new();
  if (!sig) {
    return NULL;
  }

  BIGNUM *bn_r = BN_bin2bn(r_bytes, 32, NULL);
  BIGNUM *bn_s = BN_bin2bn(s_bytes, 32, NULL);

  if (!bn_r || !bn_s || !ECDSA_SIG_set0(sig, bn_r, bn_s)) {
    // Note: ECDSA_SIG_set0 takes ownership on success, so only free on failure
    if (bn_r) BN_free(bn_r);
    if (bn_s) BN_free(bn_s);
    ECDSA_SIG_free(sig);
    return NULL;
  }

  unsigned char *sig_der = NULL;
  *out_len = i2d_ECDSA_SIG(sig, &sig_der);
  ECDSA_SIG_free(sig);

  if (*out_len <= 0) {
    return NULL;
  }

  return sig_der;
}

static int p256_verify_prehash(EVP_PKEY *pkey, const unsigned char *sig_der, int sig_len, const unsigned char *hash, size_t hash_len) {
  EVP_PKEY_CTX *vctx = EVP_PKEY_CTX_new(pkey, NULL);
  if (!vctx) {
    return 0;
  }

  int result = 0;
  if (EVP_PKEY_verify_init(vctx) > 0) {
    result = (EVP_PKEY_verify(vctx, sig_der, sig_len, hash, hash_len) == 1);
  }

  EVP_PKEY_CTX_free(vctx);
  return result;
}

}