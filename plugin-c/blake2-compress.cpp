#include "blake2.h"

void blake2b_compress(uint64_t *h, uint64_t *m, uint64_t *t, char f,
                      uint32_t rounds) {
#if __AVX2__
  if (__builtin_cpu_supports("avx2")) {
    blake2b_compress_avx2(h, m, t, f, rounds);
  } else {
    blake2b_compress_generic(h, m, t, f, rounds);
  }
#else
  blake2b_compress_generic(h, m, t, f, rounds);
#endif
}
