#include <gmp.h>
#include <libff/algebra/curves/alt_bn128/alt_bn128_pp.hpp>
#include <libff/common/profiling.hpp>
#include "plugin_util.h"

using namespace libff;

extern "C" {

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

bool hook_BN128_bn128valid(g1point *pt) {
  initBN128();
  try {
    return getPoint(pt).is_well_formed();
  } catch (std::invalid_argument) {
    return false;
  }
}

// this code mirrors https://github.com/ethereum/aleth/blob/master/libdevcrypto/LibSnark.cpp
bool hook_BN128_bn128g2valid(g2point *pt) {
  initBN128();
  try {
    alt_bn128_G2 g2pt = getPoint(pt);
    return g2pt.is_well_formed() && -alt_bn128_G2::scalar_field::one() * g2pt + g2pt == alt_bn128_G2::zero();
  } catch (std::invalid_argument) {
    return false;
  }
}

g1point *hook_BN128_bn128add(g1point *pt1, g1point *pt2) {
  initBN128();
  return projectPoint(pt1->h.hdr, getPoint(pt1) + getPoint(pt2));
}

g1point *hook_BN128_bn128mul(g1point *pt, mpz_t scalar) {
  initBN128();
  bigint<alt_bn128_q_limbs> s(scalar);
  alt_bn128_G1 g1pt = getPoint(pt);
  return projectPoint(pt->h.hdr, s * g1pt);
}

mpz_ptr hook_LIST_size(list *);
void *hook_LIST_get_long(list *, ssize_t);

bool hook_BN128_bn128ate(list *g1, list *g2) {
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
