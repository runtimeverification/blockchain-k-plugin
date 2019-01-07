#include <cstdint>
#include <crypto++/keccak.h>
#include <crypto++/ripemd.h>
#include <crypto++/sha.h>
#include <crypto++/sha3.h>
#include <secp256k1_recovery.h>
#include <gmp.h>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libff/common/profiling.hpp>
#include "runtime/alloc.h"
#include "runtime/header.h"

using namespace CryptoPP;
using namespace libff;

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

struct string *hook_KRYPTO_sha3raw(struct string *str) {
  SHA3_256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest)); 
}

struct string *hook_KRYPTO_sha3(struct string *str) {
  SHA3_256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

struct string *hook_KRYPTO_keccak256raw(struct string *str) {
  Keccak_256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest)); 
}

struct string *hook_KRYPTO_keccak256(struct string *str) {
  Keccak_256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

struct string *hook_KRYPTO_sha256raw(struct string *str) {
  SHA256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest)); 
}

struct string *hook_KRYPTO_sha256(struct string *str) {
  SHA256 h;
  unsigned char digest[32];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

struct string *hook_KRYPTO_ripemd160raw(struct string *str) {
  RIPEMD160 h;
  unsigned char digest[20];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest)); 
}

struct string *hook_KRYPTO_ripemd160(struct string *str) {
  RIPEMD160 h;
  unsigned char digest[20];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

// matches evm and bitcoin's value of V, which is in the range 27-28
struct string *hook_KRYPTO_ecdsaRecover(struct string *str, mpz_t v, struct string *r, struct string *s) {
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

struct g1point {
  struct blockheader h;
  mpz_ptr x;
  mpz_ptr y;
};

struct g2point {
  struct blockheader h;
  mpz_ptr x0;
  mpz_ptr x1;
  mpz_ptr y0;
  mpz_ptr y1;
};

struct inj {
  struct blockheader h;
  void *data;
};

bool bn128_initialized = false;

extern "C++" {

static void initBN128() {
  if (bn128_initialized) {
    return;
  }
  alt_bn128_pp::init_public_params();
  inhibit_profiling_info = true;
  bn128_initialized = true;
}

static alt_bn128_G1 getPoint(g1point *pt) {
  if (mpz_cmp_ui(pt->x, 0) == 0 && mpz_cmp_ui(pt->y, 0) == 0) {
    return alt_bn128_G1::zero();
  }
  mpz_t mod;
  mpz_init(mod);
  alt_bn128_Fq::mod.to_mpz(mod);
  if (mpz_cmp(pt->x, mod) >= 0 || mpz_cmp(pt->y, mod) >= 0) {
    throw std::invalid_argument("not a member of the field");
  }
  mpz_clear(mod);
  auto x = bigint<alt_bn128_q_limbs>(pt->x);
  auto y = bigint<alt_bn128_q_limbs>(pt->y);
  auto z = bigint<alt_bn128_q_limbs>(1);
  return alt_bn128_G1{x, y, z};
}

static alt_bn128_G2 getPoint(g2point *pt) {
  if (mpz_cmp_ui(pt->x0, 0) == 0 && mpz_cmp_ui(pt->x1, 0) == 0 
      && mpz_cmp_ui(pt->y0, 0) == 0 && mpz_cmp_ui(pt->y1, 0) == 0) {
    return alt_bn128_G2::zero();
  }
  mpz_t mod;
  mpz_init(mod);
  alt_bn128_Fq::mod.to_mpz(mod);
  if (mpz_cmp(pt->x0, mod) >= 0 || mpz_cmp(pt->x1, mod) >= 0
      || mpz_cmp(pt->y0, mod) >= 0 || mpz_cmp(pt->y1, mod) >= 0) {
    throw std::invalid_argument("not a member of the field");
  }
  mpz_clear(mod);
  auto x = alt_bn128_Fq2{bigint<alt_bn128_q_limbs>(pt->x0), bigint<alt_bn128_q_limbs>(pt->x1)};
  auto y = alt_bn128_Fq2{bigint<alt_bn128_q_limbs>(pt->y0), bigint<alt_bn128_q_limbs>(pt->y1)};
  auto z = alt_bn128_Fq2::one();
  return alt_bn128_G2{x, y, z};
}

static g1point *projectPoint(uint64_t hdr, alt_bn128_G1 pt) {
  mpz_ptr x, y;
  if (pt.is_zero()) {
    x = (mpz_ptr) malloc(sizeof(*x));
    y = (mpz_ptr) malloc(sizeof(*y));
    mpz_init_set_ui(x, 0);
    mpz_init_set_ui(y, 0);
  } else {
    pt.to_affine_coordinates();
    x = (mpz_ptr) malloc(sizeof(*x));
    y = (mpz_ptr) malloc(sizeof(*y));
    mpz_init(x);
    mpz_init(y);
    pt.X.as_bigint().to_mpz(x);
    pt.Y.as_bigint().to_mpz(y);
  }
  struct g1point *g1pt = (struct g1point *)koreAlloc(sizeof(struct g1point));
  g1pt->h.hdr = hdr;
  g1pt->x = x;
  g1pt->y = y;
  return g1pt;
}
}

bool hook_KRYPTO_bn128valid(g1point *pt) {
  initBN128();
  try {
    return getPoint(pt).is_well_formed();
  } catch (std::invalid_argument) {
    return false;
  }
}

// this code mirrors https://github.com/ethereum/aleth/blob/master/libdevcrypto/LibSnark.cpp
bool hook_KRYPTO_bn128g2valid(g2point *pt) {
  initBN128();
  try {
    alt_bn128_G2 g2pt = getPoint(pt);
    return g2pt.is_well_formed() && -alt_bn128_G2::scalar_field::one() * g2pt + g2pt == alt_bn128_G2::zero();
  } catch (std::invalid_argument) {
    return false;
  }
}

g1point *hook_KRYPTO_bn128add(g1point *pt1, g1point *pt2) {
  initBN128();
  return projectPoint(pt1->h.hdr, getPoint(pt1) + getPoint(pt2));
}

g1point *hook_KRYPTO_bn128mul(g1point *pt, mpz_t scalar) {
  initBN128();
  bigint<alt_bn128_q_limbs> s(scalar);
  alt_bn128_G1 g1pt = getPoint(pt);
  return projectPoint(pt->h.hdr, s * g1pt);
}

struct list;
mpz_ptr hook_LIST_size(struct list *);
void *hook_LIST_get_long(struct list *, ssize_t);

bool hook_KRYPTO_bn128ate(struct list *g1, struct list *g2) {
  initBN128();
  mpz_ptr g1size = hook_LIST_size(g1);
  mpz_ptr g2size = hook_LIST_size(g2);
  unsigned long g1size_long = mpz_get_ui(g1size);
  unsigned long g2size_long = mpz_get_ui(g2size);
  mpz_clear(g1size);
  mpz_clear(g2size);
  if (g1size_long != g2size_long) {
    throw std::invalid_argument("mismatched list sizes");
  }
  mpz_t bigi;
  mpz_init(bigi);
  alt_bn128_Fq12 accum = alt_bn128_Fq12::one();
  for (unsigned long i = 0; i < g1size_long; i++) {
    inj *injg1 = (inj *)hook_LIST_get_long(g1, i);
    inj *injg2 = (inj *)hook_LIST_get_long(g2, i);
    alt_bn128_G1 g1pt = getPoint((g1point *)injg1->data);
    alt_bn128_G2 g2pt = getPoint((g2point *)injg2->data);
    if (g1pt.is_zero() || g2pt.is_zero()) {
      continue;
    }
    alt_bn128_Fq12 paired = alt_bn128_miller_loop(
        alt_bn128_precompute_G1(g1pt),
        alt_bn128_precompute_G2(g2pt));
    accum = accum*paired;
  }
  return alt_bn128_final_exponentiation(accum) == alt_bn128_GT::one();
}

}
