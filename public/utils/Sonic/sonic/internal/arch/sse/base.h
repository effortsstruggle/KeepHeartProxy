// Copyright 2018-2019 The simdjson authors

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This file may have been modified by ByteDance authors. All ByteDance
// Modifications are Copyright 2022 ByteDance Authors.

#pragma once

#include <immintrin.h>
#include <Sonic/sonic/macro.h>

SONIC_PUSH_WESTMERE

namespace sonic_json {
namespace internal {
namespace sse {

// We sometimes call trailing_zero on inputs that are zero,
// but the algorithms do not end up using the returned value.
// Sadly, sanitizers are not smart enough to figure it out.

sonic_force_inline int TrailingZeroes(uint64_t input_num) {
  ////////
  // You might expect the next line to be equivalent to
  // return (int)_tzcnt_u64(input_num);
  // but the generated code differs and might be less efficient?
  ////////
  return __builtin_ctzll(input_num);
}

/* result might be undefined when input_num is zero */
sonic_force_inline uint64_t ClearLowestBit(uint64_t input_num) {
#if __BMI__
  return _blsr_u64(input_num);
#else
  return input_num & (input_num - 1);
#endif
}

/* result might be undefined when input_num is zero */
sonic_force_inline int LeadingZeroes(uint64_t input_num) {
  return __builtin_clzll(input_num);
}

sonic_force_inline long long int CountOnes(uint64_t input_num) {
  return __builtin_popcountll(input_num);
}

sonic_force_inline uint64_t PrefixXor(const uint64_t bitmask) {
#if __PCLMUL__
  __m128i all_ones = _mm_set1_epi8('\xFF');
  __m128i result =
      _mm_clmulepi64_si128(_mm_set_epi64x(0ULL, bitmask), all_ones, 0);
  return _mm_cvtsi128_si64(result);
#else
// #error "PCLMUL instruction set required. Missing option -mpclmul ?"
  return 0;
#endif
}

template <size_t ChunkSize>
sonic_force_inline void Xmemcpy(void* dst_, const void* src_, size_t chunks) {
  std::memcpy(dst_, src_, chunks * ChunkSize);
}

template <>
sonic_force_inline void Xmemcpy<16>(void* dst_, const void* src_,
                                    size_t chunks) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(dst_);
  const uint8_t* src = reinterpret_cast<const uint8_t*>(src_);
  for (size_t i = 0; i < chunks; i++) {
    // simd128<uint8_t> s(src);
    // s.store(dst);
    __m128i s = _mm_loadu_si128((__m128i const*)(src));
    _mm_storeu_si128((__m128i*)dst, s);
    src += 16, dst += 16;
  }
}

template <>
sonic_force_inline void Xmemcpy<32>(void* dst_, const void* src_,
                                    size_t chunks) {
  Xmemcpy<16>(dst_, src_, chunks * 2);
}

sonic_force_inline bool InlinedMemcmpEq(const void* _a, const void* _b,
                                        size_t s) {
  return std::memcmp(_a, _b, s) == 0;
}

sonic_force_inline int InlinedMemcmp(const void* _l, const void* _r, size_t s) {
  return std::memcmp(_l, _r, s);
}

}  // namespace sse
}  // namespace internal
}  // namespace sonic_json

SONIC_POP_TARGET
