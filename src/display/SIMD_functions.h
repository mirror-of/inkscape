#ifndef SIMD_FUNCTIONS_H
#define SIMD_FUNCTIONS_H

#include <immintrin.h>
#include <emmintrin.h>
#ifdef __SSE4_1__
  #include <smmintrin.h>
#endif

#ifndef __SSE4_1__

inline __m128i Select(__m128i a, __m128i b, __m128i selectors)
{
    return _mm_or_si128(_mm_andnot_si128(selectors, a), _mm_and_si128(selectors, b));
}

inline __m128 _mm_blendv_ps(__m128 a, __m128 b, __m128 selectors)
{
    return _mm_or_ps(_mm_andnot_ps(selectors, a), _mm_and_ps(selectors, b));
}

inline __m128i _mm_max_epi32(__m128i a, __m128i b)
{
    return Select(b, a, _mm_cmpgt_epi32(a, b));
}

inline __m128i _mm_min_epi32(__m128i a, __m128i b)
{
    return Select(a, b, _mm_cmpgt_epi32(a, b));
}

#else
inline __m128i Select(__m128i a, __m128i b, __m128i selectors)
{
  return  _mm_blendv_epi8(a, b, selectors);
}
#endif

#endif
