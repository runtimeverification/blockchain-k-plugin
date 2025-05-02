#include <cryptopp/keccak.h>
#include <cryptopp/ripemd.h>
#include <cryptopp/sha.h>
#include <cryptopp/sha3.h>
#include <openssl/evp.h>
#include <secp256k1_recovery.h>
#include <vector>

#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libff/common/profiling.hpp>

#include "blake2.h"
#include "blst.h"
#include "plugin_util.h"

using namespace CryptoPP;
using namespace libff;

extern "C" {

struct string *hook_KRYPTO_sha512raw(struct string *str) {
  SHA512 h;
  unsigned char digest[64];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return raw(digest, sizeof(digest));
}

struct string *hook_KRYPTO_sha512(struct string *str) {
  SHA512 h;
  unsigned char digest[64];
  h.CalculateDigest(digest, (unsigned char *)str->data, len(str));
  return hexEncode(digest, sizeof(digest));
}

void sha512_256(struct string *input, unsigned char *result) {
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  bool success = ctx != NULL &&
                 EVP_DigestInit_ex(ctx, EVP_sha512_256(), NULL) == 1 &&
                 EVP_DigestUpdate(ctx, input->data, len(input)) == 1 &&
                 EVP_DigestFinal_ex(ctx, result, NULL) == 1;
  if (!success)
    throw std::runtime_error("openssl sha512_256 EVP_Digest runtime error");
  EVP_MD_CTX_free(ctx);
}

struct string *hook_KRYPTO_sha512_256raw(struct string *str) {
  unsigned char digest[32];
  sha512_256(str, digest);
  return raw(digest, sizeof(digest));
}

struct string *hook_KRYPTO_sha512_256(struct string *str) {
  unsigned char digest[32];
  sha512_256(str, digest);
  return hexEncode(digest, sizeof(digest));
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
struct string *hook_KRYPTO_ecdsaRecover(struct string *str, mpz_t v,
                                        struct string *r, struct string *s) {
  if (len(str) != 32 || len(r) != 32 || len(s) != 32) {
    return allocString(0);
  }
  unsigned char sigArr[64];
  memcpy(sigArr, r->data, 32);
  memcpy(sigArr + 32, s->data, 32);
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_VERIFY |
                                                    SECP256K1_CONTEXT_SIGN);
  if (!mpz_fits_ulong_p(v)) {
    return allocString(0);
  }
  unsigned long v_long = mpz_get_ui(v);
  if (v_long < 27 || v_long > 28) {
    return allocString(0);
  }
  secp256k1_ecdsa_recoverable_signature sig;
  if (!secp256k1_ecdsa_recoverable_signature_parse_compact(ctx, &sig, sigArr,
                                                           v_long - 27)) {
    return allocString(0);
  }
  secp256k1_pubkey key;
  if (!secp256k1_ecdsa_recover(ctx, &key, &sig, (unsigned char *)str->data)) {
    return allocString(0);
  }
  unsigned char serialized[65];
  size_t len = sizeof(serialized);
  secp256k1_ec_pubkey_serialize(ctx, serialized, &len, &key,
                                SECP256K1_EC_UNCOMPRESSED);
  struct string *result = allocString(64);
  memcpy(result->data, serialized + 1, 64);
  return result;
}

struct string *hook_KRYPTO_ecdsaSign(struct string *mhash,
                                     struct string *prikey) {
  if (len(prikey) != 32 || len(mhash) != 32) {
    return hexEncode(nullptr, 0);
  }
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
  secp256k1_ecdsa_recoverable_signature sig;
  if (!secp256k1_ecdsa_sign_recoverable(ctx, &sig, (unsigned char *)mhash->data,
                                        (unsigned char *)prikey->data, NULL,
                                        NULL)) {
    return hexEncode(nullptr, 0);
  }
  unsigned char result[65];
  int recid;
  if (!secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, result,
                                                               &recid, &sig)) {
    return hexEncode(nullptr, 0);
  }
  result[64] = recid;
  return hexEncode(result, 65);
}

struct string *hook_KRYPTO_ecdsaPubKey(struct string *prikey) {
  if (len(prikey) != 32) {
    return hexEncode(nullptr, 0);
  }
  secp256k1_context *ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
  secp256k1_pubkey pubkey;
  if (!secp256k1_ec_pubkey_create(ctx, &pubkey,
                                  (unsigned char *)prikey->data)) {
    return hexEncode(nullptr, 0);
  }
  unsigned char keystring[65];
  size_t outputlen = 65;
  secp256k1_ec_pubkey_serialize(ctx, keystring, &outputlen, &pubkey,
                                SECP256K1_EC_UNCOMPRESSED);
  return hexEncode(keystring + 1, outputlen - 1);
}

struct string *hook_KRYPTO_blake2compress(struct string *params) {
  if (len(params) != 213) {
    return hexEncode(nullptr, 0);
  }
  unsigned char *data = (unsigned char *)params->data;
  uint32_t rounds = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
  uint64_t *h = (uint64_t *)&data[4];
  uint64_t *m = (uint64_t *)&data[68];
  uint64_t *t = (uint64_t *)&data[196];
  unsigned char f = data[212];

  if (f > 1) return hexEncode(nullptr, 0);

  blake2b_compress(h, m, t, f, rounds);

  return hexEncode((unsigned char *)h, 64);
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
  if (mpz_cmp_ui(pt->x0, 0) == 0 && mpz_cmp_ui(pt->x1, 0) == 0 &&
      mpz_cmp_ui(pt->y0, 0) == 0 && mpz_cmp_ui(pt->y1, 0) == 0) {
    return alt_bn128_G2::zero();
  }
  mpz_t mod;
  mpz_init(mod);
  alt_bn128_Fq::mod.to_mpz(mod);
  if (mpz_cmp(pt->x0, mod) >= 0 || mpz_cmp(pt->x1, mod) >= 0 ||
      mpz_cmp(pt->y0, mod) >= 0 || mpz_cmp(pt->y1, mod) >= 0) {
    throw std::invalid_argument("not a member of the field");
  }
  mpz_clear(mod);
  auto x = alt_bn128_Fq2{bigint<alt_bn128_q_limbs>(pt->x0),
                         bigint<alt_bn128_q_limbs>(pt->x1)};
  auto y = alt_bn128_Fq2{bigint<alt_bn128_q_limbs>(pt->y0),
                         bigint<alt_bn128_q_limbs>(pt->y1)};
  auto z = alt_bn128_Fq2::one();
  return alt_bn128_G2{x, y, z};
}

static g1point *projectPoint(uint64_t hdr, alt_bn128_G1 pt) {
  mpz_ptr x, y;
  if (pt.is_zero()) {
    x = (mpz_ptr)kore_alloc_integer(0);
    y = (mpz_ptr)kore_alloc_integer(0);
    mpz_init_set_ui(x, 0);
    mpz_init_set_ui(y, 0);
  } else {
    pt.to_affine_coordinates();
    x = (mpz_ptr)kore_alloc_integer(0);
    y = (mpz_ptr)kore_alloc_integer(0);
    mpz_init(x);
    mpz_init(y);
    pt.X.as_bigint().to_mpz(x);
    pt.Y.as_bigint().to_mpz(y);
  }
  struct g1point *g1pt = (struct g1point *)kore_alloc(sizeof(struct g1point));
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
  } catch (std::invalid_argument const &) {
    return false;
  }
}

// this code mirrors
// https://github.com/ethereum/aleth/blob/master/libdevcrypto/LibSnark.cpp
bool hook_KRYPTO_bn128g2valid(g2point *pt) {
  initBN128();
  try {
    alt_bn128_G2 g2pt = getPoint(pt);
    return g2pt.is_well_formed() &&
           -alt_bn128_G2::scalar_field::one() * g2pt + g2pt ==
               alt_bn128_G2::zero();
  } catch (std::invalid_argument const &) {
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

mpz_ptr hook_LIST_size(list *);
void *hook_LIST_get_long(list *, ssize_t);

bool hook_KRYPTO_bn128ate(list *g1, list *g2) {
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
        alt_bn128_precompute_G1(g1pt), alt_bn128_precompute_G2(g2pt));
    accum = accum * paired;
  }
  return alt_bn128_final_exponentiation(accum) == alt_bn128_GT::one();
}

mpz_ptr zero_mpz_ptr() {
  mpz_ptr m = (mpz_ptr)kore_alloc_integer(0);
  mpz_init_set_ui(m, 0);
  return m;
}

mpz_ptr blst_fp_to_mpz_ptr(const blst_fp *fp) {
  byte le[48];
  blst_lendian_from_fp(le, fp);

  mpz_ptr m = (mpz_ptr)kore_alloc_integer(48);
  mpz_init(m);
  mpz_import(m, 48, -1, 1, 0, 0, le);
  return m;
}

bool mpz_ptr_to_blst_fp(blst_fp *fp, const mpz_ptr m) {
  if (mpz_sizeinbase(m, 2) > 384) {
    return false;
  }
  byte le[48] = {};
  mpz_export(le, nullptr, -1, 1, 0, 0, m);
  blst_fp_from_lendian(fp, le);
  return true;
}

bool mpz_ptr_to_blst_scalar(blst_scalar *out, const mpz_ptr m) {
  if (mpz_sizeinbase(m, 2) > 256) {
    return false;
  }
  byte le[32] = {};
  mpz_export(le, nullptr, -1, 1, 0, 0, m);
  blst_scalar_from_le_bytes(out, le, (mpz_sizeinbase(m, 2) + 7) / 8);
  return true;
}

void blst_p1_affine_set_infinity(blst_p1_affine *blstp) {
  memset(blstp, 0, sizeof(*blstp));
}

void blst_p2_affine_set_infinity(blst_p2_affine *blstp) {
  memset(blstp, 0, sizeof(*blstp));
}

g1point* g1point_inf() {
  struct g1point *result = (struct g1point *)kore_alloc(sizeof(struct g1point));

  blockheader g1pointhdr =
      get_block_header_for_symbol((uint64_t)get_tag_for_symbol_name("Lblg1Point{}"));
  result->h = g1pointhdr;

  result->x = zero_mpz_ptr();
  result->y = zero_mpz_ptr();
  return result;
}

g1point* blst_p1_to_g1point(const blst_p1 *p) {
  struct g1point *result = (struct g1point *)kore_alloc(sizeof(struct g1point));

  blockheader g1pointhdr =
      get_block_header_for_symbol((uint64_t)get_tag_for_symbol_name("Lblg1Point{}"));
  result->h = g1pointhdr;

  if (blst_p1_is_inf(p)) {
    result->x = zero_mpz_ptr();
    result->y = zero_mpz_ptr();
    return result;
  }

  blst_p1_affine result_affine;
  blst_p1_to_affine(&result_affine, p);

  result->x = blst_fp_to_mpz_ptr(&result_affine.x);
  result->y = blst_fp_to_mpz_ptr(&result_affine.y);

  return result;
}

g2point* blst_p2_to_g2point(const blst_p2 *p) {
  struct g2point *result = (struct g2point *)kore_alloc(sizeof(struct g2point));

  blockheader g1pointhdr =
      get_block_header_for_symbol((uint64_t)get_tag_for_symbol_name("Lblg2Point{}"));
  result->h = g1pointhdr;

  if (blst_p2_is_inf(p)) {
    result->x0 = zero_mpz_ptr();
    result->y0 = zero_mpz_ptr();
    result->x1 = zero_mpz_ptr();
    result->y1 = zero_mpz_ptr();
    return result;
  }

  blst_p2_affine result_affine;
  blst_p2_to_affine(&result_affine, p);

  result->x0 = blst_fp_to_mpz_ptr(&result_affine.x.fp[0]);
  result->x1 = blst_fp_to_mpz_ptr(&result_affine.x.fp[1]);
  result->y0 = blst_fp_to_mpz_ptr(&result_affine.y.fp[0]);
  result->y1 = blst_fp_to_mpz_ptr(&result_affine.y.fp[1]);

  return result;
}

bool g1point_to_blst_p1_affine(blst_p1_affine *blstp, const g1point *g1p) {
  if (mpz_cmp_ui(g1p->x, 0) == 0 && mpz_cmp_ui(g1p->y, 0) == 0) {
    blst_p1_affine_set_infinity(blstp);
    return true;
  }

  if (!mpz_ptr_to_blst_fp(&blstp->x, g1p->x) ||
      !mpz_ptr_to_blst_fp(&blstp->y, g1p->y)) {
    return false;
  }

  return true;
}

bool g1point_to_blst_p1(blst_p1 *blstp, const g1point *g1p) {
  blst_p1_affine p_affine;

  if (!g1point_to_blst_p1_affine(&p_affine, g1p)) {
    return false;
  }

  blst_p1_from_affine(blstp, &p_affine);
  return true;
}

bool g2point_to_blst_p2_affine(blst_p2_affine *blstp, const g2point *g2p) {
  if (mpz_cmp_ui(g2p->x0, 0) == 0 && mpz_cmp_ui(g2p->y0, 0) == 0
      && mpz_cmp_ui(g2p->x1, 0) == 0 && mpz_cmp_ui(g2p->y1, 0) == 0) {
    blst_p2_affine_set_infinity(blstp);
    return true;
  }

  if (!mpz_ptr_to_blst_fp(&blstp->x.fp[0], g2p->x0) ||
      !mpz_ptr_to_blst_fp(&blstp->x.fp[1], g2p->x1) ||
      !mpz_ptr_to_blst_fp(&blstp->y.fp[0], g2p->y0) ||
      !mpz_ptr_to_blst_fp(&blstp->y.fp[1], g2p->y1)) {
    return false;
  }
  return true;
}


bool g2point_to_blst_p2(blst_p2 *blstp, const g2point *g2p) {
  blst_p2_affine p_affine;
  if (!g2point_to_blst_p2_affine(&p_affine, g2p)) {
    return false;
  }
  blst_p2_from_affine(blstp, &p_affine);
  return true;
}

struct g1point *hook_KRYPTO_bls12G1Add(g1point *first, g1point *second) {
  blst_p1 first_blst;
  blst_p1 second_blst;
  blst_p1 result_blst;

  if (!g1point_to_blst_p1(&first_blst, first)) {
    throw std::invalid_argument("Invalid point (first)");
  }
  if (!g1point_to_blst_p1(&second_blst, second)) {
    throw std::invalid_argument("Invalid point (second)");
  }

  blst_p1_add_or_double(&result_blst, &first_blst, &second_blst);
  return blst_p1_to_g1point(&result_blst);
}

struct g2point *hook_KRYPTO_bls12G2Add(g2point *first, g2point *second) {
  blst_p2 first_blst;
  blst_p2 second_blst;
  blst_p2 result_blst;

  if (!g2point_to_blst_p2(&first_blst, first)) {
    throw std::invalid_argument("Invalid point (first)");
  }
  if (!g2point_to_blst_p2(&second_blst, second)) {
    throw std::invalid_argument("Invalid point (second)");
  }

  blst_p2_add_or_double(&result_blst, &first_blst, &second_blst);
  return blst_p2_to_g2point(&result_blst);
}

struct g1point *hook_KRYPTO_bls12G1Mul(g1point *point, mpz_t scalar) {
  blst_scalar blstscalar;
  blst_p1 blstp;
  blst_p1 result;

  if (!mpz_ptr_to_blst_scalar(&blstscalar, scalar)) {
    throw std::invalid_argument("Invalid scalar");
  }
  if (!g1point_to_blst_p1(&blstp, point)) {
    throw std::invalid_argument("Invalid point");
  }

  size_t nbits = mpz_cmp_ui(scalar, 0) == 0 ? 0 : mpz_sizeinbase(scalar, 2);
  blst_p1_mult(&result, &blstp, blstscalar.b, nbits);
  return blst_p1_to_g1point(&result);
}

struct g1point *hook_KRYPTO_bls12G1Msm(list* scalars, list* g1) {
  mpz_ptr scalars_size = hook_LIST_size(scalars);
  mpz_ptr g1size = hook_LIST_size(g1);
  unsigned long scalars_size_long = mpz_get_ui(scalars_size);
  unsigned long g1size_long = mpz_get_ui(g1size);
  mpz_clear(scalars_size);
  mpz_clear(g1size);

  if (scalars_size_long != g1size_long) {
    throw std::invalid_argument("mismatched list sizes");
  }

  std::vector<blst_p1_affine> points(g1size_long);
  std::vector<blst_scalar> blst_scalars(g1size_long);

  int valid_point_count = 0;
  int first_nbits = 0;
  for (unsigned long i = 0; i < g1size_long; i++) {
    inj *injg1 = (inj *)hook_LIST_get_long(g1, i);
    g1point* g1pt = (g1point *)injg1->data;

    if (!g1point_to_blst_p1_affine(&points[valid_point_count], g1pt)) {
      throw std::invalid_argument("Invalid point");
    }
    if (blst_p1_affine_is_inf(&points[valid_point_count])) {
      continue;
    }

    inj *injs1 = (inj *)hook_LIST_get_long(scalars, i);
    mpz_ptr scalar = (mpz_ptr)injs1->data;
    if (valid_point_count == 0) {
      first_nbits = mpz_cmp_ui(scalar, 0) == 0 ? 0 : mpz_sizeinbase(scalar, 2);
    }
    if (!mpz_ptr_to_blst_scalar(&blst_scalars[valid_point_count], scalar)) {
      throw std::invalid_argument("Invalid scalar");
    }
    valid_point_count++;
  }

  if (valid_point_count == 0) {
    return g1point_inf();
  }
  if (valid_point_count == 1) {
    blst_p1 blstp;
    blst_p1_from_affine(&blstp, &points[0]);
    blst_p1 result;
    blst_p1_mult(&result, &blstp, blst_scalars[0].b, first_nbits);
    return blst_p1_to_g1point(&result);
  }

  size_t scratch_size = blst_p1s_mult_pippenger_scratch_sizeof(valid_point_count);
  std::vector<limb_t> scratch(scratch_size / sizeof(limb_t) + 1);

  const byte *scalars_arg[2] = {(byte *)blst_scalars.data(), NULL};
  const blst_p1_affine *points_arg[2] = {points.data(), NULL};
  blst_p1 result;
  blst_p1s_mult_pippenger
      ( &result
      , points_arg, valid_point_count
      , scalars_arg, sizeof(blst_scalars[0]) * 8
      , scratch.data()
      );
  return blst_p1_to_g1point(&result);
}

struct g2point *hook_KRYPTO_bls12G2Mul(g2point *point, mpz_t scalar) {
  blst_scalar blstscalar;
  blst_p2 blstp;
  blst_p2 result;

  if (!mpz_ptr_to_blst_scalar(&blstscalar, scalar)) {
    throw std::invalid_argument("Invalid scalar");
  }
  if (!g2point_to_blst_p2(&blstp, point)) {
    throw std::invalid_argument("Invalid point");
  }

  blst_p2_mult(&result, &blstp, blstscalar.b, mpz_sizeinbase(scalar, 2));
  return blst_p2_to_g2point(&result);
}

bool hook_KRYPTO_bls12G1InSubgroup(g1point *point) {
  blst_p1 blstp;

  if (!g1point_to_blst_p1(&blstp, point)) {
    return false;
  }

  if (blst_p1_is_inf(&blstp)) {
    return true;
  }

  return blst_p1_in_g1(&blstp);
}

bool hook_KRYPTO_bls12G2InSubgroup(g2point *point) {
  blst_p2 blstp;

  if (!g2point_to_blst_p2(&blstp, point)) {
    return false;
  }

  if (blst_p2_is_inf(&blstp)) {
    return true;
  }

  return blst_p2_in_g2(&blstp);
}


bool hook_KRYPTO_bls12G1OnCurve(g1point *point) {
  blst_p1 blstp;

  if (!g1point_to_blst_p1(&blstp, point)) {
    return false;
  }

  if (blst_p1_is_inf(&blstp)) {
    return true;
  }

  return blst_p1_on_curve(&blstp);
}

bool hook_KRYPTO_bls12G2OnCurve(g2point *point) {
  blst_p2 blstp;

  if (!g2point_to_blst_p2(&blstp, point)) {
    return false;
  }

  if (blst_p2_is_inf(&blstp)) {
    return true;
  }

  return blst_p2_on_curve(&blstp);
}

bool hook_KRYPTO_bls12PairingCheck(list* g1, list* g2) {
  // TODO: Most likely, this check can be improved with something that uses
  // blst_miller_loop_n, or blst_pairing_finalverify.
  mpz_ptr g1size = hook_LIST_size(g1);
  mpz_ptr g2size = hook_LIST_size(g2);
  unsigned long g1size_long = mpz_get_ui(g1size);
  unsigned long g2size_long = mpz_get_ui(g2size);
  mpz_clear(g1size);
  mpz_clear(g2size);
  if (g1size_long != g2size_long) {
    throw std::invalid_argument("mismatched list sizes");
  }

  blst_fp12 accum = *blst_fp12_one();
  for (unsigned long i = 0; i < g1size_long; i++) {
    inj *injg1 = (inj *)hook_LIST_get_long(g1, i);
    inj *injg2 = (inj *)hook_LIST_get_long(g2, i);
    g1point* g1pt = (g1point *)injg1->data;
    g2point* g2pt = (g2point *)injg2->data;

    blst_p1_affine p1_affine;
    blst_p2_affine p2_affine;
    blst_fp12 miller_result;

    if (!g1point_to_blst_p1_affine(&p1_affine, g1pt)) {
      throw std::invalid_argument("Invalid point (first)");
    }
    if (!g2point_to_blst_p2_affine(&p2_affine, g2pt)) {
      throw std::invalid_argument("Invalid point (second)");
    }

    blst_miller_loop(&miller_result, &p2_affine, &p1_affine);
    blst_fp12_mul(&accum, &accum, &miller_result);
  }

  blst_final_exp(&accum, &accum);

  return blst_fp12_is_one(&accum);
}

struct g1point *hook_KRYPTO_bls12MapFpToG1(mpz_t element) {
  blst_fp e;
  if (!mpz_ptr_to_blst_fp(&e, element)) {
    throw std::invalid_argument("Invalid field element");
  }

  blst_p1 result;
  blst_map_to_g1(&result, &e);
  return blst_p1_to_g1point(&result);
}

struct g2point *hook_KRYPTO_bls12MapFp2ToG2(mpz_t element0, mpz_t element1) {
  blst_fp2 e;
  if (!mpz_ptr_to_blst_fp(&e.fp[0], element0)) {
    throw std::invalid_argument("Invalid field element (first)");
  }
  if (!mpz_ptr_to_blst_fp(&e.fp[1], element1)) {
    throw std::invalid_argument("Invalid field element (second)");
  }

  blst_p2 result;
  blst_map_to_g2(&result, &e);
  return blst_p2_to_g2point(&result);
}

}
