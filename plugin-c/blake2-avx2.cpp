#include "blake2.h"

#if __AVX2__

#include <immintrin.h>

#include <cstring>

// adapted from https://github.com/sneves/blake2-avx2 under CC0

#define LOAD128(p) _mm_load_si128((__m128i *)(p))
#define STORE128(p, r) _mm_store_si128((__m128i *)(p), r)

#define LOADU128(p) _mm_loadu_si128((__m128i *)(p))
#define STOREU128(p, r) _mm_storeu_si128((__m128i *)(p), r)

#define LOAD(p) _mm256_load_si256((__m256i *)(p))
#define STORE(p, r) _mm256_store_si256((__m256i *)(p), r)

#define LOADU(p) _mm256_loadu_si256((__m256i *)(p))
#define STOREU(p, r) _mm256_storeu_si256((__m256i *)(p), r)

#define ROTATE16                                                               \
  _mm256_setr_epi8(2, 3, 4, 5, 6, 7, 0, 1, 10, 11, 12, 13, 14, 15, 8, 9, 2, 3, \
                   4, 5, 6, 7, 0, 1, 10, 11, 12, 13, 14, 15, 8, 9)

#define ROTATE24                                                               \
  _mm256_setr_epi8(3, 4, 5, 6, 7, 0, 1, 2, 11, 12, 13, 14, 15, 8, 9, 10, 3, 4, \
                   5, 6, 7, 0, 1, 2, 11, 12, 13, 14, 15, 8, 9, 10)

#define ADD(a, b) _mm256_add_epi64(a, b)
#define SUB(a, b) _mm256_sub_epi64(a, b)

#define XOR(a, b) _mm256_xor_si256(a, b)
#define AND(a, b) _mm256_and_si256(a, b)
#define OR(a, b) _mm256_or_si256(a, b)

#define ROT32(x) _mm256_shuffle_epi32((x), _MM_SHUFFLE(2, 3, 0, 1))
#define ROT24(x) _mm256_shuffle_epi8((x), ROTATE24)
#define ROT16(x) _mm256_shuffle_epi8((x), ROTATE16)
#define ROT63(x) _mm256_or_si256(_mm256_srli_epi64((x), 63), ADD((x), (x)))

#define BLAKE2B_LOAD_MSG_i_1(b0)                                          \
  do {                                                                    \
    b0 = _mm256_i32gather_epi64((void *)(m), LOAD128(&indices[i][0]), 8); \
  } while (0)

#define BLAKE2B_LOAD_MSG_i_2(b0)                                          \
  do {                                                                    \
    b0 = _mm256_i32gather_epi64((void *)(m), LOAD128(&indices[i][4]), 8); \
  } while (0)

#define BLAKE2B_LOAD_MSG_i_3(b0)                                          \
  do {                                                                    \
    b0 = _mm256_i32gather_epi64((void *)(m), LOAD128(&indices[i][8]), 8); \
  } while (0)

#define BLAKE2B_LOAD_MSG_i_4(b0)                                           \
  do {                                                                     \
    b0 = _mm256_i32gather_epi64((void *)(m), LOAD128(&indices[i][12]), 8); \
  } while (0)

#define BLAKE2B_LOAD_MSG_0_1(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m0, m1);    \
    t1 = _mm256_unpacklo_epi64(m2, m3);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_0_2(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m0, m1);    \
    t1 = _mm256_unpackhi_epi64(m2, m3);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_0_3(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m7, m4);    \
    t1 = _mm256_unpacklo_epi64(m5, m6);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_0_4(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m7, m4);    \
    t1 = _mm256_unpackhi_epi64(m5, m6);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_1_1(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m7, m2);    \
    t1 = _mm256_unpackhi_epi64(m4, m6);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_1_2(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m5, m4);    \
    t1 = _mm256_alignr_epi8(m3, m7, 8);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_1_3(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m2, m0);    \
    t1 = _mm256_blend_epi32(m5, m0, 0x33); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_1_4(b0)           \
  do {                                     \
    t0 = _mm256_alignr_epi8(m6, m1, 8);    \
    t1 = _mm256_blend_epi32(m3, m1, 0x33); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_2_1(b0)           \
  do {                                     \
    t0 = _mm256_alignr_epi8(m6, m5, 8);    \
    t1 = _mm256_unpackhi_epi64(m2, m7);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_2_2(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m4, m0);    \
    t1 = _mm256_blend_epi32(m6, m1, 0x33); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_2_3(b0)           \
  do {                                     \
    t0 = _mm256_alignr_epi8(m5, m4, 8);    \
    t1 = _mm256_unpackhi_epi64(m1, m3);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_2_4(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m2, m7);    \
    t1 = _mm256_blend_epi32(m0, m3, 0x33); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_3_1(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m3, m1);    \
    t1 = _mm256_unpackhi_epi64(m6, m5);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_3_2(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m4, m0);    \
    t1 = _mm256_unpacklo_epi64(m6, m7);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_3_3(b0)                            \
  do {                                                      \
    t0 = _mm256_alignr_epi8(m1, m7, 8);                     \
    t1 = _mm256_shuffle_epi32(m2, _MM_SHUFFLE(1, 0, 3, 2)); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0);                  \
  } while (0)

#define BLAKE2B_LOAD_MSG_3_4(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m4, m3);    \
    t1 = _mm256_unpacklo_epi64(m5, m0);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_4_1(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m4, m2);    \
    t1 = _mm256_unpacklo_epi64(m1, m5);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_4_2(b0)           \
  do {                                     \
    t0 = _mm256_blend_epi32(m3, m0, 0x33); \
    t1 = _mm256_blend_epi32(m7, m2, 0x33); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_4_3(b0)           \
  do {                                     \
    t0 = _mm256_alignr_epi8(m7, m1, 8);    \
    t1 = _mm256_alignr_epi8(m3, m5, 8);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_4_4(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m6, m0);    \
    t1 = _mm256_unpacklo_epi64(m6, m4);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_5_1(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m1, m3);    \
    t1 = _mm256_unpacklo_epi64(m0, m4);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_5_2(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m6, m5);    \
    t1 = _mm256_unpackhi_epi64(m5, m1);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_5_3(b0)           \
  do {                                     \
    t0 = _mm256_alignr_epi8(m2, m0, 8);    \
    t1 = _mm256_unpackhi_epi64(m3, m7);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_5_4(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m4, m6);    \
    t1 = _mm256_alignr_epi8(m7, m2, 8);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_6_1(b0)           \
  do {                                     \
    t0 = _mm256_blend_epi32(m0, m6, 0x33); \
    t1 = _mm256_unpacklo_epi64(m7, m2);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_6_2(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m2, m7);    \
    t1 = _mm256_alignr_epi8(m5, m6, 8);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_6_3(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m4, m0);    \
    t1 = _mm256_blend_epi32(m4, m3, 0x33); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_6_4(b0)                            \
  do {                                                      \
    t0 = _mm256_unpackhi_epi64(m5, m3);                     \
    t1 = _mm256_shuffle_epi32(m1, _MM_SHUFFLE(1, 0, 3, 2)); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0);                  \
  } while (0)

#define BLAKE2B_LOAD_MSG_7_1(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m6, m3);    \
    t1 = _mm256_blend_epi32(m1, m6, 0x33); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_7_2(b0)           \
  do {                                     \
    t0 = _mm256_alignr_epi8(m7, m5, 8);    \
    t1 = _mm256_unpackhi_epi64(m0, m4);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_7_3(b0)           \
  do {                                     \
    t0 = _mm256_blend_epi32(m2, m1, 0x33); \
    t1 = _mm256_alignr_epi8(m4, m7, 8);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_7_4(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m5, m0);    \
    t1 = _mm256_unpacklo_epi64(m2, m3);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_8_1(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m3, m7);    \
    t1 = _mm256_alignr_epi8(m0, m5, 8);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_8_2(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m7, m4);    \
    t1 = _mm256_alignr_epi8(m4, m1, 8);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_8_3(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m5, m6);    \
    t1 = _mm256_unpackhi_epi64(m6, m0);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_8_4(b0)           \
  do {                                     \
    t0 = _mm256_alignr_epi8(m1, m2, 8);    \
    t1 = _mm256_alignr_epi8(m2, m3, 8);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_9_1(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m5, m4);    \
    t1 = _mm256_unpackhi_epi64(m3, m0);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_9_2(b0)           \
  do {                                     \
    t0 = _mm256_unpacklo_epi64(m1, m2);    \
    t1 = _mm256_blend_epi32(m2, m3, 0x33); \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_9_3(b0)           \
  do {                                     \
    t0 = _mm256_unpackhi_epi64(m6, m7);    \
    t1 = _mm256_unpackhi_epi64(m4, m1);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_LOAD_MSG_9_4(b0)           \
  do {                                     \
    t0 = _mm256_blend_epi32(m5, m0, 0x33); \
    t1 = _mm256_unpacklo_epi64(m7, m6);    \
    b0 = _mm256_blend_epi32(t0, t1, 0xF0); \
  } while (0)

#define BLAKE2B_G1_V1(a, b, c, d, m) \
  do {                               \
    a = ADD(a, m);                   \
    a = ADD(a, b);                   \
    d = XOR(d, a);                   \
    d = ROT32(d);                    \
    c = ADD(c, d);                   \
    b = XOR(b, c);                   \
    b = ROT24(b);                    \
  } while (0)

#define BLAKE2B_G2_V1(a, b, c, d, m) \
  do {                               \
    a = ADD(a, m);                   \
    a = ADD(a, b);                   \
    d = XOR(d, a);                   \
    d = ROT16(d);                    \
    c = ADD(c, d);                   \
    b = XOR(b, c);                   \
    b = ROT63(b);                    \
  } while (0)

#define BLAKE2B_DIAG_V1(a, b, c, d)                           \
  do {                                                        \
    a = _mm256_permute4x64_epi64(a, _MM_SHUFFLE(2, 1, 0, 3)); \
    d = _mm256_permute4x64_epi64(d, _MM_SHUFFLE(1, 0, 3, 2)); \
    c = _mm256_permute4x64_epi64(c, _MM_SHUFFLE(0, 3, 2, 1)); \
  } while (0)

#define BLAKE2B_UNDIAG_V1(a, b, c, d)                         \
  do {                                                        \
    a = _mm256_permute4x64_epi64(a, _MM_SHUFFLE(0, 3, 2, 1)); \
    d = _mm256_permute4x64_epi64(d, _MM_SHUFFLE(1, 0, 3, 2)); \
    c = _mm256_permute4x64_epi64(c, _MM_SHUFFLE(2, 1, 0, 3)); \
  } while (0)

#define BLAKE2B_ROUND_V1(a, b, c, d, r, m) \
  do {                                     \
    __m256i b0;                            \
    BLAKE2B_LOAD_MSG_##r##_1(b0);          \
    BLAKE2B_G1_V1(a, b, c, d, b0);         \
    BLAKE2B_LOAD_MSG_##r##_2(b0);          \
    BLAKE2B_G2_V1(a, b, c, d, b0);         \
    BLAKE2B_DIAG_V1(a, b, c, d);           \
    BLAKE2B_LOAD_MSG_##r##_3(b0);          \
    BLAKE2B_G1_V1(a, b, c, d, b0);         \
    BLAKE2B_LOAD_MSG_##r##_4(b0);          \
    BLAKE2B_G2_V1(a, b, c, d, b0);         \
    BLAKE2B_UNDIAG_V1(a, b, c, d);         \
  } while (0)

static constexpr uint64_t blake2b_iv[8] = {
    0x6A09E667F3BCC908, 0xBB67AE8584CAA73B, 0x3C6EF372FE94F82B,
    0xA54FF53A5F1D36F1, 0x510E527FADE682D1, 0x9B05688C2B3E6C1F,
    0x1F83D9ABFB41BD6B, 0x5BE0CD19137E2179};

alignas(64) static constexpr uint32_t indices[12][16] = {
    {0, 2, 4, 6, 1, 3, 5, 7, 14, 8, 10, 12, 15, 9, 11, 13},
    {14, 4, 9, 13, 10, 8, 15, 6, 5, 1, 0, 11, 3, 12, 2, 7},
    {11, 12, 5, 15, 8, 0, 2, 13, 9, 10, 3, 7, 4, 14, 6, 1},
    {7, 3, 13, 11, 9, 1, 12, 14, 15, 2, 5, 4, 8, 6, 10, 0},
    {9, 5, 2, 10, 0, 7, 4, 15, 3, 14, 11, 6, 13, 1, 12, 8},
    {2, 6, 0, 8, 12, 10, 11, 3, 1, 4, 7, 15, 9, 13, 5, 14},
    {12, 1, 14, 4, 5, 15, 13, 10, 8, 0, 6, 9, 11, 7, 3, 2},
    {13, 7, 12, 3, 11, 14, 1, 9, 2, 5, 15, 8, 10, 0, 4, 6},
    {6, 14, 11, 0, 15, 9, 3, 8, 10, 12, 13, 1, 5, 2, 7, 4},
    {10, 8, 7, 1, 2, 4, 6, 5, 13, 15, 9, 3, 0, 11, 14, 12},
    {0, 2, 4, 6, 1, 3, 5, 7, 14, 8, 10, 12, 15, 9, 11, 13},
    {14, 4, 9, 13, 10, 8, 15, 6, 5, 1, 0, 11, 3, 12, 2, 7},
};
void blake2b_compress_avx2(uint64_t *h, uint64_t *m, uint64_t *t, char f,
                           uint32_t rounds) {
  uint64_t halign[8];
  memcpy(halign, h, sizeof(halign));

  __m256i a = LOAD(halign);
  __m256i b = LOAD(halign + 4);

  const __m256i m0 = _mm256_broadcastsi128_si256(LOADU128(m + 0));
  const __m256i m1 = _mm256_broadcastsi128_si256(LOADU128(m + 2));
  const __m256i m2 = _mm256_broadcastsi128_si256(LOADU128(m + 4));
  const __m256i m3 = _mm256_broadcastsi128_si256(LOADU128(m + 6));
  const __m256i m4 = _mm256_broadcastsi128_si256(LOADU128(m + 8));
  const __m256i m5 = _mm256_broadcastsi128_si256(LOADU128(m + 10));
  const __m256i m6 = _mm256_broadcastsi128_si256(LOADU128(m + 12));
  const __m256i m7 = _mm256_broadcastsi128_si256(LOADU128(m + 14));
  __m256i t0, t1;

  const __m256i iv0 = a;
  const __m256i iv1 = b;
  __m256i c = LOAD(blake2b_iv);
  __m256i d = XOR(LOAD(blake2b_iv + 4),
                  _mm256_set_epi64x(0, f ? 0xFFFFFFFFFFFFFFFF : 0, t[1], t[0]));

  for (uint32_t i = 0; i < rounds / 10; i++) {
    BLAKE2B_ROUND_V1(a, b, c, d, 0, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 1, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 2, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 3, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 4, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 5, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 6, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 7, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 8, m);
    BLAKE2B_ROUND_V1(a, b, c, d, 9, m);
  }
  for (uint32_t i = 0; i < rounds % 10; i++) {
    BLAKE2B_ROUND_V1(a, b, c, d, i, m);
  }

  a = XOR(a, c);
  b = XOR(b, d);
  a = XOR(a, iv0);
  b = XOR(b, iv1);
  STORE(halign, a);
  STORE(halign + 4, b);
  memcpy(h, halign, sizeof(halign));
}

#endif
