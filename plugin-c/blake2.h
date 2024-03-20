#ifndef BLAKE2_H
#define BLAKE2_H

#include <cstdint>

void blake2b_compress(uint64_t *h, uint64_t *m, uint64_t *t, char f,
                      uint32_t rounds);
void blake2b_compress_generic(uint64_t *h, uint64_t *m, uint64_t *t, char f,
                              uint32_t rounds);
#if __AVX2__
void blake2b_compress_avx2(uint64_t *h, uint64_t *m, uint64_t *t, char f,
                           uint32_t rounds);
#endif

#endif
