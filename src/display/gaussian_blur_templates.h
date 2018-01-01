#ifdef __SSE3__
  // not any faster, at least on Haswell?
  #define _mm_loadu_pd(p) _mm_castsi128_pd(_mm_lddqu_si128((__m128i *)(p)))
  #define _mm_loadu_ps(p) _mm_castsi128_ps(_mm_lddqu_si128((__m128i *)(p)))
  #define _mm_loadu_si128(p) _mm_lddqu_si128(p)

  #define _mm256_loadu_pd(p) _mm256_castsi256_pd(_mm256_lddqu_si256((__m256i *)(p)))
  #define _mm256_loadu_ps(p) _mm256_castsi256_ps(_mm256_lddqu_si256((__m256i *)(p)))
  #define _mm256_loadu_si128(p) _mm256_lddqu_si256(p)
#else
  #undef _mm_loadu_pd
  #undef _mm_loadu_ps
  #undef _mm_loadu_si128
  #undef _mm256_loadu_pd
  #undef _mm256_loadu_ps
  #undef _mm256_loadu_si128
#endif

template <typename AnyType>
struct MyTraits
{
};

template <>
struct MyTraits<float>
{
#ifdef __AVX__
  typedef __m256 SIMDtype;
#else
  typedef __m128 SIMDtype;
#endif
};

template <>
struct MyTraits<int16_t>
{
#ifdef __AVX2__
  typedef __m256i SIMDtype;
#else
  typedef __m128i SIMDtype;
#endif
};

template <>
struct MyTraits<double>
{
#ifdef __AVX__
  typedef __m256d SIMDtype;
#else
  typedef __m128d SIMDtype;
#endif
};

#if defined(__AVX__) && defined(__GNUC__)
FORCE_INLINE __m256 _mm256_setr_m128(__m128 lo, __m128 hi)
{
  return _mm256_insertf128_ps(_mm256_castps128_ps256(lo), hi, 1);
}

FORCE_INLINE __m256i _mm256_setr_m128i(__m128i lo, __m128i hi)
{
  return _mm256_insertf128_si256(_mm256_castsi128_si256(lo), hi, 1);
}

FORCE_INLINE __m256d _mm256_setr_m128d(__m128d lo, __m128d hi)
{
  return _mm256_insertf128_pd(_mm256_castpd128_pd256(lo), hi, 1);
}
#endif

#ifdef __FMA__
//#pragma GCC push_options
//#pragma GCC target("fma")
FORCE_INLINE __m128 MultiplyAdd(__m128 a, __m128 b, __m128 c)
{
  return _mm_fmadd_ps(a, b, c);
}

FORCE_INLINE __m256 MultiplyAdd(__m256 a, __m256 b, __m256 c)
{
  return _mm256_fmadd_ps(a, b, c);
}

FORCE_INLINE __m256d MultiplyAdd(__m256d a, __m256d b, __m256d c)
{
  return _mm256_fmadd_pd(a, b, c);
}
//#pragma GCC pop_options
#endif

#ifndef __GNUC__
FORCE_INLINE __m128d operator + (__m128d a, __m128d b)
{
  return _mm_add_pd(a, b);
}

FORCE_INLINE __m128d operator - (__m128d a, __m128d b)
{
  return _mm_sub_pd(a, b);
}

FORCE_INLINE __m128d operator * (__m128d a, __m128d b)
{
  return _mm_mul_pd(a, b);
}

FORCE_INLINE __m256d operator + (__m256d a, __m256d b)
{
  return _mm256_add_pd(a, b);
}

FORCE_INLINE __m256d operator - (__m256d a, __m256d b)
{
  return _mm256_sub_pd(a, b);
}

FORCE_INLINE __m256d operator * (__m256d a, __m256d b)
{
  return _mm256_mul_pd(a, b);
}

FORCE_INLINE __m128 operator + (__m128 a, __m128 b)
{
  return _mm_add_ps(a, b);
}

FORCE_INLINE __m128 operator - (__m128 a, __m128 b)
{
  return _mm_sub_ps(a, b);
}

FORCE_INLINE __m128 operator * (__m128 a, __m128 b)
{
  return _mm_mul_ps(a, b);
}

FORCE_INLINE __m256 operator + (__m256 a, __m256 b)
{
  return _mm256_add_ps(a, b);
}

FORCE_INLINE __m256 operator - (__m256 a, __m256 b)
{
  return _mm256_sub_ps(a, b);
}

FORCE_INLINE __m256 operator * (__m256 a, __m256 b)
{
  return _mm256_mul_ps(a, b);
}
#endif

#ifdef __AVX__
FORCE_INLINE float ExtractElement0(__m256 x)
{
  return _mm_cvtss_f32(_mm256_castps256_ps128(x));
}

FORCE_INLINE double ExtractElement0(__m256d x)
{
  return _mm_cvtsd_f64(_mm256_castpd256_pd128(x));
}
#endif

FORCE_INLINE float ExtractElement0(__m128 x)
{
  return _mm_cvtss_f32(x);
}

FORCE_INLINE double ExtractElement0(__m128d x)
{
  return _mm_cvtsd_f64(x);
}

template<int SIZE>
static void calcTriggsSdikaInitialization(double const M[N*N], float uold[N][SIZE], float const uplus[SIZE], float const vplus[SIZE], float const alpha, float vold[N][SIZE])
{
    __m128 v4f_alpha = _mm_set1_ps(alpha);
    ssize_t c;
    for (c = 0; c + 4 <= SIZE; c += 4)
    {
    	__m128  uminp[N];
        for(ssize_t i=0; i<N; i++)
           uminp[i] = _mm_loadu_ps(&uold[i][c]) - _mm_loadu_ps(&uplus[c]);

        __m128 v4f_vplus = _mm_loadu_ps(&vplus[c]);

        for(ssize_t i=0; i<N; i++)
        {
            __m128 voldf = _mm_setzero_ps();
            for(ssize_t j=0; j<N; j++)
            {
                voldf = voldf + uminp[j] * _mm_set1_ps(M[i*N+j]);
            }
            // Properly takes care of the scaling coefficient alpha and vplus (which is already appropriately scaled)
            // This was arrived at by starting from a version of the blur filter that ignored the scaling coefficient
            // (and scaled the final output by alpha^2) and then gradually reintroducing the scaling coefficient.
            _mm_storeu_ps(&vold[i][c], voldf * v4f_alpha + v4f_vplus);
        }
    }
    while (c < SIZE)
    {
        double uminp[N];
        for(ssize_t i=0; i<N; i++) uminp[i] = uold[i][c] - uplus[c];
        for(ssize_t i=0; i<N; i++) {
            double voldf = 0;
            for(ssize_t j=0; j<N; j++) {
                voldf += uminp[j]*M[i*N+j];
            }
            // Properly takes care of the scaling coefficient alpha and vplus (which is already appropriately scaled)
            // This was arrived at by starting from a version of the blur filter that ignored the scaling coefficient
            // (and scaled the final output by alpha^2) and then gradually reintroducing the scaling coefficient.
            vold[i][c] = voldf*alpha;
            vold[i][c] += vplus[c];
        }
        ++c;
    }
}

template<int SIZE>
static void calcTriggsSdikaInitialization(double const M[N*N], double uold[N][SIZE], double const uplus[SIZE], double const vplus[SIZE], double const alpha, double vold[N][SIZE])
{
    __m128d v2f_alpha = _mm_set1_pd(alpha);
    ssize_t c;
    for (c = 0; c <= SIZE - 2; c += 2)
    {
    	__m128d  uminp[N];
        for(ssize_t i=0; i<N; i++)
           uminp[i] = _mm_loadu_pd(&uold[i][c]) - _mm_loadu_pd(&uplus[c]);

        __m128d v2f_vplus = _mm_loadu_pd(&vplus[c]);

        for(ssize_t i=0; i<N; i++)
        {
            __m128d voldf = _mm_setzero_pd();
            for(ssize_t j=0; j<N; j++)
            {
                voldf = voldf + uminp[j] * _mm_load1_pd(&M[i*N+j]);
            }
            // Properly takes care of the scaling coefficient alpha and vplus (which is already appropriately scaled)
            // This was arrived at by starting from a version of the blur filter that ignored the scaling coefficient
            // (and scaled the final output by alpha^2) and then gradually reintroducing the scaling coefficient.
            _mm_storeu_pd(&vold[i][c], voldf * v2f_alpha + v2f_vplus);
        }
    }
    while (c < SIZE)
    {
        double uminp[N];
        for(ssize_t i=0; i<N; i++) uminp[i] = uold[i][c] - uplus[c];
        for(ssize_t i=0; i<N; i++) {
            double voldf = 0;
            for(ssize_t j=0; j<N; j++) {
                voldf += uminp[j]*M[i*N+j];
            }
            // Properly takes care of the scaling coefficient alpha and vplus (which is already appropriately scaled)
            // This was arrived at by starting from a version of the blur filter that ignored the scaling coefficient
            // (and scaled the final output by alpha^2) and then gradually reintroducing the scaling coefficient.
            vold[i][c] = voldf*alpha;
            vold[i][c] += vplus[c];
        }
        ++c;
    }
}

FORCE_INLINE __m128i PartialVectorMask(ssize_t n)
{
  return _mm_loadu_si128((__m128i *)&PARTIAL_VECTOR_MASK[sizeof(PARTIAL_VECTOR_MASK) / 2 - n]);
}

#ifdef __AVX__
FORCE_INLINE __m256i PartialVectorMask32(ssize_t n)
{
  return _mm256_loadu_si256((__m256i *)&PARTIAL_VECTOR_MASK[sizeof(PARTIAL_VECTOR_MASK) / 2 - n]);
}
#endif

#if !defined(_WIN32) && !defined(_MSC_VER)
  // using _mm_maskmove_si64() is preferable to _mm_maskmoveu_si128(), but for some reason on Windows, it causes memory corruption
  // could it be due to mixing x87 and MMX?
  #define CAN_USE_MMX
#endif

#ifdef CAN_USE_MMX
// return __m64 so that it can be used by _mm_movemask_si64()
FORCE_INLINE __m64 PartialVectorMask8(ssize_t n)
{
  return _mm_cvtsi64_m64(*(int64_t *)&PARTIAL_VECTOR_MASK[sizeof(PARTIAL_VECTOR_MASK) / 2 - n]);
}
#else
FORCE_INLINE __m128i PartialVectorMask8(ssize_t n)
{
  return _mm_loadl_epi64((__m128i *)&PARTIAL_VECTOR_MASK[sizeof(PARTIAL_VECTOR_MASK) / 2 - n]);
}
#endif

#ifdef __AVX__
FORCE_INLINE __m256d LoadDoubles(__m256d &out, double *x)
{
  return out = _mm256_loadu_pd(x);
}

FORCE_INLINE __m256d LoadDoubles(__m256d &out, float *x)
{
  return out = _mm256_cvtps_pd(_mm_loadu_ps(x));
}

FORCE_INLINE __m256d LoadDoubles(__m256d &out, uint8_t *x)
{
  return out = _mm256_cvtepi32_pd(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(int32_t *)x)));
}

FORCE_INLINE __m256d LoadDoubles(__m256d &out, uint16_t *x)
{
  return out = _mm256_cvtepi32_pd(_mm_cvtepu16_epi32(_mm_loadl_epi64((__m128i *)x)));
}

FORCE_INLINE __m256 LoadFloats(__m256 &out, float *x)    // seriously? compiler needs to be told to inline this when PIC on?
{
  return out = _mm256_loadu_ps(x);
}

FORCE_INLINE __m256 LoadFloats(__m256 &out, uint8_t *x)
{
    __m128i temp = _mm_loadl_epi64((__m128i *)x);
#ifdef __AVX2__
    out = _mm256_cvtepi32_ps(_mm256_cvtepu8_epi32(temp));
#else
    out = _mm256_cvtepi32_ps(_mm256_setr_m128i(_mm_cvtepu8_epi32(temp), _mm_cvtepu8_epi32(_mm_shuffle_epi32(temp, _MM_SHUFFLE(0, 0, 0, 1)))));
#endif
    return out;
}

FORCE_INLINE __m256 LoadFloats(__m256 &out, uint16_t *x)
{
    __m128i temp = _mm_loadu_si128((__m128i *)x);
    __m256i i32;
#ifdef __AVX2__
    i32 = _mm256_cvtepu16_epi32(temp);
#else
    __m128i zero = _mm_setzero_si128();
    i32 = _mm256_setr_m128i(_mm_unpacklo_epi16(temp, zero), _mm_unpackhi_epi16(temp, zero));
#endif
  return out = _mm256_cvtepi32_ps(i32);
}

template <bool partial = false>   // no, this parameter isn't redundant - without it, there will be a redundant n == 4 check when partial = 0
FORCE_INLINE void StoreDoubles(double *out, __m256d x, ssize_t n = 4)
{
  if (partial)
    _mm256_maskstore_pd(out, PartialVectorMask32(n * sizeof(double)), x);
  else
    _mm256_storeu_pd(out, x);
}

template <bool partial = false>
FORCE_INLINE void StoreDoubles(float *out, __m256d x, ssize_t n = 4)
{
  __m128 f32 = _mm256_cvtpd_ps(x);
  if (partial)
    _mm_maskstore_ps(out, PartialVectorMask(n * sizeof(float)), f32);
  else
    _mm_storeu_ps(out, f32);
}

template <bool partial = false>
FORCE_INLINE void StoreDoubles(uint16_t *out, __m256d x, ssize_t n = 4)
{
  __m128i i32 = _mm256_cvtpd_epi32(x),
          u16 = _mm_packus_epi32(i32, i32);
  if (partial)
  {
#ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(u16), PartialVectorMask8(n * sizeof(int16_t)), (char *)out);
#else
    _mm_maskmoveu_si128(u16, PartialVectorMask8(n * sizeof(int16_t)), (char *)out);
#endif
  }
  else
    _mm_storel_epi64((__m128i *)out, u16);
}

template <bool partial = false>
FORCE_INLINE void StoreDoubles(uint8_t *out, __m256d x, ssize_t n = 4)
{
  __m128i i32 = _mm256_cvtpd_epi32(x),
          u16 = _mm_packus_epi32(i32, i32),
           u8 = _mm_packus_epi16(u16, u16);
  if (partial)
  {
#ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(u8), PartialVectorMask8(n), (char *)out);    
#else
    _mm_maskmoveu_si128(u8, PartialVectorMask8(n), (char *)out);
#endif
  }
  else
    *(int32_t *)out = _mm_cvtsi128_si32(u8);
}

FORCE_INLINE void StoreDoubles(uint8_t *out, __m256d x)
{
  __m128i vInt = _mm_cvtps_epi32(_mm256_cvtpd_ps(x));
  *(int32_t *)out = _mm_cvtsi128_si32(_mm_packus_epi16(_mm_packus_epi32(vInt, vInt), vInt));
}

template <bool partial = false>   // no, this parameter isn't redundant - without it, there will be a redundant n == 8 check when partial = 0
FORCE_INLINE void StoreFloats(float *out, __m256 x, ssize_t n = 8)
{
  if (partial)
    _mm256_maskstore_ps(out, PartialVectorMask32(n * sizeof(float)), x);
  else
    _mm256_storeu_ps(out, x);
}

template <bool partial = false>
FORCE_INLINE void StoreFloats(uint16_t *out, __m256 x, ssize_t n = 8)
{
  __m256i i32 = _mm256_cvtps_epi32(x);
  __m128i u16 = _mm_packus_epi32(_mm256_castsi256_si128(i32), _mm256_extractf128_si256(i32, 1));
  if (partial)
    _mm_maskmoveu_si128(u16, PartialVectorMask(n * sizeof(int16_t)), (char *)out);
  else
    _mm_storeu_si128((__m128i *)out, u16);
}

template <bool partial = false>
FORCE_INLINE void StoreFloats(uint8_t *out, __m256 x, ssize_t n = 8)
{
  __m256i i32 = _mm256_cvtps_epi32(x);
  __m128i i32Hi = _mm256_extractf128_si256(i32, 1),
           u16 = _mm_packus_epi32(_mm256_castsi256_si128(i32), i32Hi),
           u8 = _mm_packus_epi16(u16, u16);
  if (partial)
  {
#ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(u8), PartialVectorMask8(n), (char *)out);
#else
    _mm_maskmoveu_si128(u8, PartialVectorMask8(n), (char *)out);
#endif
  }
  else
    _mm_storel_epi64((__m128i *)out, u8);
}
#endif

#ifdef __AVX__
FORCE_INLINE __m256 BroadcastSIMD(__m256 &out, float x)
{
  return out = _mm256_set1_ps(x);
}

FORCE_INLINE __m256d BroadcastSIMD(__m256d &out, double x)
{
  return out = _mm256_set1_pd(x);
}

FORCE_INLINE __m256i BroadcastSIMD(__m256i &out, int16_t x)
{
  return out = _mm256_set1_epi16(x);
}
#endif

FORCE_INLINE __m128 BroadcastSIMD(__m128 &out, float x)
{
  return out = _mm_set1_ps(x);
}

FORCE_INLINE __m128d BroadcastSIMD(__m128d &out, double x)
{
  return out = _mm_set1_pd(x);
}

FORCE_INLINE __m128i BroadcastSIMD(__m128i &out, int16_t x)
{
  return out = _mm_set1_epi16(x);
}


FORCE_INLINE __m128 LoadFloats(__m128 &out, float *x)
{
  return out = _mm_loadu_ps(x);
}

FORCE_INLINE __m128 LoadFloats(__m128 &out, uint8_t *x)
{
  __m128i u8 = _mm_cvtsi32_si128(*(int32_t *)x),
        i32;
#ifdef __SSE4_1__
  i32 = _mm_cvtepu8_epi32(u8);
#else
  __m128i zero = _mm_setzero_si128();
  i32 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(u8, zero), zero);
#endif
  return out = _mm_cvtepi32_ps(i32);
}

FORCE_INLINE __m128 LoadFloats(__m128 &out, uint16_t *x)
{
  __m128i u16 = _mm_loadl_epi64((__m128i *)x),
      i32;
#ifdef __SSE4_1__
  i32 = _mm_cvtepu16_epi32(u16);
#else
  __m128i zero = _mm_setzero_si128();
  i32 = _mm_unpacklo_epi16(u16, zero);
#endif
  return out = _mm_cvtepi32_ps(i32);
}


template <bool partial = false>   // no, this parameter isn't redundant - without it, there will be a redundant n == 4 check when partial = 0
FORCE_INLINE void StoreFloats(float *out, __m128 x, ssize_t n = 4)
{
  if (partial)
  {
#ifdef __AVX__
    _mm_maskstore_ps(out, PartialVectorMask(n * sizeof(float)), x);
#else
    _mm_maskmoveu_si128(_mm_castps_si128(x), PartialVectorMask(n * sizeof(float)), (char *)out);
#endif
  }
  else
  {
    _mm_storeu_ps(out, x);
  }
}

template <bool partial = false>
FORCE_INLINE void StoreFloats(uint16_t *out, __m128 x, ssize_t n = 4)
{
  __m128i i32 = _mm_cvtps_epi32(x),
#ifdef __SSE4_1__
          u16 = _mm_packus_epi32(i32, i32);
#else
          u16 = _mm_max_epi16(_mm_packs_epi32(i32, i32), _mm_setzero_si128());    // can get away with treating as int16 for now
#endif
  if (partial)
  {
#ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(u16), PartialVectorMask8(n * sizeof(int16_t)), (char *)out);
#else
    _mm_maskmoveu_si128(u16, PartialVectorMask(n * sizeof(int16_t)), (char *)out);
#endif
  }
  else
    _mm_storel_epi64((__m128i *)out, u16);
}

template <bool partial = false>
FORCE_INLINE void StoreFloats(uint8_t *out, __m128 x, ssize_t n = 4)
{
  __m128i i32 = _mm_cvtps_epi32(x),
           u8 = _mm_packus_epi16(_mm_packs_epi32(i32, i32), i32);    // should use packus_epi32, but that's only in SSE4
  if (partial)
  {
#ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(u8), PartialVectorMask8(n), (char *)out);
#else
    _mm_maskmoveu_si128(u8, PartialVectorMask(n), (char *)out);
#endif
  }
  else
    *(int32_t *)out = _mm_cvtsi128_si32(u8);
}


FORCE_INLINE __m128d LoadDoubles(__m128d &out, double *x)
{
    return out = _mm_loadu_pd(x);
}

FORCE_INLINE __m128d LoadDoubles(__m128d &out, uint8_t *x)
{
  __m128i u8 = _mm_cvtsi32_si128(*(uint16_t *)x),
      i32;
#ifdef __SSE4_1__
  i32 = _mm_cvtepu8_epi32(u8);
#else
  __m128i zero = _mm_setzero_si128();
  i32 = _mm_unpacklo_epi16(_mm_unpacklo_epi8(u8, zero), zero);
#endif
  return out = _mm_cvtepi32_pd(i32);
}

FORCE_INLINE __m128d LoadDoubles(__m128d &out, uint16_t *x)
{
  __m128i u16 = _mm_cvtsi32_si128(*(uint32_t *)x),
        i32;
#ifdef __SSE4_1__
  i32 = _mm_cvtepu16_epi32(u16);
#else
  __m128i zero = _mm_setzero_si128();
  i32 = _mm_unpacklo_epi16(u16, zero);
#endif
  return out = _mm_cvtepi32_pd(i32);
}

template <bool partial = false>
FORCE_INLINE void StoreDoubles(double *out, __m128d x, ssize_t n = 2)
{
  if (partial)
  {
  #ifdef __AVX__
    _mm_maskstore_pd(out, PartialVectorMask(n * sizeof(double)), x);
  #else
    _mm_maskmoveu_si128(_mm_castpd_si128(x), PartialVectorMask(n * sizeof(double)), (char *)out); 
  #endif
  }
  else
  {
    _mm_storeu_pd(out, x);
  }
}

template <bool partial = false>
FORCE_INLINE void StoreDoubles(float *out, __m128d x, ssize_t n = 2)
{
  __m128 f32 = _mm_cvtpd_ps(x);
  if (partial)
  {
  #ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(_mm_castps_si128(f32)), PartialVectorMask8(n * sizeof(float)), (char *)out);
  #else
    _mm_maskmoveu_si128(_mm_castps_si128(f32), PartialVectorMask8(n * sizeof(float)), (char *)out);
  #endif
  }
  else
  {
    _mm_storel_pi((__m64 *)out, f32);
  }
}

template <bool partial = false>
FORCE_INLINE void StoreDoubles(uint16_t *out, __m128d x, ssize_t n = 2)
{
  __m128i i32 = _mm_cvtpd_epi32(x),
#ifdef __SSE4_1__
          u16 = _mm_packus_epi32(i32, i32);
#else
          u16 = _mm_max_epi16(_mm_packs_epi32(i32, i32), _mm_setzero_si128());    // can get away with using i16 for now
#endif
  if (partial)
  {
  #ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(u16), PartialVectorMask8(n * sizeof(int16_t)), (char *)out);
  #else
    _mm_maskmoveu_si128(u16, PartialVectorMask8(n * sizeof(int16_t)), (char *)out);
  #endif
  }
  else
  {
    *(uint32_t *)out = _mm_cvtsi128_si32(u16);
  }
}

template <bool partial = false>
FORCE_INLINE void StoreDoubles(uint8_t *out, __m128d x, ssize_t n = 2)
{
  __m128i i32 = _mm_cvtpd_epi32(x),
#ifdef __SSE4_1__
          u16 = _mm_packus_epi32(i32, i32),
#else
          u16 = _mm_max_epi16(_mm_packs_epi32(i32, i32), _mm_setzero_si128()),     // can get away with using i16 for now
#endif
           u8 = _mm_packus_epi16(u16, u16);

  if (partial)
  {
  #ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(u8), PartialVectorMask8(n), (char *)out);
  #else
    _mm_maskmoveu_si128(u8, PartialVectorMask8(n), (char *)out);
  #endif
  }
  else
  {
    *(uint16_t *)out = _mm_cvtsi128_si32(u8);
  }
}

#ifdef __AVX__
FORCE_INLINE __m256 Load4x2Floats(uint8_t *row0, uint8_t *row1)
{
  return _mm256_cvtepi32_ps(_mm256_setr_m128i(_mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(int32_t *)row0)),
                                              _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(int32_t *)row1))));
}

FORCE_INLINE __m256 Load4x2Floats(uint16_t *row0, uint16_t *row1)
{
    return _mm256_cvtepi32_ps(_mm256_setr_m128i(_mm_cvtepu16_epi32(_mm_loadl_epi64((__m128i *)row0)),
                                                _mm_cvtepu16_epi32(_mm_loadl_epi64((__m128i *)row1))));
}

FORCE_INLINE __m256 Load4x2Floats(float *row0, float *row1)
{
  return _mm256_setr_m128(_mm_loadu_ps(row0), _mm_loadu_ps(row1));
}
#endif

FORCE_INLINE __m128i LoadAndScaleToInt16(__m128i &out, uint8_t *x)
{
  // convert from [0-255] to [0-16383]
  // leave 1 spare bit so that 2 values can be added without overflow for symmetric filters
  __m128i u8 = _mm_loadl_epi64((__m128i *)x),
      i16;
#ifdef __SSE4_1__
  i16 = _mm_cvtepu8_epi16(u8);
#else
  i16 = _mm_unpacklo_epi8(u8, _mm_setzero_si128());
#endif
  return out = _mm_slli_epi16(i16, 6);
}

__m128i LoadAndScaleToInt16(__m128i &out, int16_t *x)
{
  return out = _mm_loadu_si128((__m128i *)x);
}

#ifdef __AVX2__

FORCE_INLINE __m256i LoadAndScaleToInt16(__m256i &out, uint8_t *x)
{
  // convert from [0-255] to [0-16383]
  // leave 1 spare bit so that 2 values can be added without overflow for symmetric filters
  return out = _mm256_slli_epi16(_mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i *)x)), 6);
}

FORCE_INLINE __m256i LoadAndScaleToInt16(__m256i &out, int16_t *x)
{
  return out = _mm256_loadu_si256((__m256i *)x);
}

#endif

template <bool partial = false>
FORCE_INLINE void ScaleAndStoreInt16(uint8_t *out, __m128i x, ssize_t n = 8)
{
  __m128i i16 = _mm_srai_epi16(_mm_adds_epi16(x, _mm_set1_epi16(32)), 6),
          u8 = _mm_packus_epi16(i16, i16);
  if (partial)
  {
#ifdef CAN_USE_MMX
    _mm_maskmove_si64(_mm_movepi64_pi64(u8), PartialVectorMask8(n), (char *)out);
#else
    _mm_maskmoveu_si128(u8, PartialVectorMask8(n), (char *)out);
#endif
  }
  else
    _mm_storel_epi64((__m128i *)out, u8);
}

template <bool partial = false>
FORCE_INLINE void ScaleAndStoreInt16(int16_t *out, __m128i i16, ssize_t n = 8)
{
  if (partial)
    _mm_maskmoveu_si128(i16, PartialVectorMask(n * sizeof(int16_t)), (char *)out);
  else
    _mm_storeu_si128((__m128i *)out, i16);
}

#ifdef __AVX2__

template <bool partial = false>
FORCE_INLINE void ScaleAndStoreInt16(uint8_t *out, __m256i x, ssize_t n = 16)
{
  __m256i i16 = _mm256_srai_epi16(_mm256_adds_epi16(x, _mm256_set1_epi16(32)), 6);
  __m128i u8 = _mm256_castsi256_si128(_mm256_packus_epi16(i16, _mm256_permute2f128_si256(i16, i16, 1)));
  if (partial)
    _mm_maskmoveu_si128(u8, PartialVectorMask(n), (char *)out);
  else
    _mm_storeu_si128((__m128i *)out, u8);
}

template <bool partial = false>
FORCE_INLINE void ScaleAndStoreInt16(int16_t *out, __m256i i16, ssize_t n = 16)
{
    if (partial)
    {
      _mm_maskmoveu_si128(_mm256_castsi256_si128(i16), PartialVectorMask(min(ssize_t(8), n) * sizeof(int16_t)), (char *)out);
      _mm_maskmoveu_si128(_mm256_extractf128_si256(i16, 1), PartialVectorMask(max(ssize_t(0), n - 8) * sizeof(int16_t)), (char *)&out[8]);
    }
    else
      _mm256_storeu_si256((__m256i *)out, i16);
}
#endif

// selectors are doubles to avoid int-float domain transition
FORCE_INLINE __m128d Select(__m128d a, __m128d b, __m128d selectors)
{
#ifdef __SSE4_1__
  return _mm_blendv_pd(a, b, selectors);
#else
  return _mm_or_pd(_mm_andnot_pd(selectors, a), _mm_and_pd(selectors, b));
#endif
}

// selectors are floats to avoid int-float domain transition
FORCE_INLINE __m128 Select(__m128 a, __m128 b, __m128 selectors)
{
#ifdef __SSE4_1__
  return _mm_blendv_ps(a, b, selectors);
#else
  return _mm_or_ps(_mm_andnot_ps(selectors, a), _mm_and_ps(selectors, b));
#endif
}

// even these simple ops need to be redeclared for each SIMD architecture due to VEX and non-VEX encodings of SSE instructions
#ifdef __AVX__
FORCE_INLINE __m128 Cast256To128(__m256 v)
{
  return _mm256_castps256_ps128(v);
}

FORCE_INLINE __m128d Cast256To128(__m256d v)
{
  return _mm256_castpd256_pd128(v);
}
FORCE_INLINE __m128i Cast256To128(__m256i v)
{
    return _mm256_castsi256_si128(v);
}
#endif

FORCE_INLINE __m128 Cast256To128(__m128 v)
{
    return v;
}

FORCE_INLINE __m128d Cast256To128(__m128d v)
{
  return v;
}

FORCE_INLINE __m128i Cast256To128(__m128i v)
{
  return v;
}


// does 1D IIR convolution on multiple rows (height) of data
// IntermediateType must be float or double
template <bool transposeOut, bool isForwardPass, bool isBorder, int channels, typename OutType, typename InType, typename IntermediateType>
FORCE_INLINE void Convolve1DHorizontalRef(SimpleImage <OutType> out,
                                        SimpleImage <InType> in,
                                        IntermediateType *borderValues,   // [y][color]
                                        ssize_t xStart, ssize_t xEnd, ssize_t width, ssize_t height,
                                        typename MyTraits<IntermediateType>::SIMDtype *vCoefficients, double M[N * N])
{
  ssize_t xStep = isForwardPass ? 1 : -1;
  
  ssize_t y = 0;
  do
  {
    ssize_t c = 0;
	do
    {
    IntermediateType prevOut[N];
    ssize_t x = xStart;
    if (isBorder && !isForwardPass)
    {
      // xStart must be width - 1
      IntermediateType u[N + 1][1];  // u[0] = last forward filtered value, u[1] = 2nd last forward filtered value, ...
      for (ssize_t i = 0; i < N + 1; ++i)
      {
		 u[i][0] = in[y][(xStart + i * xStep) * channels + c];
      }
      IntermediateType backwardsInitialState[N][1];
      calcTriggsSdikaInitialization<1>(M, u, &borderValues[y * channels + c], &borderValues[y * channels + c], ExtractElement0(vCoefficients[0]), backwardsInitialState);
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = backwardsInitialState[i][0];

      if (transposeOut)
        out[x][y * channels + c] = clip_round_cast<OutType, IntermediateType>(prevOut[0]);
      else
        out[y][x * channels + c] = clip_round_cast<OutType, IntermediateType>(prevOut[0]);
      x += xStep;
      if (x == xEnd)
        goto nextIteration;    // do early check here so that we can still use do-while for forward pass
    }
    else if (isBorder && isForwardPass)
    {
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = in[y][0 * channels + c];
    }
    else
    {
      for (ssize_t i = 0; i < N; ++i)
      {
        prevOut[i] = transposeOut ? out[xStart - (i + 1) * xStep][y * channels + c]
                                  : out[y][(xStart - (i + 1) * xStep) * channels + c];
      }
    }

    do
    {
      IntermediateType sum = prevOut[0] * ExtractElement0(vCoefficients[1])
                      + prevOut[1] * ExtractElement0(vCoefficients[2])
                      + prevOut[2] * ExtractElement0(vCoefficients[3])
                      + in[y][x * channels + c] * ExtractElement0(vCoefficients[0]);    // add last for best accuracy since this terms tends to be the smallest
      if (transposeOut)
          out[x][y * channels + c] = clip_round_cast<OutType, IntermediateType>(sum);
      else
          out[y][x * channels + c] = clip_round_cast<OutType, IntermediateType>(sum);
	  prevOut[2] = prevOut[1];
      prevOut[1] = prevOut[0];
      prevOut[0] = sum;
      x += xStep;
    } while (x != xEnd);
      ++c;
    } while (c < channels);
    nextIteration:
    ++y;
  } while (y < height);
}

template <int channels, bool transposeOut, ssize_t xStep, int i0, int i1, int i2, typename OutType, typename InType>
FORCE_INLINE void DoOneIIR(SimpleImage<OutType> out, SimpleImage<InType> in, __m256d &vSum, __m256d &vIn, ssize_t x, ssize_t y, __m256d vCoefficients[N + 1], __m256d prevOut[N])
{
    vSum = vIn * vCoefficients[0];
    LoadDoubles(vIn, &in[y][(x + xStep) * channels]);  // load data for next iteration early to hide latency (software pipelining)

    // since coefficient[0] * in can be very small, it should be added to a similar magnitude term to minimize rounding error. For this gaussian filter, the 2nd smallest term is usually coefficient[3] * out[-3]
#ifdef __FMA__
    // this expression uses fewer MADs than the max. possible, but has a shorter critical path and is actually faster
    vSum = MultiplyAdd(prevOut[i2], vCoefficients[3], vSum) + MultiplyAdd(prevOut[i1], vCoefficients[2], prevOut[i0] * vCoefficients[1]);
#else
    vSum = prevOut[i0] * vCoefficients[1]
         + prevOut[i1] * vCoefficients[2]
         + prevOut[i2] * vCoefficients[3]
         + vIn         * vCoefficients[0];
#endif
    if (transposeOut)
        StoreDoubles(&out[x][y * channels], vSum);
    else
        StoreDoubles(&out[y][x * channels], vSum);
}

// input is always untransposed
// for reverse pass, input is output from forward pass
// for transposed output, in-place operation isn't possible
// hack: GCC fails to compile when FORCE_INLINE on, most likely because OpenMP doesn't generate code using the target defined in #pragma, but the default (SSE2 only), creating 2 incompatible functions that can't be inlined
template <bool transposeOut, bool isForwardPass, bool isBorder, int channels, typename OutType, typename InType, typename SIMD_Type>
static /*FORCE_INLINE*/ void Convolve1DHorizontal(SimpleImage<OutType> out,
									   SimpleImage<InType> in,
                                       double *borderValues,
                                       ssize_t xStart, ssize_t xEnd, ssize_t width, ssize_t height,
                                       SIMD_Type *vCoefficients, double M[N * N])
{
#if 0
    
        Convolve1DHorizontalRef<transposeOut, isForwardPass, isBorder, channels>(out,
            in,
            borderValues,
            xStart, xEnd, width, height,
            vCoefficients, M);
        return;
    
#endif
  const ssize_t xStep = isForwardPass ? 1 : -1;
  if (channels == 4)
  {
#ifdef __AVX__
  ssize_t y = 0;
  do
  {
    __m256d prevOut[N];
    
    ssize_t x = xStart;
    if (isBorder && !isForwardPass)
    {
      // condition: xStart must be width - 1
      double u[N + 1][channels];  //[x][channels]
      for (ssize_t i = 0; i < N + 1; ++i)
      {
         __m256d temp;
        _mm256_storeu_pd(u[i], LoadDoubles(temp, &in[y][(xStart + i * xStep) * channels]));
      }
      double backwardsInitialState[N][channels];
      calcTriggsSdikaInitialization<channels>(M, u, &borderValues[y * channels], &borderValues[y * channels], ExtractElement0(vCoefficients[0]), backwardsInitialState);
      for (ssize_t i = 0; i < N; ++i)
        LoadDoubles(prevOut[i], backwardsInitialState[i]);

      if (transposeOut)
        StoreDoubles(&out[x][y * channels], prevOut[0]);
      else
        StoreDoubles(&out[y][x * channels], prevOut[0]);

      x += xStep;
      if (x == xEnd)
        goto nextIteration;
    }
    else if (isBorder && isForwardPass)
    {
      __m256d firstPixel;
      LoadDoubles(firstPixel, &in[y][0 * channels]);
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = firstPixel;
    }
    else
    {
      for (ssize_t i = 0; i < N; ++i)
      {
        if (transposeOut)
          LoadDoubles(prevOut[i], &out[xStart - (i + 1) * xStep][y * channels]);
        else
          LoadDoubles(prevOut[i], &out[y][(xStart - (i + 1) * xStep) * channels]);
      }
    }
    
#if 0   // no measurable speedup
    // same as loop below, but unrolled 3 times to increase ||ism, hide latency, and reduce overhead of shifting the sliding window (prevOut)
    __m256d vIn;
    LoadDoubles(vIn, &in[y][xStart * channels]);
    for ( ; isForwardPass ? (x < xEnd - 3) : (x > xEnd + 3);  )
    {
        __m256d vSum;
        DoOneIIR<channels, transposeOut, xStep, 0, 1, 2>(out, in, vSum, vIn, x, y, vCoefficients, prevOut);
        prevOut[2] = vSum;
        x += xStep;

        DoOneIIR<channels, transposeOut, xStep, 2, 0, 1>(out, in, vSum, vIn, x, y, vCoefficients, prevOut);
        prevOut[1] = vSum;
        x += xStep;

        DoOneIIR<channels, transposeOut, xStep, 1, 2, 0>(out, in, vSum, vIn, x, y, vCoefficients, prevOut);
        prevOut[0] = vSum;
        x += xStep;
    }
#endif
    while (isForwardPass ? (x < xEnd) : (x > xEnd))
    {
      __m256d vIn, vSum;
      LoadDoubles(vIn, &in[y][x * channels]),

      // since coefficient[0] * in can be very small, it should be added to a similar magnitude term to minimize rounding error. For this gaussian filter, the 2nd smallest term is usually coefficient[3] * out[-3]
    #ifdef __FMA__
      // this expression uses fewer MADs than the max. possible, but has a shorter critical path and is actually faster
      vSum = MultiplyAdd(vIn, vCoefficients[0], prevOut[2] * vCoefficients[3]) + MultiplyAdd(prevOut[1], vCoefficients[2], prevOut[0] * vCoefficients[1]);
    #else
      vSum	=    prevOut[0] * vCoefficients[1]
               + prevOut[1] * vCoefficients[2]
               + prevOut[2] * vCoefficients[3]
               + vIn        * vCoefficients[0];
    #endif
      if (transposeOut)
          StoreDoubles(&out[x][y * channels], vSum);
	  else
          StoreDoubles(&out[y][x * channels], vSum);

      prevOut[2] = prevOut[1];
      prevOut[1] = prevOut[0];
      prevOut[0] = vSum;
      x += xStep;
    }
  nextIteration:
    ++y;
  } while (y < height);
#else
  // todo: yuck, find some way to refactor (emulate m256d perhaps?)
  ssize_t y = 0;
  do
  {
    __m128d prevOut[N][2];
    
    ssize_t x = xStart;
    if (isBorder && !isForwardPass)
    {
      // condition: xStart must be width - 1
      double u[N + 1][channels];  //[x][channels]
      for (ssize_t i = 0; i < N + 1; ++i)
      {
         __m128d temp;
        _mm_storeu_pd(u[i], LoadDoubles(temp, &in[y][(xStart + i * xStep) * channels]));
        _mm_storeu_pd(&u[i][2], LoadDoubles(temp, &in[y][(xStart + i * xStep) * channels + 2]));
      }
      double backwardsInitialState[N][channels];
      calcTriggsSdikaInitialization<channels>(M, u, &borderValues[y * channels], &borderValues[y * channels], ExtractElement0(vCoefficients[0]), backwardsInitialState);
      for (ssize_t i = 0; i < N; ++i)
      {
        LoadDoubles(prevOut[i][0], backwardsInitialState[i]);
        LoadDoubles(prevOut[i][1], &backwardsInitialState[i][2]);
      }

      if (transposeOut)
      {
        StoreDoubles(&out[x][y * channels], prevOut[0][0]);
        StoreDoubles(&out[x][y * channels + 2], prevOut[0][1]);
      }
      else
      {
        StoreDoubles(&out[y][x * channels], prevOut[0][0]);
        StoreDoubles(&out[y][x * channels + 2], prevOut[0][1]);
      }

      x += xStep;
      if (x == xEnd)
        goto nextIteration;
    }
    else if (isBorder && isForwardPass)
    {
      __m128d firstPixel[2];
      LoadDoubles(firstPixel[0], &in[y][0 * channels]);
      LoadDoubles(firstPixel[1], &in[y][0 * channels + 2]);
      for (ssize_t i = 0; i < N; ++i)
      {
        prevOut[i][0] = firstPixel[0];
        prevOut[i][1] = firstPixel[1];
      }
    }
    else
    {
      for (ssize_t i = 0; i < N; ++i)
      {
        if (transposeOut)
        {
          LoadDoubles(prevOut[i][0], &out[xStart - (i + 1) * xStep][y * channels]);
          LoadDoubles(prevOut[i][1], &out[xStart - (i + 1) * xStep][y * channels + 2]);
        }
        else
        {
          LoadDoubles(prevOut[i][0], &out[y][(xStart - (i + 1) * xStep) * channels]);
          LoadDoubles(prevOut[i][1], &out[y][(xStart - (i + 1) * xStep) * channels + 2]);
        }
      }
    }
    
    while (isForwardPass ? (x < xEnd) : (x > xEnd))
    {
      __m128d vIn[2], vSum[2];
      LoadDoubles(vIn[0], &in[y][x * channels]),
      LoadDoubles(vIn[1], &in[y][x * channels + 2]),

      // since coefficient[0] * in can be very small, it should be added to a similar magnitude term to minimize rounding error. For this gaussian filter, the 2nd smallest term is usually coefficient[3] * out[-3]
      vSum[0]	=    prevOut[0][0] * vCoefficients[1][0]
               + prevOut[1][0] * vCoefficients[2][0]
               + prevOut[2][0] * vCoefficients[3][0]
               + vIn[0]        * vCoefficients[0][0];

      vSum[1]	=    prevOut[0][1] * vCoefficients[1][1]
               + prevOut[1][1] * vCoefficients[2][1]
               + prevOut[2][1] * vCoefficients[3][1]
               + vIn[1]        * vCoefficients[0][1];
      if (transposeOut)
      {
        StoreDoubles(&out[x][y * channels], vSum[0]);
        StoreDoubles(&out[x][y * channels + 2], vSum[1]);
      }
      else
      {
        StoreDoubles(&out[y][x * channels], vSum[0]);
        StoreDoubles(&out[y][x * channels + 2], vSum[1]);
      }
      prevOut[2][0] = prevOut[1][0];
      prevOut[2][1] = prevOut[1][1];
      prevOut[1][0] = prevOut[0][0];
      prevOut[1][1] = prevOut[0][1];
      prevOut[0][0] = vSum[0];
      prevOut[0][1] = vSum[1];
      x += xStep;
    }
  nextIteration:
    ++y;
  } while (y < height);
#endif
  }
  else
  {
      ssize_t y = 0;
      do
      {
          if (isForwardPass)
          {
              ssize_t x = xStart;
              __m128d feedback[2],
                  k0[2];
          #ifdef __AVX__
              k0[0] = Cast256To128(vCoefficients[0]);
              k0[1] = _mm256_extractf128_pd(vCoefficients[0], 1);
          #else
              k0[0] = vCoefficients[0];
              k0[1] = vCoefficients[1];
          #endif        

              if (isBorder && isForwardPass)
              {
                  // xStart must be 0
                  feedback[0] = feedback[1] = _mm_set1_pd(in[y][0]);
              }
              else
              {
                  LoadDoubles(feedback[0], &out[y][xStart - 3 * xStep]);
                  LoadDoubles(feedback[1], &out[y][xStart - 1 * xStep]);
                  
                  feedback[1] = _mm_shuffle_pd(feedback[0], feedback[1], _MM_SHUFFLE2(0, 1));
                  feedback[0] = _mm_shuffle_pd(feedback[0], feedback[0], _MM_SHUFFLE2(0, 0));
              }
              // feedback = [garbage y-3 y-2 y-1]
              for (; x != xEnd; x += xStep)
              {
                  __m128d _in = _mm_set1_pd(in[y][x]),
                      newOutput;
              #ifdef __SSE4_1__
                  feedback[0] = _mm_blend_pd(feedback[0], _in, 0x1);
                  newOutput = _mm_add_pd(_mm_dp_pd(feedback[0], k0[0], 0x31),
                                         _mm_dp_pd(feedback[1], k0[1], 0x31));
                  feedback[0] = _mm_blend_pd(feedback[0], newOutput, 0x1);  // insert back input
              #else
                  __m128d FIRST_ELEMENT_MASK = _mm_castsi128_pd(_mm_set_epi64x(0, ~uint64_t(0)));
                  feedback[0] = Select(feedback[0], _in, FIRST_ELEMENT_MASK);

                  __m128d partialDP = _mm_add_pd
                                      (
                                        _mm_mul_pd(feedback[0], k0[0]),
                                        _mm_mul_pd(feedback[1], k0[1])
                                      );
                  newOutput = _mm_add_pd
                              (
                                partialDP,
                                _mm_shuffle_pd(partialDP, partialDP, _MM_SHUFFLE2(0, 1))
                              );
                  feedback[0] = Select(feedback[0], newOutput, FIRST_ELEMENT_MASK);  // insert back input
              #endif
                  out[y][x] = _mm_cvtsd_f64(newOutput);
                  feedback[0] = _mm_shuffle_pd(feedback[0], feedback[1], _MM_SHUFFLE2(0, 0));
                  feedback[1] = _mm_shuffle_pd(feedback[1], feedback[0], _MM_SHUFFLE2(0, 1));
              }
          }
          else
          {
              __m128d feedback[2], k4[2];
          #ifdef __AVX__
              k4[0] = Cast256To128(vCoefficients[4]);
              k4[1] = _mm256_extractf128_pd(vCoefficients[4], 1);
          #else
              k4[0] = vCoefficients[8];
              k4[1] = vCoefficients[9];
          #endif
              ssize_t x = xStart;
              if (isBorder && !isForwardPass)
              {
                  // xstart must be width - 1
                  double u[N + 1][1];  //[x][y][channels]
                  for (ssize_t i = 0; i < N + 1; ++i)
                  {
                      u[i][0] = in[y][xStart + i * xStep];
                  }
                  #define ROUND_UP(a, b) ((a + b - 1) / b * b)
                  double backwardsInitialState[ROUND_UP(N, 2)][1];    // pad so vector loads don't go past end
                  calcTriggsSdikaInitialization<1>(M, u, &borderValues[y * channels], &borderValues[y * channels], ExtractElement0(vCoefficients[0]), backwardsInitialState);

                  feedback[0] = _mm_load_pd(&backwardsInitialState[0][0]);
                  feedback[1] = _mm_load_pd(&backwardsInitialState[2][0]);

                  out[y][x] = backwardsInitialState[0][0];
                  x += xStep;
                  if (x == xEnd)
                      continue;
              }
              else
              {
                  LoadDoubles(feedback[0], &out[y][xStart - xStep]);
                  LoadDoubles(feedback[1], &out[y][xStart - 3 * xStep]);
              }

              for (; x != xEnd; x += xStep)
              {
                  __m128d _in = _mm_set1_pd(in[y][x]),
                      newOutput;
              #ifdef __SSE4_1__     
                  feedback[1] = _mm_blend_pd(feedback[1], _in, 0x2);
                  newOutput = _mm_add_pd(_mm_dp_pd(feedback[0], k4[0], 0x32),
                                         _mm_dp_pd(feedback[1], k4[1], 0x32));
                  feedback[1] = _mm_blend_pd(feedback[1], newOutput, 0x2);  // insert back input
             #else
                  __m128d LAST_ELEMENT_MASK = _mm_castsi128_pd(_mm_set_epi64x(~uint64_t(0), 0));
                  feedback[1] = Select(feedback[1], _in, LAST_ELEMENT_MASK);

                 __m128d partialDP = _mm_add_pd
                                      (
                                        _mm_mul_pd(feedback[0], k4[0]),
                                        _mm_mul_pd(feedback[1], k4[1])
                                      );
                  newOutput = _mm_add_pd
                              (
                                partialDP,
                                _mm_shuffle_pd(partialDP, partialDP, _MM_SHUFFLE2(0, 0))
                              );
                  feedback[1] = Select(feedback[1], newOutput, LAST_ELEMENT_MASK);
              #endif

                  __m128d temp = _mm_shuffle_pd(feedback[1], feedback[1], _MM_SHUFFLE2(0, 1));
                  out[y][x] = _mm_cvtsd_f64(temp);

                  feedback[1] = _mm_shuffle_pd(feedback[0], feedback[1], _MM_SHUFFLE2(1, 1));
                  feedback[0] = _mm_shuffle_pd(feedback[1], feedback[0], _MM_SHUFFLE2(0, 1));
              }
          }
          ++y;
      } while (y < height);
  }
}


template <int channels, bool transposeOut, ssize_t xStep, int i0, int i1, int i2, typename OutType, typename InType>
FORCE_INLINE void DoOneIIR(SimpleImage<OutType> out, SimpleImage<InType> in, __m256 &vSum, __m256 &vIn, ssize_t x, ssize_t y, __m256 vCoefficients[N + 1], __m256 prevOut[N])
{
    vSum = vIn * vCoefficients[0];

    // load data for next iteration early to hide latency (software pipelining)
    vIn = Load4x2Floats(&in[y][(x + xStep) * channels],
                        &in[y + 1][(x + xStep) * channels]);

    // since coefficient[0] * in can be very small, it should be added to a similar magnitude term to minimize rounding error. For this gaussian filter, the 2nd smallest term is usually coefficient[3] * out[-3]
#ifdef __FMA__
    // this expression uses fewer MADs than the max. possible, but has a shorter critical path and is actually faster
    vSum = MultiplyAdd(prevOut[i2], vCoefficients[3], vSum) + MultiplyAdd(prevOut[i1], vCoefficients[2], prevOut[i0] * vCoefficients[1]);
#else
    vSum = prevOut[i0] * vCoefficients[1]
         + prevOut[i1] * vCoefficients[2]
         + prevOut[i2] * vCoefficients[3]
         + vIn         * vCoefficients[0];
#endif
    if (transposeOut)
    {
        StoreFloats(&out[x][y * channels], _mm256_castps256_ps128(vSum));
        StoreFloats(&out[x][(y + 1) * channels], _mm256_extractf128_ps(vSum, 1));
    }
    else
    {
        StoreFloats(&out[y][x * channels], _mm256_castps256_ps128(vSum));
        StoreFloats(&out[y + 1][x * channels], _mm256_extractf128_ps(vSum, 1));
    }
}

// hack: GCC fails to compile when FORCE_INLINE on, most likely because OpenMP doesn't generate code using the target defined in #pragma, but the default (SSE2 only), creating 2 incompatible functions that can't be inlined
template <bool transposeOut, bool isForwardPass, bool isBorder, int channels, typename OutType, typename InType, typename SIMD_Type>
static /*FORCE_INLINE*/void Convolve1DHorizontal(SimpleImage<OutType> out,
                                     SimpleImage<InType> in,
                                     float *borderValues,
                                     ssize_t xStart, ssize_t xEnd, ssize_t width, ssize_t height,
									 SIMD_Type *vCoefficients, double M[N * N])
{
#if 0
    MyTraits<float>::SIMDtype coefficients2[4];
    
    if (channels == 1)
    {
        coefficients2[0] = _mm256_set1_ps(((float *)vCoefficients)[0]);
        coefficients2[1] = _mm256_set1_ps(((float *)vCoefficients)[3]);
        coefficients2[2] = _mm256_set1_ps(((float *)vCoefficients)[2]);
        coefficients2[3] = _mm256_set1_ps(((float *)vCoefficients)[1]);
        vCoefficients = coefficients2;
    }
    Convolve1DHorizontalRef<transposeOut, isForwardPass, isBorder, channels>(out,
        in,
        borderValues,
        xStart, xEnd, width, height,
        vCoefficients, M);
    return;
#endif
  const ssize_t xStep = isForwardPass ? 1 : -1;
  
  if (channels == 4)
  {
      ssize_t y = 0;
#ifdef __AVX__
      for (; y <= height - 2; y += 2)      // AVX code process 2 rows at a time
      {
          __m256 prevOut[N];

          ssize_t x = xStart;

          if (isBorder && !isForwardPass)
          {
              float u[N + 1][2 * channels];  //[x][y][channels]
              for (ssize_t i = 0; i < N + 1; ++i)
              {
                  __m128 temp;
                  _mm_storeu_ps(&u[i][0], LoadFloats(temp, &in[y][(xStart + i * xStep) * channels]));
                  _mm_storeu_ps(&u[i][channels], LoadFloats(temp, &in[y + 1][(xStart + i * xStep) * channels]));
              }
              float backwardsInitialState[N][2 * channels];
              calcTriggsSdikaInitialization<2 * channels>(M, u, &borderValues[y * channels], &borderValues[y * channels], ExtractElement0(vCoefficients[0]), backwardsInitialState);
              for (ssize_t i = 0; i < N; ++i)
                LoadFloats(prevOut[i], backwardsInitialState[i]);

              if (transposeOut)
              {
                  StoreFloats(&out[x][y * channels], _mm256_castps256_ps128(prevOut[0]));
                  StoreFloats(&out[x][(y + 1) * channels], _mm256_extractf128_ps(prevOut[0], 1));
              }
              else
              {
                  StoreFloats(&out[y][x * channels], _mm256_castps256_ps128(prevOut[0]));
                  StoreFloats(&out[y + 1][x * channels], _mm256_extractf128_ps(prevOut[0], 1));
              }
              x += xStep;
              if (x == xEnd)
                  continue;
          }
          else if (isBorder && isForwardPass)
          {
              // xStart must be 0
              __m256 firstPixel = Load4x2Floats(&in[y][0 * channels], &in[y + 1][0 * channels]);
              for (ssize_t i = 0; i < N; ++i)
                  prevOut[i] = firstPixel;
          }
          else
          {
              for (ssize_t i = 0; i < N; ++i)
              {
                  prevOut[i] = transposeOut ? Load4x2Floats(&out[xStart - (i + 1) * xStep][y * channels],
                                                            &out[xStart - (i + 1) * xStep][(y + 1) * channels])
                      : Load4x2Floats(&out[y][(xStart - (i + 1) * xStep) * channels],
                                      &out[y + 1][(xStart - (i + 1) * xStep) * channels]);
              }
          }

#if 0  // 2x slower than no unrolling - too many register spills?  
          // same as loop below, but unrolled 3 times to increase ||ism, hide latency, and reduce overhead of shifting the sliding window (prevOut)
          __m256 vIn = Load4x2Floats(&in[y][xStart * channels],
                                     &in[y + 1][xStart * channels]);

          for (; isForwardPass ? (x < xEnd - 3) : (x > xEnd + 3); )
          {
              __m256 vSum;
              DoOneIIR<channels, transposeOut, xStep, 0, 1, 2>(out, in, vSum, vIn, x, y, vCoefficients, prevOut);
              prevOut[2] = vSum;
              x += xStep;

              DoOneIIR<channels, transposeOut, xStep, 2, 0, 1>(out, in, vSum, vIn, x, y, vCoefficients, prevOut);
              prevOut[1] = vSum;
              x += xStep;

              DoOneIIR<channels, transposeOut, xStep, 1, 2, 0>(out, in, vSum, vIn, x, y, vCoefficients, prevOut);
              prevOut[0] = vSum;
              x += xStep;
          }
#endif
          for (; x != xEnd; x += xStep)
          {
              __m256 vIn = Load4x2Floats(&in[y][x * channels],
                                         &in[y + 1][x * channels]),
                  vSum;

              // since coefficient[0] * in can be very small, it should be added to a similar magnitude term to minimize rounding error. For this gaussian filter, the 2nd smallest term is usually coefficient[3] * out[-3]
#ifdef __FMA__
   // this expression uses fewer MADs than the max. possible, but has a shorter critical path and is actually faster
              vSum = MultiplyAdd(vIn, vCoefficients[0], prevOut[2] * vCoefficients[3]) + MultiplyAdd(prevOut[1], vCoefficients[2], prevOut[0] * vCoefficients[1]);
#else
              vSum = prevOut[0] * vCoefficients[1]
                  + prevOut[1] * vCoefficients[2]
                  + prevOut[2] * vCoefficients[3]
                  + vIn        * vCoefficients[0];
#endif

              if (transposeOut)
              {
                  StoreFloats(&out[x][y * channels], _mm256_castps256_ps128(vSum));
                  StoreFloats(&out[x][(y + 1) * channels], _mm256_extractf128_ps(vSum, 1));
              }
              else
              {
                  StoreFloats(&out[y][x * channels], _mm256_castps256_ps128(vSum));
                  StoreFloats(&out[y + 1][x * channels], _mm256_extractf128_ps(vSum, 1));
              }
              prevOut[2] = prevOut[1];
              prevOut[1] = prevOut[0];
              prevOut[0] = vSum;
          }
      }
#endif
      for (; y < height; ++y)
      {
          __m128 prevOut[N];
          ssize_t x = xStart;

          if (isBorder && !isForwardPass)
          {
              float u[N + 1][channels];  //[x][channels]
              for (ssize_t i = 0; i < N + 1; ++i)
              {
                  __m128 temp;
                  _mm_storeu_ps(u[i], LoadFloats(temp, &in[y][(xStart + i * xStep) * channels]));
              }
              float backwardsInitialState[N][channels];
              calcTriggsSdikaInitialization<channels>(M, u, &borderValues[y * channels], &borderValues[y * channels], ExtractElement0(vCoefficients[0]), backwardsInitialState);
              for (ssize_t i = 0; i < N; ++i)
                  LoadFloats(prevOut[i], backwardsInitialState[i]);

              if (transposeOut)
                  StoreFloats(&out[x][y * channels], prevOut[0]);
              else
                  StoreFloats(&out[y][x * channels], prevOut[0]);
              x += xStep;
              if (x == xEnd)
                  continue;
          }
          else if (isBorder && isForwardPass)
          {
              // xStart must be 0
              __m128 firstPixel;
              LoadFloats(firstPixel, &in[y][0 * channels]);
              for (ssize_t i = 0; i < N; ++i)
                  prevOut[i] = firstPixel;
          }
          else
          {
              for (ssize_t i = 0; i < N; ++i)
              {
                  if (transposeOut)
                    LoadFloats(prevOut[i], &out[xStart - (i + 1) * xStep][y * channels]);
                  else
                    LoadFloats(prevOut[i], &out[y][(xStart - (i + 1) * xStep) * channels]);
              }
          }

          do
          {
              __m128 vIn, vSum;
              LoadFloats(vIn, &in[y][x * channels]);
              // since coefficient[0] * in can be very small, it should be added to a similar magnitude term to minimize rounding error. For this gaussian filter, the 2nd smallest term is usually coefficient[3] * out[-3]
#ifdef __FMA__
  // this expression uses fewer MADs than the max. possible, but has a shorter critical path and is actually faster
              vSum = MultiplyAdd(vIn, Cast256To128(vCoefficients[0]), prevOut[2] * Cast256To128(vCoefficients[3])) + MultiplyAdd(prevOut[1], Cast256To128(vCoefficients[2]), prevOut[0] * Cast256To128(vCoefficients[1]));
#else
              vSum = prevOut[0] * Cast256To128(vCoefficients[1])
                  + prevOut[1] * Cast256To128(vCoefficients[2])
                  + prevOut[2] * Cast256To128(vCoefficients[3])
                  + vIn        * Cast256To128(vCoefficients[0]);
#endif
              if (transposeOut)
              {
                  StoreFloats(&out[x][y * channels], vSum);
              }
              else
              {
                  StoreFloats(&out[y][x * channels], vSum);
              }
              prevOut[2] = prevOut[1];
              prevOut[1] = prevOut[0];
              prevOut[0] = vSum;
              x += xStep;
          } while (x != xEnd);
      }
  }
  else
  {
      //static_assert(!transposeOut, "transpose not supported");
      ssize_t y = 0;

      const ssize_t Y_BLOCK_SIZE = 8;
#ifdef __AVX__
      for (; y <= height - Y_BLOCK_SIZE; y += Y_BLOCK_SIZE)
      {
          if (isForwardPass)
          {
              ssize_t x = xStart;
              __m256 feedback[4],
                  k0 = vCoefficients[0],
                  k1 = vCoefficients[1],
                  k2 = vCoefficients[2],
                  k3 = vCoefficients[3];
             
              if (isBorder && isForwardPass)
              {
                  // xStart must be 0
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                    feedback[i] = _mm256_setr_m128(_mm_set1_ps(in[y + i * 2][0]),
                                                   _mm_set1_ps(in[y + i * 2 + 1][0]));
                  }
              }
              else
              {
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                      feedback[i] = Load4x2Floats(&out[y + i * 2][xStart - 3 * xStep],
                                                  &out[y + i * 2 + 1][xStart - 3 * xStep]);
                      feedback[i] = _mm256_shuffle_ps(feedback[i], feedback[i], _MM_SHUFFLE(2, 1, 0, 0));
                  }
              }
              // feedback0 = [garbage y-3 y-2 y-1]
              for (; x <= xEnd - 4; x += 4)
              {
                  __m256 _in[4];
                  for (ssize_t i = 0; i < 4; ++i)
                    _in[i] = Load4x2Floats(&in[y + i * 2][x], &in[y + i * 2 + 1][x]);

                  for (int i = 0; i < 4; ++i)
                  {
                      feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x11);
                      feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k0, 0xf1), 0x11);  // insert back input
                  }

                  for (int i = 0; i < 4; ++i)
                  {
                      feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x22);
                      feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k1, 0xf2), 0x22);  // insert back input
                  }

                  for (int i = 0; i < 4; ++i)
                  {
                      feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x44);
                      feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k2, 0xf4), 0x44);  // insert back input
                  }
                  
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                    feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x88);
                    feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k3, 0xf8), 0x88);  // insert back input

                    _mm_storeu_ps((float *)&out[y + i * 2][x], _mm256_castps256_ps128(feedback[i]));
                    _mm_storeu_ps((float *)&out[y + i * 2 + 1][x], _mm256_extractf128_ps(feedback[i], 1));
                  }
              }
              for (; x != xEnd; x += xStep)
              {
                  // todo: make these scalar loads to avoid buffer overflow
                  __m256 _in[4];
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                      _in[i] = Load4x2Floats(&in[y + i * 2][x],
                                             &in[y + i * 2 + 1][x]);
                  }

                  for (int i = 0; i < 4; ++i)
                  {
                      feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x11);
                      feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k0, 0xf1), 0x11);  // insert back input
                  }
                  
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                      out[y + i * 2][x] = _mm_cvtss_f32(_mm256_castps256_ps128(feedback[i]));
                      out[y + i * 2 + 1][x] = _mm_cvtss_f32(_mm256_extractf128_ps(feedback[i], 1));
                  }

                  for (int i = 0; i < 4; ++i)
                    feedback[i] = _mm256_shuffle_ps(feedback[i], feedback[i], _MM_SHUFFLE(0, 3, 2, 0));
              }
          }
          else
          {
              __m256 feedback[4],
                  k4 = vCoefficients[4],
                  k5 = vCoefficients[5],
                  k6 = vCoefficients[6],
                  k7 = vCoefficients[7];
              ssize_t x = xStart;
              if (isBorder && !isForwardPass)
              {
                  // xstart must be width - 1
                  float u[N + 1][8 * channels];  //[x][y][channels]
                  for (ssize_t i = 0; i < N + 1; ++i)
                  {
                      for (ssize_t _y = 0; _y < 8; ++_y)
                        u[i][_y] = in[y + _y][xStart + i * xStep];
                  }
                  float backwardsInitialState[N][8 * channels];
                  calcTriggsSdikaInitialization<8 * channels>(M, u, &borderValues[y * channels], &borderValues[y * channels], ExtractElement0(vCoefficients[0]), backwardsInitialState);
                  
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                    float temp[2][N + 1];
                    for (ssize_t j = 0; j < N; ++j)
                    {
                      temp[0][j] = backwardsInitialState[j][i * 2];
                      temp[1][j] = backwardsInitialState[j][i * 2 + 1];
                    }
                    feedback[i] = Load4x2Floats(temp[0], temp[1]);
                  }

                  for (ssize_t _y = 0; _y < Y_BLOCK_SIZE; ++_y)
                    out[y + _y][x] = backwardsInitialState[0][_y];

                  x += xStep;
                  if (x == xEnd)
                      continue;
              }
              else
              {
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                      feedback[i] = Load4x2Floats(&out[y + i * 2][xStart - xStep],
                                                  &out[y + i * 2 + 1][xStart - xStep]);
                  }
              }
              for (; x - 4 >= xEnd; x -= 4)
              {
                  __m256 _in[4];
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                     _in[i] = Load4x2Floats(&in[y + i * 2][x - 3],
                                            &in[y + i * 2 + 1][x - 3]);
                  }

                  for (int i = 0; i < 4; ++i)
                  {
                    feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x88);
                    feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k4, 0xf8), 0x88);  // insert back input
                  }

                  for (int i = 0; i < 4; ++i)
                  {
                    feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x44);
                    feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k5, 0xf4), 0x44);  // insert back input
                  }
                  
                  for (int i = 0; i < 4; ++i)
                  {
                      feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x22);
                      feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k6, 0xf2), 0x22);  // insert back input
                  }
                  
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                      feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x11);
                      feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k7, 0xf1), 0x11);  // insert back input

                      StoreFloats(&out[y + i * 2][x - 3], _mm256_castps256_ps128(feedback[i]));
                      StoreFloats(&out[y + i * 2 + 1][x - 3], _mm256_extractf128_ps(feedback[i], 1));
                  }                  
              }

              for ( ; x != xEnd; x += xStep)
              {
                  // todo: make these scalar loads to avoid buffer overflow
                  __m256 _in[4];
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                    _in[i] = Load4x2Floats(&in[y + i * 2][x - 3],
                                           &in[y + i * 2 + 1][x - 3]);
                  }
                  
                  for (int i = 0; i < 4; ++i)
                  {
                      feedback[i] = _mm256_blend_ps(feedback[i], _in[i], 0x88);
                      feedback[i] = _mm256_blend_ps(feedback[i], _mm256_dp_ps(feedback[i], k4, 0xf8), 0x88);  // insert back input
                  }
                  
                  for (ssize_t i = 0; i < 4; ++i)
                  {
                      __m256 temp = _mm256_shuffle_ps(feedback[i], feedback[i], _MM_SHUFFLE(0, 0, 0, 3));
                      out[y + i * 2][x] = _mm_cvtss_f32(_mm256_castps256_ps128(temp));
                      out[y + i * 2 + 1][x] = _mm_cvtss_f32(_mm256_extractf128_ps(temp, 1));
                  }

                 
                  for (int i = 0; i < 4; ++i)
                    feedback[i] = _mm256_shuffle_ps(feedback[i], feedback[i], _MM_SHUFFLE(2, 1, 0, 3));
              }
          }
      }
#endif
      for (; y < height; ++y)
      {
          if (isForwardPass)
          {
              ssize_t x = xStart;
              __m128 feedback0,
              k0 = Cast256To128(vCoefficients[0]);

              if (isBorder && isForwardPass)
              {
                  // xStart must be 0
                  feedback0 = _mm_set1_ps(in[y][0]);
              }
              else
              {
                  LoadFloats(feedback0, &out[y][xStart - 3 * xStep]);
                  feedback0 = _mm_shuffle_ps(feedback0, feedback0, _MM_SHUFFLE(2, 1, 0, 0));
              }
              // feedback0 = [garbage y-3 y-2 y-1]
              for (; x != xEnd; x += xStep)
              {
                  __m128 _in0 = _mm_set1_ps(in[y][x]);

              #ifdef __SSE4_1__
                  feedback0 = _mm_blend_ps(feedback0, _in0, 0x1);
                  feedback0 = _mm_blend_ps(feedback0, _mm_dp_ps(feedback0, k0, 0xf1), 0x1);  // insert back input
              #else
                  const __m128 FIRST_ELEMENT_MASK = _mm_castsi128_ps(_mm_set_epi32(0, 0, 0, ~0));
                  feedback0 = Select(feedback0, _in0, FIRST_ELEMENT_MASK);

                  __m128 partialDP = _mm_mul_ps(feedback0, k0);
                  partialDP = _mm_add_ps(partialDP, _mm_shuffle_ps(partialDP, partialDP, _MM_SHUFFLE(0, 0, 3, 2)));
                  __m128 DP = _mm_add_ps(partialDP, _mm_shuffle_ps(partialDP, partialDP, _MM_SHUFFLE(0, 0, 0, 1)));

                  feedback0 = Select(feedback0, DP, FIRST_ELEMENT_MASK);  // insert back input
              #endif
                  out[y][x] = _mm_cvtss_f32(feedback0);
                  feedback0 = _mm_shuffle_ps(feedback0, feedback0, _MM_SHUFFLE(0, 3, 2, 0));
              }
          }
          else
          {
              __m128 feedback0,
                  k4 = Cast256To128(vCoefficients[4]);

              ssize_t x = xStart;
              if (isBorder && !isForwardPass)
              {
                  // xstart must be width - 1
                  float u[N + 1][channels];  //[x][y][channels]
                  for (ssize_t i = 0; i < N + 1; ++i)
                  {
                      u[i][0] = in[y][xStart + i * xStep];
                  }
                  float backwardsInitialState[N][channels];
                  calcTriggsSdikaInitialization<channels>(M, u, &borderValues[y * channels], &borderValues[y * channels], ExtractElement0(vCoefficients[0]), backwardsInitialState);

                  float temp[N + 1];
                  for (ssize_t i = 0; i < N; ++i)
                  {
                      temp[i] = backwardsInitialState[i][0];
                  }
                  LoadFloats(feedback0, temp);

                  out[y][x] = backwardsInitialState[0][0];
                  x += xStep;
                  if (x == xEnd)
                      continue;
              }
              else
              {
                  LoadFloats(feedback0, &out[y][xStart - xStep]);
              }
              
              for (; x != xEnd; x += xStep)
              {
                  __m128 _in0 = _mm_set1_ps(in[y][x]);
                      
              #ifdef __SSE4_1__
                  feedback0 = _mm_blend_ps(feedback0, _in0, 0x8);
                  feedback0 = _mm_blend_ps(feedback0, _mm_dp_ps(feedback0, k4, 0xf8), 0x8);  // insert back input
              #else
                  const __m128 LAST_ELEMENT_MASK = _mm_castsi128_ps(_mm_set_epi32(~0, 0, 0, 0));
                  feedback0 = Select(feedback0, _in0, LAST_ELEMENT_MASK);

                  __m128 partialDP = _mm_mul_ps(feedback0, k4);
                  partialDP = _mm_add_ps(partialDP, _mm_shuffle_ps(partialDP, partialDP, _MM_SHUFFLE(1, 0, 0, 0)));
                  __m128 DP = _mm_add_ps(partialDP, _mm_shuffle_ps(partialDP, partialDP, _MM_SHUFFLE(2, 0, 0, 0)));

                  feedback0 = Select(feedback0, DP, LAST_ELEMENT_MASK);  // insert back input
              #endif
                  __m128 temp = _mm_shuffle_ps(feedback0, feedback0, _MM_SHUFFLE(0, 0, 0, 3));
                  out[y][x] = _mm_cvtss_f32(temp);
                  feedback0 = _mm_shuffle_ps(feedback0, feedback0, _MM_SHUFFLE(2, 1, 0, 3));
              }
          }
      }
  }
}


// does 1D IIR convolution on multiple rows (height) of data
// IntermediateType must be float or double
template <bool isForwardPass, bool isBorder, typename OutType, typename InType, typename IntermediateType>
FORCE_INLINE void Convolve1DVerticalRef(SimpleImage<OutType> out,
                                        SimpleImage<InType> in,
                                        IntermediateType *borderValues,   // [y][color]
                                        ssize_t yStart, ssize_t yEnd, ssize_t width, ssize_t height,
                                        typename MyTraits<IntermediateType>::SIMDtype *vCoefficients, double M[N * N])
{
  ssize_t yStep = isForwardPass ? 1 : -1;
  
  ssize_t x = 0;
  do
  {
    IntermediateType prevOut[N];
    ssize_t y = yStart;
    if (isBorder && !isForwardPass)
    {
      IntermediateType u[N + 1][1];  // u[0] = last forward filtered value, u[1] = 2nd last forward filtered value, ...
      for (ssize_t i = 0; i < N + 1; ++i)
      {
		 u[i][0] = in[yStart + i * yStep][x];
      }
      IntermediateType backwardsInitialState[N][1];
      calcTriggsSdikaInitialization<1>(M, u, &borderValues[x], &borderValues[x], ExtractElement0(vCoefficients[0]), backwardsInitialState);
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = backwardsInitialState[i][0];
      
      out[y][x] = clip_round_cast<OutType, IntermediateType>(prevOut[0]);
      y += yStep;
      if (y == yEnd)
        goto nextIteration;
    }
    else if (isBorder && isForwardPass)
    {
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = in[0][x];
    }
    else
    {
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = out[yStart - (i + 1) * yStep][x];
    }
    
    do
    {
      IntermediateType sum = prevOut[0] * ExtractElement0(vCoefficients[1])
                      + prevOut[1] * ExtractElement0(vCoefficients[2])
                      + prevOut[2] * ExtractElement0(vCoefficients[3])
                      + in[y][x] * ExtractElement0(vCoefficients[0]);    // add last for best accuracy since this terms tends to be the smallest
       
     
	  out[y][x] = clip_round_cast<OutType, IntermediateType>(sum);
	  prevOut[2] = prevOut[1];
      prevOut[1] = prevOut[0];
      prevOut[0] = sum;
      y += yStep;
    } while (y != yEnd);
    nextIteration:
    ++x;
  } while (x < width);
}



// input is always untransposed
// for reverse pass, input is output from forward pass
// for transposed output, in-place operation isn't possible
// hack: GCC fails to compile when FORCE_INLINE on, most likely because OpenMP doesn't generate code using the target defined in #pragma, but the default (SSE2 only), creating 2 incompatible functions that can't be inlined
template <bool isForwardPass, bool isBorder, typename OutType, typename InType, typename SIMD_Type>
static /*FORCE_INLINE*/ void Convolve1DVertical(SimpleImage<OutType> out,
									 SimpleImage<InType> in,
                                     float *borderValues,
                                     ssize_t yStart, ssize_t yEnd, ssize_t width, ssize_t height,
									 SIMD_Type *vCoefficients, double M[N * N])
{
#if 0
    Convolve1DVerticalRef<isForwardPass, isBorder>(out,
        in,
        borderValues,
        yStart, yEnd, width, height,
        vCoefficients, M);
    return;
#endif
  const ssize_t yStep = isForwardPass ? 1 : -1;
  
  const int SIMD_WIDTH = 8;
  ssize_t x = 0;
#ifdef __AVX__
  for ( ; x <= width - SIMD_WIDTH; x += SIMD_WIDTH)
  {
    __m256 prevOut[N];
    ssize_t y = yStart;
    if (isBorder && !isForwardPass)
    {
      float u[N + 1][SIMD_WIDTH];  //[x][channels]
      for (ssize_t i = 0; i < N + 1; ++i)
      {
         __m256 temp;
        _mm256_storeu_ps(u[i], LoadFloats(temp, &in[yStart + i * yStep][x]));
      }
      float backwardsInitialState[N][SIMD_WIDTH];
      calcTriggsSdikaInitialization<SIMD_WIDTH>(M, u, &borderValues[x], &borderValues[x], ExtractElement0(vCoefficients[0]), backwardsInitialState);
      for (ssize_t i = 0; i < N; ++i)
        LoadFloats(prevOut[i], backwardsInitialState[i]);

      StoreFloats(&out[y][x], prevOut[0]);

      y += yStep;
      if (y == yEnd)
        continue;
    }
    else if (isBorder && isForwardPass)
    {
      // yStart must be 0
      __m256 firstPixel;
      LoadFloats(firstPixel, &in[0][x]);
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = firstPixel;
    }
    else
    {
      for (ssize_t i = 0; i < N; ++i)
      {
        LoadFloats(prevOut[i], &out[yStart - (i + 1) * yStep][x]);
      }
    }

    do
    {
      __m256 vIn;
      LoadFloats(vIn, &in[y][x]);
      __m256 vSum = vIn * vCoefficients[0];

      vSum	=    prevOut[0] * vCoefficients[1]
               + prevOut[1] * vCoefficients[2]
               + prevOut[2] * vCoefficients[3]
               + vSum;
   
      StoreFloats(&out[y][x], vSum);

      prevOut[2] = prevOut[1];
      prevOut[1] = prevOut[0];
      prevOut[0] = vSum;
      y += yStep;
    } while (isForwardPass ? (y < yEnd) : (y > yEnd));
  }
#endif
  {
  const ssize_t SIMD_WIDTH = 4;
  for (; x < width; x += SIMD_WIDTH)
  {
    __m128 prevOut[N];
    ssize_t y = yStart;
    if (isBorder && !isForwardPass)
    {
      float u[N + 1][SIMD_WIDTH];  //[x][channels]
      for (ssize_t i = 0; i < N + 1; ++i)
      {
        __m128 temp;
        _mm_storeu_ps(u[i], LoadFloats(temp, &in[yStart + i * yStep][x]));
      }
      float backwardsInitialState[N][SIMD_WIDTH];
      calcTriggsSdikaInitialization<SIMD_WIDTH>(M, u, &borderValues[x], &borderValues[x], ExtractElement0(vCoefficients[0]), backwardsInitialState);
      for (ssize_t i = 0; i < N; ++i)
        LoadFloats(prevOut[i], backwardsInitialState[i]);

      StoreFloats<true>(&out[y][x], prevOut[0], min(SIMD_WIDTH, width - x));   // todo: specialize loop to avoid partial stores

      y += yStep;
      if (y == yEnd)
        continue;
    }
    else if (isBorder && isForwardPass)
    {
      // yStart must be 0
      __m128 firstPixel;
      LoadFloats(firstPixel, &in[0][x]);
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = firstPixel;
    }
    else
    {
      for (ssize_t i = 0; i < N; ++i)
      {
        LoadFloats(prevOut[i], &out[yStart - (i + 1) * yStep][x]);
      }
    }

    do
    {
      __m128 vIn;
      LoadFloats(vIn, &in[y][x]);
      __m128 vSum = vIn * Cast256To128(vCoefficients[0]);

      vSum	=    prevOut[0] * Cast256To128(vCoefficients[1])
               + prevOut[1] * Cast256To128(vCoefficients[2])
               + prevOut[2] * Cast256To128(vCoefficients[3])
               + vSum;

      StoreFloats<true>(&out[y][x], vSum, min(SIMD_WIDTH, width - x));   // todo: specialize loop to avoid partial stores

      prevOut[2] = prevOut[1];
      prevOut[1] = prevOut[0];
      prevOut[0] = vSum;
      y += yStep;
    } while (isForwardPass ? (y < yEnd) : (y > yEnd));
  }
  }
}

// input is always untransposed
// for reverse pass, input is output from forward pass
// for transposed output, in-place operation isn't possible
// hack: GCC fails to compile when FORCE_INLINE on, most likely because OpenMP doesn't generate code using the target defined in #pragma, but the default (SSE2 only), creating 2 incompatible functions that can't be inlined
template <bool isForwardPass, bool isBorder, typename OutType, typename InType, typename SIMD_Type>
static /*FORCE_INLINE*/ void Convolve1DVertical(SimpleImage<OutType> out,
									 SimpleImage<InType> in,
                                     double *borderValues,
                                     ssize_t yStart, ssize_t yEnd, ssize_t width, ssize_t height,
									 SIMD_Type *vCoefficients, double M[N * N])
{
#if 0
    Convolve1DVerticalRef<isForwardPass, isBorder>(out,
        in,
        borderValues,
        yStart, yEnd, width, height,
        vCoefficients, M);
    return;
#endif

  const ssize_t yStep = isForwardPass ? 1 : -1,
       SIMD_WIDTH = 4;
  ssize_t x = 0;
#ifdef __AVX__
  for ( ; x <= width - SIMD_WIDTH; x += SIMD_WIDTH)
  {
    __m256d prevOut[N];
    ssize_t y = yStart;

    if (isBorder && !isForwardPass)
    {
      // condition: yStart must be height - 1
      double u[N + 1][SIMD_WIDTH];  //[x][channels]
      for (ssize_t i = 0; i < N + 1; ++i)
      {
        __m256d temp;
        _mm256_storeu_pd(u[i], LoadDoubles(temp, &in[yStart + i * yStep][x]));
      }
      double backwardsInitialState[N][SIMD_WIDTH];
      calcTriggsSdikaInitialization<SIMD_WIDTH>(M, u, &borderValues[x], &borderValues[x], ExtractElement0(vCoefficients[0]), backwardsInitialState);
      for (ssize_t i = 0; i < N; ++i)
        LoadDoubles(prevOut[i], backwardsInitialState[i]);

      StoreDoubles(&out[y][x], prevOut[0]);

      y += yStep;
      if (y == yEnd)
        continue;
    }
    else if (isBorder && isForwardPass)
    {
      // condition: yStart must be 0
      __m256d firstPixel;
      LoadDoubles(firstPixel, &in[0][x]);
      for (ssize_t i = 0; i < N; ++i)
        prevOut[i] = firstPixel;
    }
    else
    {
      for (ssize_t i = 0; i < N; ++i)
        LoadDoubles(prevOut[i], &out[yStart - (i + 1) * yStep][x]);
    }
    
    do
    {
      __m256d vIn;
      LoadDoubles(vIn, &in[y][x]);
      __m256d vSum = vIn * vCoefficients[0];

      vSum	=    prevOut[0] * vCoefficients[1]
               + prevOut[1] * vCoefficients[2]
               + prevOut[2] * vCoefficients[3]
               + vSum;

      StoreDoubles(&out[y][x], vSum);

      prevOut[2] = prevOut[1];
      prevOut[1] = prevOut[0];
      prevOut[0] = vSum;
      y += yStep;
    } while (y != yEnd);
  }
#endif
  {
  const ssize_t SIMD_WIDTH = 2;
  for (; x < width; x += SIMD_WIDTH)
  {
      __m128d prevOut[N];
      ssize_t y = yStart;

      if (isBorder && !isForwardPass)
      {
          // condition: yStart must be height - 1
          double u[N + 1][SIMD_WIDTH];  //[x][channels]
          for (ssize_t i = 0; i < N + 1; ++i)
          {
              __m128d temp;
              _mm_storeu_pd(u[i], LoadDoubles(temp, &in[yStart + i * yStep][x]));
          }
          double backwardsInitialState[N][SIMD_WIDTH];
          calcTriggsSdikaInitialization<SIMD_WIDTH>(M, u, &borderValues[x], &borderValues[x], ExtractElement0(vCoefficients[0]), backwardsInitialState);
          for (ssize_t i = 0; i < N; ++i)
              LoadDoubles(prevOut[i], backwardsInitialState[i]);

          StoreDoubles<true>(&out[y][x], prevOut[0], min(SIMD_WIDTH, width - x));      // todo: specialize loop to avoid partial stores

          y += yStep;
          if (y == yEnd)
              continue;
      }
      else if (isBorder && isForwardPass)
      {
          // condition: yStart must be 0
          __m128d firstPixel;
          LoadDoubles(firstPixel, &in[0][x]);
          for (ssize_t i = 0; i < N; ++i)
              prevOut[i] = firstPixel;
      }
      else
      {
          for (ssize_t i = 0; i < N; ++i)
             LoadDoubles(prevOut[i], &out[yStart - (i + 1) * yStep][x]);
      }

      do
      {
          __m128d vIn;
          LoadDoubles(vIn, &in[y][x]);
          __m128d vSum = vIn * Cast256To128(vCoefficients[0]);

          vSum = prevOut[0] * Cast256To128(vCoefficients[1])
              + prevOut[1] * Cast256To128(vCoefficients[2])
              + prevOut[2] * Cast256To128(vCoefficients[3])
              + vSum;

          StoreDoubles<true>(&out[y][x], vSum, min(SIMD_WIDTH, width - x));     // todo: specialize loop to avoid partial stores

          prevOut[2] = prevOut[1];
          prevOut[1] = prevOut[0];
          prevOut[0] = vSum;
          y += yStep;
      } while (y != yEnd);
  }
  }
}

// hack: GCC fails to compile when FORCE_INLINE on, most likely because OpenMP doesn't generate code using the target defined in #pragma, but the default (SSE2 only), creating 2 incompatible functions that can't be inlined
template <bool transposeOut, int channels>
static /*FORCE_INLINE*/ void Copy2D(SimpleImage<uint8_t> out, SimpleImage<float> in, ssize_t width, ssize_t height)
{
  ssize_t y = 0;
  do
  {
    ssize_t x = 0;
    if (channels == 4)
    {
    #ifdef __AVX2__
      for (; x <= width - 2; x += 2)
      {
        __m256i vInt = _mm256_cvtps_epi32(_mm256_loadu_ps(&in[y][x * channels]));
	    __m256i vInt2 = _mm256_permute2x128_si256(vInt, vInt, 1);

        __m128i u16 = _mm256_castsi256_si128(_mm256_packus_epi32(vInt, vInt2));
        __m128i vRGBA = _mm_packus_epi16(u16, u16);

	    if (transposeOut)
	    {
          *(uint32_t *)&out[x][y * channels] = _mm_extract_epi32(vRGBA, 0);
          *(uint32_t *)&out[x + 1][y * channels] = _mm_extract_epi32(vRGBA, 1);
	    }
	    else
	    {
          _mm_storel_epi64((__m128i *)&out[y][x * channels], vRGBA);
	    }
      }
    #endif
      while (x < width)
      {
          __m128 data = _mm_loadu_ps(&in[y][x * channels]);
          if (transposeOut)
            StoreFloats(&out[x][y * channels], data);
          else
            StoreFloats(&out[y][x * channels], data);
          ++x;
      }
    }
    else if (channels == 1)
    {
#ifdef __AVX__
      for (; x <= width - 8; x += 8)
      {
          StoreFloats(&out[y][x], _mm256_loadu_ps(&in[y][x]));
      }
      if (x < width)
        StoreFloats<true>(&out[y][x], _mm256_loadu_ps(&in[y][x]), width - x);
#else
      for (; x <= width - 4; x += 4)
      {
         StoreFloats(&out[y][x], _mm_loadu_ps(&in[y][x]));
      }
      if (x < width)
        StoreFloats<true>(&out[y][x], _mm_loadu_ps(&in[y][x]), width - x);
#endif
    }
    ++y;
  } while (y < height);
}

// hack: GCC fails to compile when FORCE_INLINE on, most likely because OpenMP doesn't generate code using the target defined in #pragma, but the default (SSE2 only), creating 2 incompatible functions that can't be inlined
template <bool transposeOut, int channels>
static /*FORCE_INLINE*/ void Copy2D(SimpleImage<uint16_t> out, SimpleImage<float> in, ssize_t width, ssize_t height)
{
  ssize_t y = 0;
  do
  {
    ssize_t x = 0;
    if (channels == 4)
    {
    #ifdef __AVX2__
      for (; x <= width - 2; x += 2)
      {
        __m256i vInt = _mm256_cvtps_epi32(_mm256_loadu_ps(&in[y][x * channels]));
	    __m256i vInt2 = _mm256_permute2x128_si256(vInt, vInt, 1);

        __m128i vRGBA = _mm256_castsi256_si128(_mm256_packus_epi32(vInt, vInt2));

	    if (transposeOut)
	    {
          _mm_storel_epi64((__m128i *)&out[x][y * channels], vRGBA);
          _mm_storel_epi64((__m128i *)&out[x + 1][y * channels], _mm_shuffle_epi32(vRGBA, _MM_SHUFFLE(0, 0, 3, 2)));
	    }
	    else
	    {
          _mm_storeu_si128((__m128i *)&out[y][x * channels], vRGBA);
	    }
      }
    #endif
      while (x < width)
      {
          __m128 data = _mm_loadu_ps(&in[y][x * channels]);
          if (transposeOut)
            StoreFloats(&out[x][y * channels], data);
          else
            StoreFloats(&out[y][x * channels], data);
          ++x;
      }
    }
    else if (channels == 1)
    {
    #ifdef __AVX__
      for (; x <= width - 8; x += 8)
      {
        StoreFloats(&out[y][x], _mm256_loadu_ps(&in[y][x]));
      }
      if (x < width)
        StoreFloats<true>(&out[y][x], _mm256_loadu_ps(&in[y][x]), width - x);
    #else
      for (; x <= width - 4; x += 4)
      {
        StoreFloats(&out[y][x], _mm_loadu_ps(&in[y][x]));
      }
      if (x < width)
        StoreFloats<true>(&out[y][x], _mm_loadu_ps(&in[y][x]), width - x);
    #endif
    }
    ++y;
  } while (y < height);
}

// hack: GCC fails to compile when FORCE_INLINE on, most likely because OpenMP doesn't generate code using the target defined in #pragma, but the default (SSE2 only), creating 2 incompatible functions that can't be inlined
template <bool transposeOut, int channels>
static /*FORCE_INLINE*/ void Copy2D(SimpleImage<uint16_t> out, SimpleImage<double> in, ssize_t width, ssize_t height)
{
  ssize_t y = 0;
  do
  {
    ssize_t x = 0;
    if (channels == 4)
    {
      for ( ; x < width; x++)
      {
      #ifdef __AVX__
        __m128i i32 = _mm256_cvtpd_epi32(_mm256_loadu_pd(&in[y][x * channels]));
      #else
        __m128d in0 = _mm_load_pd(&in[y][x * channels]),
                in1 = _mm_load_pd(&in[y][x * channels + 2]);
        __m128i i32 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(_mm_cvtpd_epi32(in0)), _mm_castsi128_ps(_mm_cvtpd_epi32(in1)), _MM_SHUFFLE(1, 0, 1, 0)));
      #endif
      #ifdef __SSE4_1__
        __m128i vRGBA = _mm_packus_epi32(i32, i32);
      #else
        __m128i vRGBA = _mm_max_epi16(_mm_packs_epi32(i32, i32), _mm_setzero_si128());     // hack: can get away with i16 for now
      #endif

	    if (transposeOut)
	    {
          _mm_storel_epi64((__m128i *)&out[x][y * channels], vRGBA);
	    }
	    else
	    {
          _mm_storel_epi64((__m128i *)&out[y][x * channels], vRGBA);
	    }
      }
    }
    else if (channels == 1)
    {
    #ifdef __AVX__
      for (; x <= width - 4; x += 4)
      {
        StoreDoubles(&out[y][x], _mm256_loadu_pd(&in[y][x]));
      }
      if (x < width)
      {
        StoreDoubles<true>(&out[y][x], _mm256_loadu_pd(&in[y][x]), width - x);
      }
    #else
      for (; x <= width - 2; x += 2)
      {
        StoreDoubles(&out[y][x], _mm_loadu_pd(&in[y][x]));
      }
      if (x < width)
      {
        StoreDoubles<true>(&out[y][x], _mm_loadu_pd(&in[y][x]), width - x);
      }
    #endif
    }
    ++y;
  } while (y < height);
}

template <bool transposeOut, int channels>
FORCE_INLINE void Copy2D(SimpleImage<float> out, SimpleImage<double> in, ssize_t width, ssize_t height)
{
  ssize_t y = 0;
  do
  {
      if (channels == 4)
      {
          ssize_t x = 0;
          do
          {
          #ifdef __AVX__
              __m128 v4f_data = _mm256_cvtpd_ps(_mm256_loadu_pd(&in[y][x * channels]));
          #else
              __m128 v4f_data = _mm_shuffle_ps(_mm_cvtpd_ps(_mm_loadu_pd(&in[y][x * channels])),
                  _mm_cvtpd_ps(_mm_loadu_pd(&in[y][x * channels + 2])), _MM_SHUFFLE(1, 0, 1, 0));
          #endif
              if (transposeOut)
                  _mm_store_ps(&out[x][y * channels], v4f_data);
              else
                  _mm_store_ps(&out[y][x * channels], v4f_data);
              ++x;
          } while (x < width);
      }
    else
    {
      // 1 channel
      ssize_t x;
    #ifdef __AVX__
      for (x = 0; x <= width - 4; x += 4)
        StoreDoubles(&out[y][x], _mm256_loadu_pd(&in[y][x]));
      if (x < width)
        StoreDoubles<true>(&out[y][x], _mm256_loadu_pd(&in[y][x]), width - x);
    #else
      for (x = 0; x <= width - 2; x += 2)
        StoreDoubles(&out[y][x], _mm_loadu_pd(&in[y][x]));
      if (x < width)
        StoreDoubles<true>(&out[y][x], _mm_loadu_pd(&in[y][x]), width - x);
    #endif
    }
    ++y;
  } while (y < height);
}

// hack: GCC fails to compile when FORCE_INLINE on, most likely because OpenMP doesn't generate code using the target defined in #pragma, but the default (SSE2 only), creating 2 incompatible functions that can't be inlined
template <bool transposeOut, int channels>
static /*FORCE_INLINE*/ void Copy2D(SimpleImage<uint8_t> out, SimpleImage <double> in, ssize_t width, ssize_t height)
{
    ssize_t y = 0;
    do
    {
        if (channels == 4)
        {
            ssize_t x = 0;
            do
            {
            #ifdef __AVX__
                __m256d _in = _mm256_load_pd(&in[y][x * channels]);
                __m128i i32 = _mm256_cvtpd_epi32(_in),
            #else
                __m128d in0 = _mm_load_pd(&in[y][x * channels]),
                        in1 = _mm_load_pd(&in[y][x * channels + 2]);
                __m128i i32 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(_mm_cvtpd_epi32(in0)), _mm_castsi128_ps(_mm_cvtpd_epi32(in1)), _MM_SHUFFLE(1, 0, 1, 0))),
            #endif
            #ifdef __SSE4_1__
                    u16 = _mm_packus_epi32(i32, i32),
            #else
                    u16 = _mm_max_epi16(_mm_packs_epi32(i32, i32), _mm_setzero_si128()),
            #endif
                    u8 = _mm_packus_epi16(u16, u16);
                if (transposeOut)
                    *(int32_t *)&out[x][y * channels] = _mm_cvtsi128_si32(u8);
                else
                    *(int32_t *)&out[y][x * channels] = _mm_cvtsi128_si32(u8);
                ++x;
            } while (x < width);
        }
        else
        {
            // 1 channel
            ssize_t x;
        #ifdef __AVX__
            for (x = 0; x <= width - 4; x += 4)
              StoreDoubles(&out[y][x], _mm256_load_pd(&in[y][x * channels]));
            if (x < width)
              StoreDoubles<true>(&out[y][x], _mm256_load_pd(&in[y][x * channels]), width - x);
        #else
            for (x = 0; x <= width - 2; x += 2)
              StoreDoubles(&out[y][x], _mm_load_pd(&in[y][x * channels]));
            if (x < width)
              StoreDoubles<true>(&out[y][x], _mm_load_pd(&in[y][x * channels]), width - x);
        #endif
        }
        ++y;
    } while (y < height);
}

#if 0  // comment this function out to ensure everything is vectorized
template <bool transposeOut, int channels, typename OutType, typename InType>
FORCE_INLINE void Copy2D(SimpleImage<OutType> out, SimpleImage <InType> in, ssize_t width, ssize_t height)
{
  ssize_t y = 0;
  do
  {
    ssize_t x = 0;
	do
    {
      ssize_t c = 0;
      do
      {
		 if (transposeOut)
          out[x][y * channels + c] = clip_round_cast<OutType, InType>(in[y][x * channels + c]);
	     else
          out[y][x * channels + c] = clip_round_cast<OutType, InType>(in[y][x * channels + c]);
         ++c;
      } while (c < channels);
	  ++x;
    } while (x < width);
    ++y;
  } while (y < height);
}
#endif

template <typename AnyType>
void StoreFloatsTransposed(SimpleImage<AnyType> out, __m128 x)
{
    out[0][0] = _mm_cvtss_f32(x);
    out[1][0] = _mm_cvtss_f32(_mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 0, 0, 1)));
    out[2][0] = _mm_cvtss_f32(_mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 0, 0, 2)));
    out[3][0] = _mm_cvtss_f32(_mm_shuffle_ps(x, x, _MM_SHUFFLE(0, 0, 0, 3)));
}

// input & output are color interleaved
template <bool transposeOut, int channels, typename IntermediateType, typename OutType, typename InType>
void ConvolveHorizontal(SimpleImage<OutType> out, SimpleImage<InType> in, ssize_t width, ssize_t height, float sigmaX, bool canOverwriteInput = false)
{
  const bool convertOutput = typeid(OutType) != typeid(IntermediateType);

  typedef typename MyTraits<IntermediateType>::SIMDtype SIMDtype;

  double bf[N];
  double M[N*N]; // matrix used for initialization procedure (has to be double)
  double b[N + 1];

  calcFilter(sigmaX, bf);

  for (size_t i = 0; i<N; i++)
      bf[i] = -bf[i];

  b[0] = 1; // b[0] == alpha (scaling coefficient)

  for (size_t i = 0; i<N; i++)
  {
      b[i + 1] = bf[i];
      b[0] -= b[i + 1];
  }

  calcTriggsSdikaM(bf, M);  // Compute initialization matrix

  SIMDtype *vCoefficients;

  if (channels == 4)
  {
    vCoefficients = (SIMDtype *)ALIGNED_ALLOCA(sizeof(SIMDtype) * (N + 1), sizeof(SIMDtype));
    for (ssize_t i = 0; i <= N; ++i)
      BroadcastSIMD(vCoefficients[i], (IntermediateType)b[i]);
  }
  else
  {
      if (typeid(IntermediateType) == typeid(double))
      {
      #ifdef __AVX2__
          __m256d *_vCoefficients = (__m256d *)ALIGNED_ALLOCA(sizeof(SIMDtype) * 2 * (N + 1), sizeof(SIMDtype));
          vCoefficients = (SIMDtype *)_vCoefficients;

          __m256d temp = _mm256_loadu_pd(b);
          _vCoefficients[0] = _mm256_permute4x64_pd(temp, _MM_SHUFFLE(1, 2, 3, 0));
          _vCoefficients[1] = _mm256_permute4x64_pd(temp, _MM_SHUFFLE(2, 3, 0, 1));
          _vCoefficients[2] = _mm256_permute4x64_pd(temp, _MM_SHUFFLE(3, 0, 1, 2));
          _vCoefficients[3] = _mm256_permute4x64_pd(temp, _MM_SHUFFLE(0, 1, 2, 3));

          // permutations for backward pass
          _vCoefficients[4] = _mm256_permute4x64_pd(temp, _MM_SHUFFLE(0, 3, 2, 1));
          _vCoefficients[5] = _mm256_permute4x64_pd(temp, _MM_SHUFFLE(1, 0, 3, 2));
          _vCoefficients[6] = _mm256_permute4x64_pd(temp, _MM_SHUFFLE(2, 1, 0, 3));
          _vCoefficients[7] = _mm256_permute4x64_pd(temp, _MM_SHUFFLE(3, 2, 1, 0));
      #else
          double *coefficients = (double *)ALIGNED_ALLOCA(sizeof(SIMDtype) * 4 * (N + 1), sizeof(SIMDtype));
          vCoefficients = (SIMDtype *)coefficients;

          coefficients[0] = b[0];
          coefficients[1] = b[3];
          coefficients[2] = b[2];
          coefficients[3] = b[1];

          coefficients[4] = b[1];
          coefficients[5] = b[0];
          coefficients[6] = b[3];
          coefficients[7] = b[2];

          coefficients[8] = b[2];
          coefficients[9] = b[1];
          coefficients[10] = b[0];
          coefficients[11] = b[3];

          coefficients[12] = b[3];
          coefficients[13] = b[2];
          coefficients[14] = b[1];
          coefficients[15] = b[0];

          // permutations for backward pass
          coefficients[16] = b[1];
          coefficients[17] = b[2];
          coefficients[18] = b[3];
          coefficients[19] = b[0];

          coefficients[20] = b[0];
          coefficients[21] = b[1];
          coefficients[22] = b[2];
          coefficients[23] = b[3];

          coefficients[24] = b[3];
          coefficients[25] = b[0];
          coefficients[26] = b[1];
          coefficients[27] = b[2];

          coefficients[28] = b[2];
          coefficients[29] = b[3];
          coefficients[30] = b[0];
          coefficients[31] = b[1];
      #endif
      }
      else
      {
      #ifdef __AVX__
          __m256 *_vCoefficients = (__m256 *)ALIGNED_ALLOCA(sizeof(SIMDtype) * 2 * (N + 1), sizeof(SIMDtype));
          vCoefficients = (SIMDtype *)_vCoefficients;

          __m256 temp = _mm256_castps128_ps256(_mm256_cvtpd_ps(_mm256_loadu_pd(b)));
          temp = _mm256_permute2f128_ps(temp, temp, 0);
          _vCoefficients[0] = _mm256_permute_ps(temp, _MM_SHUFFLE(1, 2, 3, 0));
          _vCoefficients[1] = _mm256_permute_ps(temp, _MM_SHUFFLE(2, 3, 0, 1));
          _vCoefficients[2] = _mm256_permute_ps(temp, _MM_SHUFFLE(3, 0, 1, 2));
          _vCoefficients[3] = _mm256_permute_ps(temp, _MM_SHUFFLE(0, 1, 2, 3));

          // permutations for backward pass
          _vCoefficients[4] = _mm256_permute_ps(temp, _MM_SHUFFLE(0, 3, 2, 1));
          _vCoefficients[5] = _mm256_permute_ps(temp, _MM_SHUFFLE(1, 0, 3, 2));
          _vCoefficients[6] = _mm256_permute_ps(temp, _MM_SHUFFLE(2, 1, 0, 3));
          _vCoefficients[7] = _mm256_permute_ps(temp, _MM_SHUFFLE(3, 2, 1, 0));
      #else
          __m128 *_vCoefficients = (__m128 *)ALIGNED_ALLOCA(sizeof(SIMDtype) * 2 * (N + 1), sizeof(SIMDtype));
          vCoefficients = (SIMDtype *)_vCoefficients;

          __m128 temp = _mm_shuffle_ps
                       (
                         _mm_cvtpd_ps(_mm_loadu_pd(b)),
                         _mm_cvtpd_ps(_mm_loadu_pd(&b[2])),
                         _MM_SHUFFLE(1, 0, 1, 0)
                       );
          _vCoefficients[0] = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 2, 3, 0));
          _vCoefficients[1] = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 3, 0, 1));
          _vCoefficients[2] = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 0, 1, 2));
          _vCoefficients[3] = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 1, 2, 3));

          // permutations for backward pass
          _vCoefficients[4] = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(0, 3, 2, 1));
          _vCoefficients[5] = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(1, 0, 3, 2));
          _vCoefficients[6] = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(2, 1, 0, 3));
          _vCoefficients[7] = _mm_shuffle_ps(temp, temp, _MM_SHUFFLE(3, 2, 1, 0));
      #endif
      }
  }
  const ssize_t Y_BLOCK_SIZE = 8;

  // X_BLOCK_SIZE * channels * sizeof(InType) had better be SIMD aligned
  const ssize_t X_BLOCK_SIZE = transposeOut ? 8
      : INT32_MAX / 2;

    #pragma omp parallel
    {
      AlignedImage<IntermediateType, sizeof(SIMDtype)> forwardFilteredTemp;   // TODO: can directly output to output buffer if !transposeOut & output type is float
      SimpleImage<IntermediateType> forwardFiltered;
      if (!convertOutput && !transposeOut)
      {
          forwardFiltered = SimpleImage<IntermediateType>((IntermediateType *)out.buffer, out.pitch);
      }
      /*
      else if (canOverwriteInput && typeid(InType) == typeid(IntermediateType))
      {
          forwardFiltered = SimpleImage<IntermediateType>((IntermediateType *)in.buffer, in.pitch);
      }*/
      else
      {
          forwardFilteredTemp.Resize(width * channels, Y_BLOCK_SIZE);
          forwardFiltered = forwardFilteredTemp;
      }

      IntermediateType *borderValues = (IntermediateType *)alloca(channels * Y_BLOCK_SIZE * sizeof(IntermediateType));

      #pragma omp for
      for (ssize_t y0 = 0; y0 < height; y0 += Y_BLOCK_SIZE)
      {
        ssize_t x = 0;
        ssize_t yBlockSize = min(height - y0, Y_BLOCK_SIZE);

          ssize_t i = 0;
          do
          {
            ssize_t color = 0;
            do
            {
              borderValues[i * channels + color] = in[y0 + i][(width - 1) * channels + color];
              ++color;
            } while (color < channels);
            ++i;
          } while (i < yBlockSize);
          
          ssize_t xBlockSize = min(max(X_BLOCK_SIZE, ssize_t(N)), width);    // try to process at least X_BLOCK_SIZE or else later, data won't be SIMD aligned
          // convolve pixels[0:FILTER_SIZE - 1]
          Convolve1DHorizontal<false, true, true, channels>(forwardFiltered,
                                               in.SubImage(0, y0),
                                               (IntermediateType *)NULL,
                                               x, x + xBlockSize, width, yBlockSize,
                                               vCoefficients,
                                               M);

        x += xBlockSize;
        while (x < width)
        {
          xBlockSize = min(width - x, X_BLOCK_SIZE);
          
          Convolve1DHorizontal<false, true, false, channels>(forwardFiltered,
                                                  in.SubImage(0, y0),
                                                  (IntermediateType *)NULL,
                                                  x, x + xBlockSize, width, yBlockSize,
                                                  vCoefficients,
                                                  M);
          x += xBlockSize;
		}

        //--------------- backward pass--------------------------
		SimpleImage <IntermediateType> floatOut;
		// if output type is fixed point, we still compute an intermediate result as float for better precision
	    if (convertOutput)
	    {
          floatOut = forwardFiltered;
	    }
	    else
	    {
		  floatOut = SimpleImage<IntermediateType>((IntermediateType *)&out[transposeOut ? 0 : y0][(transposeOut ? y0 : 0) * channels], out.pitch);
	    }
        x = width - 1;

        ssize_t lastAligned = RoundDown(width, X_BLOCK_SIZE);
        // todo: check is this really vector aligned?
        xBlockSize = min(max(width - lastAligned, ssize_t(N)), width);      // try to process more than N pixels so that later, data is SIMD aligned

		  // in-place operation (use forwardFiltered as both input & output) is possible due to internal register buffering
		if (transposeOut && !convertOutput)
            Convolve1DHorizontal<true, false, true, channels>(floatOut,
                                                           forwardFiltered,
                                                           borderValues,
                                                           x, x - xBlockSize, width, yBlockSize,
                                                           vCoefficients,
                                                           M);
		else
            Convolve1DHorizontal<false, false, true, channels>(floatOut,
                                                           forwardFiltered,
                                                           borderValues,
                                                           x, x - xBlockSize, width, yBlockSize,
                                                           vCoefficients,
                                                           M);

		  if (convertOutput)
		  {
              ssize_t outCornerX = x + 1 - xBlockSize;
			  Copy2D<transposeOut, channels>(out.SubImage((transposeOut ? y0 : outCornerX) * channels, transposeOut ? outCornerX : y0), floatOut.SubImage(outCornerX * channels, 0), xBlockSize, yBlockSize);
		  }
        x -= xBlockSize;
        while (x >= 0)
        {
          xBlockSize = min(X_BLOCK_SIZE, x + 1);

		  if (transposeOut && !convertOutput)
              Convolve1DHorizontal<true, false, false, channels>(floatOut,
                                                   forwardFiltered,
                                                   borderValues,
                                                   x, x - xBlockSize, width, yBlockSize,
                                                   vCoefficients,
                                                   M);
		  else
              Convolve1DHorizontal<false, false, false, channels>(floatOut,
                                                   forwardFiltered,
                                                   borderValues,
                                                   x, x - xBlockSize, width, yBlockSize,
                                                   vCoefficients,
                                                   M);
			
			if (convertOutput)
		    {
                ssize_t outCornerX = x + 1 - xBlockSize;
                Copy2D<transposeOut, channels>(out.SubImage((transposeOut ? y0 : outCornerX) * channels, transposeOut ? outCornerX : y0), floatOut.SubImage(outCornerX * channels, 0), xBlockSize, yBlockSize);
		    }
          x -= xBlockSize;
        }
      }
    }
}

template <bool transposeOut, int channels, typename OutType, typename InType>
void ConvolveHorizontal(SimpleImage<OutType> out, SimpleImage<InType> in, ssize_t width, ssize_t height, float sigmaX, bool canOverwriteInput = false)
{
  if (sigmaX > MAX_SIZE_FOR_SINGLE_PRECISION)
    ConvolveHorizontal<transposeOut, channels, double, OutType, InType>(out, in, width, height, sigmaX, canOverwriteInput);
  else
    ConvolveHorizontal<transposeOut, channels, float, OutType, InType>(out, in, width, height, sigmaX, canOverwriteInput);
}

// handles blocking
// input & output are color interleaved
template <typename IntermediateType, typename OutType, typename InType>
void ConvolveVertical(SimpleImage<OutType> out, SimpleImage<InType> in, ssize_t width, ssize_t height, double sigmaY)
{
  const bool convertOutput = typeid(OutType) != typeid(IntermediateType);

  const ssize_t Y_BLOCK_SIZE = 8,
                X_BLOCK_SIZE = 40;   // must be multiple of SIMD width or else say goodbye to throughput

  typedef typename MyTraits<IntermediateType>::SIMDtype SIMDtype;

    double bf[N];
    double M[N*N]; // matrix used for initialization procedure (has to be double)
    double b[N + 1];

    calcFilter(sigmaY, bf);

    for (size_t i = 0; i<N; i++)
        bf[i] = -bf[i];

    b[0] = 1; // b[0] == alpha (scaling coefficient)

    for (size_t i = 0; i<N; i++)
    {
        b[i + 1] = bf[i];
        b[0] -= b[i + 1];
    }
    b[3] = 1 - (b[0] + b[1] + b[2]);
    // Compute initialization matrix
    calcTriggsSdikaM(bf, M);

    SIMDtype *vCoefficients = (SIMDtype *)ALIGNED_ALLOCA(sizeof(SIMDtype) * (N + 1), sizeof(SIMDtype));

    for (ssize_t i = 0; i <= N; ++i)
    {
      BroadcastSIMD(vCoefficients[i], (IntermediateType)b[i]);
    }

    #pragma omp parallel
    {
      AlignedImage<IntermediateType, sizeof(SIMDtype)> forwardFiltered;    // TODO: can directly output to output buffer if output type is float
      forwardFiltered.Resize(X_BLOCK_SIZE, height);

      IntermediateType *borderValues = (IntermediateType *)alloca(X_BLOCK_SIZE * sizeof(IntermediateType));

      #pragma omp for
      for (ssize_t x0 = 0; x0 < width; x0 += X_BLOCK_SIZE)
      {
        ssize_t y = 0;
        ssize_t xBlockSize = min(width - x0, X_BLOCK_SIZE);

        ssize_t i = 0;
        do
        {
          borderValues[i] = in[height - 1][x0 + i];
          ++i;
        } while (i < xBlockSize);
          
        ssize_t yBlockSize = min(ssize_t(N), height);
          // convolve pixels[0:filterSize - 1]
          Convolve1DVertical<true, true>(forwardFiltered,
                                         in.SubImage(x0, 0),
                                         (IntermediateType *)NULL,
                                         y, y + yBlockSize, xBlockSize, height,
                                         vCoefficients,
                                         M);

        y += yBlockSize;
        while (y < height)
        {
          yBlockSize = min(height - y, Y_BLOCK_SIZE);
          
  	      Convolve1DVertical<true, false>(forwardFiltered,
                                          in.SubImage(x0, 0),
                                          (IntermediateType *)NULL,
                                          y, y + yBlockSize, xBlockSize, height,
                                          vCoefficients,
                                          M);
          y += yBlockSize;
		}

        //--------------- backward pass--------------------------
		SimpleImage<IntermediateType> floatOut;
		// if output type is fixed point, we still compute an intermediate result as float for better precision
	    if (convertOutput)
	    {
          floatOut = forwardFiltered;
	    }
	    else
	    {
		  floatOut = SimpleImage<IntermediateType>((IntermediateType *)&out[0][x0], out.pitch);
	    }
        y = height - 1;
        yBlockSize = min(ssize_t(N), height);

		// in-place operation (use forwardFiltered as both input & output) is possible due to internal register buffering
		Convolve1DVertical<false, true>(floatOut,
                                                       forwardFiltered,
                                                       borderValues,
                                                       y, y - yBlockSize, xBlockSize, height,
                                                       vCoefficients,
                                                       M);

		  if (convertOutput)
		  {
              ssize_t outCornerY = y + 1 - yBlockSize;
              Copy2D<false, 1>(out.SubImage(x0, outCornerY), floatOut.SubImage(0, outCornerY), xBlockSize, yBlockSize);
		  }
        y -= yBlockSize;
        while (y >= 0)
        {
          yBlockSize = min(Y_BLOCK_SIZE, y + 1);

		  Convolve1DVertical<false, false>(floatOut,
                                                   forwardFiltered,
                                                   borderValues,
                                                   y, y - yBlockSize, xBlockSize, y,
                                                   vCoefficients,
                                                   M);
			
			if (convertOutput)
		    {
                ssize_t outCornerY = y + 1 - yBlockSize;
                Copy2D<false, 1>(out.SubImage(x0, outCornerY), floatOut.SubImage(0, outCornerY), xBlockSize, yBlockSize);
		    }
            y -= yBlockSize;
        }
      }
    }
}

template <typename OutType, typename InType>
void ConvolveVertical(SimpleImage<OutType> out, SimpleImage<InType> in, ssize_t width, ssize_t height, float sigmaY)
{
  if (sigmaY > MAX_SIZE_FOR_SINGLE_PRECISION)
    ConvolveVertical<double>(out, in, width, height, sigmaY);
  else
    ConvolveVertical<float>(out, in, width, height, sigmaY);
}

// 2D
template <int channels, typename OutType, typename InType>
void Convolve(SimpleImage<OutType> out, SimpleImage<InType> in, ssize_t width, ssize_t height, float sigmaX, float sigmaY)
{
  using namespace std::chrono;
  typedef uint16_t HorizontalFilteredType;
  AlignedImage<HorizontalFilteredType, sizeof(__m256)> horizontalFiltered;
  
  const bool DO_TIMING = false;
  high_resolution_clock::time_point t0;
  if (DO_TIMING)
    t0 = high_resolution_clock::now();

  const bool TRANSPOSE = channels != 1;     // means for the 1st and 2nd pass to transposes output

  if (TRANSPOSE)
  {
    horizontalFiltered.Resize(height * channels, width);
  }
  else
  {
    horizontalFiltered.Resize(width * channels, height);
  }
  ConvolveHorizontal<TRANSPOSE, channels>(horizontalFiltered, in, width, height, sigmaX);

#if 0
  // save intermediate image
  float scale;
  if (typeid(InType) == typeid(uint8_t))
    scale = 1.0f;
  else if (typeid(InType) == typeid(uint16_t))
    scale = 1.0f;
  else
    scale = 1.0f;
  SaveImage("horizontal_filtered.png", horizontalFiltered, TRANSPOSE ? height : width, TRANPOSE ? width : height, channels, scale);
#endif

  if (DO_TIMING)
    cout << "Thoriz=" << duration_cast<milliseconds>(high_resolution_clock::now() - t0).count() << " ms" << endl;

  //---------------------------------------------------

  if (DO_TIMING)
    t0 = high_resolution_clock::now();
  if (TRANSPOSE)
  {
    ConvolveHorizontal<true, channels>(out, horizontalFiltered, height, width, sigmaY, true);
  }
  else
  {
    ConvolveVertical(out, horizontalFiltered, width * channels, height, sigmaY);
  }
  if (DO_TIMING)
    cout << "Tvert=" << duration_cast<milliseconds>(high_resolution_clock::now() - t0).count() << " ms" << endl;
}

#ifndef __SSSE3__
  #define DO_FIR_IN_FLOAT    // without mulhrs_epi16, int16 not competitive
#else
  #undef DO_FIR_IN_FLOAT
#endif

#ifdef DO_FIR_IN_FLOAT

// in-place (out = in) operation not allowed
template <int channels, bool symmetric, bool onBorder, typename OutType, typename InType, typename SIMD_Type>
void ConvolveHorizontalFIR(SimpleImage<OutType> out, SimpleImage<InType> in,
						   ssize_t width, ssize_t height,
						   ssize_t xStart, ssize_t xEnd,
						   SIMD_Type *vFilter, int filterSize)   // filterSize assumed to be odd
{
    if (channels == 4)
    {
        ssize_t y = 0;
        do
        {
            ssize_t x = xStart;
        #ifdef __AVX__
            const ssize_t SIMD_WIDTH = 8,
                PIXELS_PER_ITERATION = SIMD_WIDTH / channels;
            __m256 vSum,
                   leftBorderValue, rightBorderValue;
            if (onBorder)
            {
              __m128 temp;
              LoadFloats(temp, &in[y][0 * channels]);
              leftBorderValue = _mm256_setr_m128(temp, temp);

              LoadFloats(temp, &in[y][(width - 1) * channels]);
              rightBorderValue = _mm256_setr_m128(temp, temp);
            }
            goto middle;
            
            do
            {
                StoreFloats(&out[y][(x - PIXELS_PER_ITERATION) * channels], vSum);
            middle:
                if (symmetric)
                {
                    __m256 vIn;
                    vSum = vFilter[0] * LoadFloats(vIn, &in[y][x * channels]);
                    for (ssize_t i = 1; i <= filterSize; ++i)
                    {
                        __m256 filter = vFilter[i];
                        ssize_t srcX = x - i;
                        if (onBorder)
                          srcX = max(-(PIXELS_PER_ITERATION - 1), srcX);    // hack: do this for now until LoadFloats is modified to support partial loads

                        __m256 leftNeighbor, rightNeighbor;
                        LoadFloats(leftNeighbor, &in[y][srcX * channels]);
                        srcX = x + i;
                        if (onBorder)
                          srcX = min(width - 1, srcX);     // hack: do this for now until LoadFloats is modified to support partial loads

                        LoadFloats(rightNeighbor, &in[y][srcX * channels]);
                        if (onBorder)
                        {
                            __m256i notPastEnd = PartialVectorMask32(min(PIXELS_PER_ITERATION, max(ssize_t(0), width - i - x)) * channels * sizeof(float)),
                                beforeBeginning = PartialVectorMask32(min(PIXELS_PER_ITERATION, max(ssize_t(0), i - x)) * channels * sizeof(float));
                            leftNeighbor = _mm256_blendv_ps(leftNeighbor, leftBorderValue, _mm256_castsi256_ps(beforeBeginning));
                            rightNeighbor = _mm256_blendv_ps(rightBorderValue, rightNeighbor, _mm256_castsi256_ps(notPastEnd));
                        }
                        vSum = vSum + filter * (leftNeighbor + rightNeighbor);
                    }
                }
                else
                {
                    vSum = _mm256_setzero_ps();
                    ssize_t i = 0;
                    // the smaller & simpler machine code for do-while probably outweighs the cost of the extra add
                    do
                    {
                        ssize_t srcX = x - filterSize / 2 + i;
                        // todo: border not handled
                        __m256 vIn;
                        LoadFloats(vIn, &in[y][srcX * channels]);
                        vSum = vSum + vFilter[i] * vIn;
                        ++i;
                    } while (i < filterSize);
                }
                x += PIXELS_PER_ITERATION;
            } while (x < xEnd);
            StoreFloats<true>(&out[y][(x - PIXELS_PER_ITERATION) * channels], vSum, (xEnd - (x - PIXELS_PER_ITERATION)) * channels);
         #else
            do
            {
                __m128 vSum;
                if (symmetric)
                {
                    __m128 vIn;
                    LoadFloats(vIn, &in[y][x * channels]);
                    vSum = vFilter[0] * vIn;
                    for (ssize_t i = 1; i <= filterSize; ++i)
                    {
                        ssize_t srcX = x - i;
                        if (onBorder)
                            srcX = max(ssize_t(0), srcX);

                        __m128 leftNeighbor, rightNeighbor;
                        LoadFloats(leftNeighbor, &in[y][srcX * channels]);

                        srcX = x + i;
                        if (onBorder)
                            srcX = min(width - 1, srcX);
                        LoadFloats(rightNeighbor, &in[y][srcX * channels]);

                        vSum = vSum + vFilter[i] * (leftNeighbor + rightNeighbor);
                    }
                }
                else
                {
                    vSum = _mm_setzero_ps();
                    ssize_t i = 0;
                    do
                    {
                        ssize_t srcX = x - filterSize / 2 + i;
                        if (onBorder)
                          srcX = min(width - 1, max(ssize_t(0), srcX));

                        __m128 vIn;
                        LoadFloats(vIn, &in[y][srcX * channels]);
                        vSum = vSum + vFilter[i] * vIn;
                        ++i;
                    } while (i < filterSize);
                }
                StoreFloats(&out[y][x * channels], vSum);
                ++x;
            } while (x < xEnd);

          #endif
            ++y;
        } while (y < height);
    }
    else
    {
        // 1 channel
        // todo: can merge with 4 channel?
        ssize_t y = 0;
        do
        {
            ssize_t x = xStart;
        #ifdef __AVX__
            const ssize_t SIMD_WIDTH = 8;
            
            __m256 leftBorderValue, rightBorderValue;
            if (onBorder)
            {
              leftBorderValue = _mm256_set1_ps(in[y][0 * channels]);
              rightBorderValue = _mm256_set1_ps(in[y][(width - 1) * channels]);
            }
            __m256 vSum;
            // trashed performance from basic block reordering warning
            goto middle2;
            do
            {
                // write out values from previous iteration
                StoreFloats(&out[y][(x - SIMD_WIDTH) * channels], vSum);
            middle2:
                if (symmetric)
                {
                    __m256 vIn;
                    vSum = vFilter[0] * LoadFloats(vIn, &in[y][x * channels]);
                    for (ssize_t i = 1; i <= filterSize; ++i)
                    {
                        __m256 filter = vFilter[i];

                        ssize_t srcX = x - i;
                        if (onBorder)
                            srcX = max(-(SIMD_WIDTH - 1), srcX);    // hack: do this for now until LoadFloats is modified to support partial loads

                        __m256 leftNeighbor, rightNeighbor;
                        LoadFloats(leftNeighbor, &in[y][srcX * channels]);
                        srcX = x + i;
                        if (onBorder)
                            srcX = min(width - 1, srcX);     // hack: do this for now until LoadFloats is modified to support partial loads
                        LoadFloats(rightNeighbor, &in[y][srcX * channels]);

                        if (onBorder)
                        {
                          __m256i notPastEnd = PartialVectorMask32(min(SIMD_WIDTH, max(ssize_t(0), width - i - x)) * sizeof(float)),
                             beforeBeginning = PartialVectorMask32(min(SIMD_WIDTH, max(ssize_t(0), i - x)) * sizeof(float));
                          leftNeighbor = _mm256_blendv_ps(leftNeighbor, leftBorderValue, _mm256_castsi256_ps(beforeBeginning));
                          rightNeighbor = _mm256_blendv_ps(rightBorderValue, rightNeighbor, _mm256_castsi256_ps(notPastEnd));
                        }
                        vSum = vSum + filter * (leftNeighbor + rightNeighbor);
                    }
                }
                else
                {
                    vSum = _mm256_setzero_ps();
                    ssize_t i = 0;
                    // the smaller & simpler machine code for do-while probably outweighs the cost of the extra add
                    do
                    {
                        ssize_t srcX = x - filterSize / 2 + i;
                        // todo: border not handled
                        __m256 vIn;
                        LoadFloats(vIn, &in[y][srcX * channels]);
                        vSum = vSum + vFilter[i] * vIn;
                        ++i;
                    } while (i < filterSize);
                }
                x += SIMD_WIDTH;
            } while (x < xEnd);
            StoreFloats<true>(&out[y][(x - SIMD_WIDTH) * channels], vSum, xEnd - (x - SIMD_WIDTH));
        #else
            // SSE only
            const ssize_t SIMD_WIDTH = 4;
            
            __m128 leftBorderValue, rightBorderValue;
            if (onBorder)
            {
              leftBorderValue = _mm_set1_ps(in[y][0 * channels]);
              rightBorderValue = _mm_set1_ps(in[y][(width - 1) * channels]);
            }
            __m128 vSum;
            // trashed performance from basic block reordering warning
            goto middle2;
            do
            {
                // write out values from previous iteration
                StoreFloats(&out[y][(x - SIMD_WIDTH) * channels], vSum);
            middle2:
                if (symmetric)
                {
                    __m128 vIn;
                    vSum = vFilter[0] * LoadFloats(vIn, &in[y][x * channels]);
                    for (ssize_t i = 1; i <= filterSize; ++i)
                    {
                        __m128 filter = vFilter[i];

                        ssize_t srcX = x - i;
                        if (onBorder)
                            srcX = max(-(SIMD_WIDTH - 1), srcX);    // hack: do this for now until LoadFloats is modified to support partial loads

                        __m128 leftNeighbor, rightNeighbor;
                        LoadFloats(leftNeighbor, &in[y][srcX * channels]);
                        srcX = x + i;
                        if (onBorder)
                            srcX = min(width - 1, srcX);     // hack: do this for now until LoadFloats is modified to support partial loads
                        LoadFloats(rightNeighbor, &in[y][srcX * channels]);

                        if (onBorder)
                        {
                          __m128i notPastEnd = PartialVectorMask(min(SIMD_WIDTH, max(ssize_t(0), width - i - x)) * sizeof(float)),
                             beforeBeginning = PartialVectorMask(min(SIMD_WIDTH, max(ssize_t(0), i - x)) * sizeof(float));
                          leftNeighbor = Select(leftNeighbor, leftBorderValue, _mm_castsi128_ps(beforeBeginning));
                          rightNeighbor = Select(rightBorderValue, rightNeighbor, _mm_castsi128_ps(notPastEnd));
                        }
                        vSum = vSum + filter * (leftNeighbor + rightNeighbor);
                    }
                }
                else
                {
                    vSum = _mm_setzero_ps();
                    ssize_t i = 0;
                    // the smaller & simpler machine code for do-while probably outweighs the cost of the extra add
                    do
                    {
                        ssize_t srcX = x - filterSize / 2 + i;
                        // todo: border not handled
                        __m128 vIn;
                        LoadFloats(vIn, &in[y][srcX * channels]);
                        vSum = vSum + vFilter[i] * vIn;
                        ++i;
                    } while (i < filterSize);
                }
                x += SIMD_WIDTH;
            } while (x < xEnd);
            StoreFloats<true>(&out[y][(x - SIMD_WIDTH) * channels], vSum, xEnd - (x - SIMD_WIDTH));
        #endif
            ++y;
        } while (y < height);
    }
}

#else

// DO_FIR_IN_INT16

// in-place (out = in) operation not allowed
template <int channels, bool symmetric, bool onBorder, typename OutType, typename InType, typename SIMD_Type>
void ConvolveHorizontalFIR(SimpleImage<OutType> out, SimpleImage<InType> in,
						   ssize_t width, ssize_t height,
						   ssize_t xStart, ssize_t xEnd,
						   SIMD_Type *vFilter, int filterSize)
{
    if (channels == 4)
    {
        ssize_t y = 0;
        do
        {
        #ifdef __AVX2__
            int16_t *convertedIn;
            if (typeid(InType) == typeid(int16_t))
            {
              convertedIn = (int16_t *)&in[y][0];
            }
            else
            {
              convertedIn = (int16_t *)ALIGNED_ALLOCA(RoundUp(width * channels, ssize_t(sizeof(__m256i) / sizeof(int16_t))) * sizeof(int16_t), sizeof(__m256i));
              for (ssize_t x = 0; x < width * channels; x += 16)
              {
                __m128i u8 = _mm_loadu_si128((__m128i *)&in[y][x]);
                __m256i i16 = _mm256_slli_epi16(_mm256_cvtepu8_epi16(u8), 6);
                _mm256_store_si256((__m256i *)&convertedIn[x], i16);
              }
            }
            ssize_t x = xStart;
            const ssize_t SIMD_WIDTH = 16,
                PIXELS_PER_ITERATION = SIMD_WIDTH / channels;
            __m256i vSum;
            __m256i leftBorderValue, rightBorderValue;
            if (onBorder)
            {
                __m128i temp = _mm_set1_epi64x(*(int64_t *)&convertedIn[0 * channels]);
                leftBorderValue = _mm256_setr_m128i(temp, temp);
                temp = _mm_set1_epi64x(*(int64_t *)&convertedIn[(width - 1) * channels]);
                rightBorderValue = _mm256_setr_m128i(temp, temp);
            }
            goto middle2;
            do
            {
                ScaleAndStoreInt16(&out[y][(x - PIXELS_PER_ITERATION) * channels], vSum);
            middle2:
                if (symmetric)
                {
                    __m256i center = _mm256_loadu_si256((__m256i *)&convertedIn[x * channels]);
                    vSum = _mm256_mulhrs_epi16(vFilter[0], center);
                    for (ssize_t i = 1; i <= filterSize; ++i)
                    {
                        __m256i filter = vFilter[i];

                        ssize_t srcX = x - i;
                        if (onBorder)
                            srcX = max(-(PIXELS_PER_ITERATION - 1), srcX);     // todo: use this for now until LoadAndScaleToInt16() supports partial loads

                        __m256i leftNeighbor, rightNeighbor;
                        leftNeighbor = _mm256_loadu_si256((__m256i *)&convertedIn[srcX * channels]);

                        srcX = x + i;
                        if (onBorder)
                            srcX = min(width - 1, srcX);
                        rightNeighbor = _mm256_loadu_si256((__m256i *)&convertedIn[srcX * channels]);

                        if (onBorder)
                        {
                            __m256i leftMask = PartialVectorMask32(min(PIXELS_PER_ITERATION, max(ssize_t(0), i - x)) * channels * sizeof(int16_t)),
                                rightMask = PartialVectorMask32(min(PIXELS_PER_ITERATION, width - (x + i)) * channels * sizeof(int16_t));
                            leftNeighbor = _mm256_blendv_epi8(leftNeighbor, leftBorderValue, leftMask);
                            rightNeighbor = _mm256_blendv_epi8(rightBorderValue, rightNeighbor, rightMask);
                        }
                        vSum = _mm256_adds_epi16(vSum, _mm256_mulhrs_epi16(filter, _mm256_adds_epi16(leftNeighbor, rightNeighbor)));
                    }
                }
                else
                {
                    throw 0;
                }
                x += PIXELS_PER_ITERATION;
            } while (x < xEnd);
            ScaleAndStoreInt16<true>(&out[y][(x - PIXELS_PER_ITERATION) * channels], vSum, (xEnd - (x - PIXELS_PER_ITERATION)) * channels);
        #else
            // SSSE3 only
            int16_t *convertedIn;
            if (typeid(InType) == typeid(int16_t))
            {
              convertedIn = (int16_t *)&in[y][0];
            }
            else
            {
              convertedIn = (int16_t *)ALIGNED_ALLOCA(RoundUp(width * channels, ssize_t(sizeof(__m128i) / sizeof(int16_t))) * sizeof(int16_t), sizeof(__m128i));
              for (ssize_t x = 0; x < width * channels; x += 8)
              {
                  __m128i u8 = _mm_loadl_epi64((__m128i *)&in[y][x]);
                  __m128i i16 = _mm_slli_epi16(_mm_cvtepu8_epi16(u8), 6);
                  _mm_store_si128((__m128i *)&convertedIn[x], i16);
              }
            }
            ssize_t x = xStart;
            const ssize_t SIMD_WIDTH = 8,
                PIXELS_PER_ITERATION = SIMD_WIDTH / channels;
            __m128i vSum;
            __m128i leftBorderValue, rightBorderValue;
            if (onBorder)
            {
                leftBorderValue = _mm_set1_epi64x(*(int64_t *)&convertedIn[0 * channels]);
                rightBorderValue = _mm_set1_epi64x(*(int64_t *)&convertedIn[(width - 1) * channels]);
            }
            goto middle3;
            do
            {
                ScaleAndStoreInt16(&out[y][(x - PIXELS_PER_ITERATION) * channels], vSum);
            middle3:
                if (symmetric)
                {
                    __m128i center;
                    vSum = _mm_mulhrs_epi16(Cast256To128(vFilter[0]), LoadAndScaleToInt16(center, &convertedIn[x * channels]));
                    for (ssize_t i = 1; i <= filterSize; ++i)
                    {
                        __m128i filter = Cast256To128(vFilter[i]);

                        ssize_t srcX = x - i;
                        if (onBorder)
                            srcX = max(-(PIXELS_PER_ITERATION - 1), srcX);     // todo: use this for now until LoadAndScaleToInt16() supports partial loads

                        __m128i leftNeighbor = _mm_loadu_si128((__m128i *)&convertedIn[srcX * channels]);

                        srcX = x + i;
                        if (onBorder)
                            srcX = min(width - 1, srcX);

                        __m128i rightNeighbor = _mm_loadu_si128((__m128i *)&convertedIn[srcX * channels]);

                        if (onBorder)
                        {
                            __m128i leftMask = PartialVectorMask(min(PIXELS_PER_ITERATION, max(ssize_t(0), i - x)) * channels * sizeof(int16_t)),
                                rightMask = PartialVectorMask(min(PIXELS_PER_ITERATION, width - (x + i)) * channels * sizeof(int16_t));
                            leftNeighbor = _mm_blendv_epi8(leftNeighbor, leftBorderValue, leftMask);
                            rightNeighbor = _mm_blendv_epi8(rightBorderValue, rightNeighbor, rightMask);
                        }
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(filter, _mm_adds_epi16(leftNeighbor, rightNeighbor)));
                    }
                }
                else
                {
                    throw 0;
                }
                x += PIXELS_PER_ITERATION;
            } while (x < xEnd);
            ScaleAndStoreInt16<true>(&out[y][(x - PIXELS_PER_ITERATION) * channels], vSum, (xEnd - (x - PIXELS_PER_ITERATION)) * channels);
        #endif
            
            ++y;
        } while (y < height);
    }
    else
    {
    #ifdef __GNUC__
        const static void *labels[] =
        {
            NULL,
            &&remainder1,
            &&remainder2,
            &&remainder3,
            &&remainder4,

            &&remainder5,
            &&remainder6,
            &&remainder7,
            &&remainder8
        };
    #endif
        // 1 channel
        // todo: can merge with 4 channel?
        ssize_t y = 0;
        do
        {
            int16_t *convertedIn;
            if (typeid(InType) == typeid(int16_t))
            {
              convertedIn = (int16_t *)&in[y][0];
            }
            else
            {
              convertedIn = (int16_t *)ALIGNED_ALLOCA(RoundUp(width, ssize_t(sizeof(__m256i) / sizeof(int16_t))) * sizeof(int16_t), sizeof(__m256i));
              #ifdef __AVX2__
              for (ssize_t x = 0; x < width; x += 16)
              {
                __m128i u8 = _mm_loadu_si128((__m128i *)&in[y][x]);
                __m256i i16 = _mm256_slli_epi16(_mm256_cvtepu8_epi16(u8), 6);
                _mm256_store_si256((__m256i *)&convertedIn[x], i16);
              }
              #else
              for (ssize_t x = 0; x < width; x += 8)
              {
                  __m128i u8 = _mm_loadl_epi64((__m128i *)&in[y][x]);
                  __m128i i16 = _mm_slli_epi16(_mm_cvtepu8_epi16(u8), 6);
                  _mm_store_si128((__m128i *)&convertedIn[x], i16);
              }
              #endif
            }
            ssize_t x = xStart;
            const ssize_t SIMD_WIDTH = 8;
            __m128i vSum;
            __m128i leftBorderValue, rightBorderValue;
            if (onBorder)
            {
                leftBorderValue = _mm_set1_epi16(convertedIn[0 * channels]);
                rightBorderValue = _mm_set1_epi16(convertedIn[(width - 1) * channels]);
            }
            goto middle;
            do
            {
                ScaleAndStoreInt16(&out[y][x - SIMD_WIDTH], vSum);
            middle:
                if (symmetric)
                {
                #ifdef __GNUC__
                    // up to 1.2x faster by using palignr instead of unaligned loads!
                    // the greatest difficulty with using palignr is that the offset must be a compile time constant
                    // solution? Duff's device
                    __m128i leftHalf[2],
                           rightHalf[2],
                           center = _mm_loadu_si128((__m128i *)&convertedIn[x]);

                    if (onBorder)
                    {
                        __m128i mask = PartialVectorMask(min(SIMD_WIDTH, width - x) * sizeof(int16_t));
                        center = _mm_blendv_epi8(rightBorderValue, center, mask);
                    }
                    rightHalf[0] = leftHalf[1] = center;
                    
                    vSum = _mm_mulhrs_epi16(Cast256To128(vFilter[0]), rightHalf[0]);
                    
                    ssize_t base = 0;
                    while (base < filterSize)
                    {
                        leftHalf[0] = rightHalf[0];
                        rightHalf[1] = leftHalf[1];
                        rightHalf[0] = _mm_loadu_si128((__m128i *)&convertedIn[x + base + 8]);
                        leftHalf[1] = _mm_loadu_si128((__m128i *)&convertedIn[x - base - 8]);

                        if (onBorder)
                        {
                            __m128i leftMask = PartialVectorMask(min(SIMD_WIDTH, max(ssize_t(0), (base + 8) - x)) * sizeof(int16_t)),
                                    rightMask = PartialVectorMask(min(SIMD_WIDTH, width - (x + base + 8)) * sizeof(int16_t));
                            leftHalf[1] = _mm_blendv_epi8(leftHalf[1], leftBorderValue, leftMask);
                            rightHalf[0] = _mm_blendv_epi8(rightBorderValue, rightHalf[0], rightMask);
                        }

                        goto *labels[min(ssize_t(8), filterSize - base)];
                        __m128i v, v2;
                    remainder8:
                        v = rightHalf[0];     // same as palignr(right, left, 16)
                        v2 = leftHalf[1];
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(Cast256To128(vFilter[base + 8]), _mm_adds_epi16(v, v2)));
                    remainder7:
                        v = _mm_alignr_epi8(rightHalf[0], leftHalf[0], 14);
                        v2 = _mm_alignr_epi8(rightHalf[1], leftHalf[1], 2);
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(Cast256To128(vFilter[base + 7]), _mm_adds_epi16(v, v2)));
                    remainder6:
                        v = _mm_alignr_epi8(rightHalf[0], leftHalf[0], 12);
                        v2 = _mm_alignr_epi8(rightHalf[1], leftHalf[1], 4);
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(Cast256To128(vFilter[base + 6]), _mm_adds_epi16(v, v2)));
                    remainder5:
                        v = _mm_alignr_epi8(rightHalf[0], leftHalf[0], 10);
                        v2 = _mm_alignr_epi8(rightHalf[1], leftHalf[1], 6);
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(Cast256To128(vFilter[base + 5]), _mm_adds_epi16(v, v2)));
                    remainder4:
                        v = _mm_alignr_epi8(rightHalf[0], leftHalf[0], 8);
                        v2 = _mm_alignr_epi8(rightHalf[1], leftHalf[1], 8);
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(Cast256To128(vFilter[base + 4]), _mm_adds_epi16(v, v2)));
                    remainder3:
                        v = _mm_alignr_epi8(rightHalf[0], leftHalf[0], 6);
                        v2 = _mm_alignr_epi8(rightHalf[1], leftHalf[1], 10);
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(Cast256To128(vFilter[base + 3]), _mm_adds_epi16(v, v2)));
                    remainder2:
                        v = _mm_alignr_epi8(rightHalf[0], leftHalf[0], 4);
                        v2 = _mm_alignr_epi8(rightHalf[1], leftHalf[1], 12);
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(Cast256To128(vFilter[base + 2]), _mm_adds_epi16(v, v2)));
                    remainder1:
                        v = _mm_alignr_epi8(rightHalf[0], leftHalf[0], 2);
                        v2 = _mm_alignr_epi8(rightHalf[1], leftHalf[1], 14);
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(Cast256To128(vFilter[base + 1]), _mm_adds_epi16(v, v2)));
                        base += 8;
                    }
                  #else
                    __m128i center;
                    vSum = _mm_mulhrs_epi16(Cast256To128(vFilter[0]), LoadAndScaleToInt16(center, &convertedIn[x * channels]));
                    for (ssize_t i = 1; i <= filterSize; ++i)
                    {
                        __m128i filter = Cast256To128(vFilter[i]);

                        ssize_t srcX = x - i;
                        if (onBorder)
                            srcX = max(-(SIMD_WIDTH - 1), srcX);     // todo: use this for now until LoadAndScaleToInt16() supports partial loads

                        __m128i leftNeighbor = _mm_loadu_si128((__m128i *)&convertedIn[srcX * channels]);

                        srcX = x + i;
                        if (onBorder)
                            srcX = min(width - 1, srcX);

                        __m128i rightNeighbor = _mm_loadu_si128((__m128i *)&convertedIn[srcX * channels]);

                        if (onBorder)
                        {
                            __m128i leftMask = PartialVectorMask(min(SIMD_WIDTH, max(ssize_t(0), i - x)) * sizeof(int16_t)),
                                rightMask = PartialVectorMask(min(SIMD_WIDTH, width - (x + i)) * sizeof(int16_t));
                            leftNeighbor = _mm_blendv_epi8(leftNeighbor, leftBorderValue, leftMask);
                            rightNeighbor = _mm_blendv_epi8(rightBorderValue, rightNeighbor, rightMask);
                        }
                        vSum = _mm_adds_epi16(vSum, _mm_mulhrs_epi16(filter, _mm_adds_epi16(leftNeighbor, rightNeighbor)));
                    }
                  #endif
                }
                else
                {
                    throw 0;
                }
                x += SIMD_WIDTH;
            } while (x < xEnd);
            ScaleAndStoreInt16<true>(&out[y][x - SIMD_WIDTH], vSum, xEnd - (x - SIMD_WIDTH));
            ++y;
        } while (y < height);
    }
}
#endif

// handles blocking
// in-place (out = in) operation not allowed
template <int channels, typename OutType, typename InType>
void ConvolveHorizontalFIR(SimpleImage<OutType> out,
                           SimpleImage<InType> in,
						   ssize_t width, ssize_t height, float sigmaX)
{
#ifdef DO_FIR_IN_FLOAT
  typedef MyTraits<float>::SIMDtype SIMDtype;
#else
  typedef MyTraits<int16_t>::SIMDtype SIMDtype;
#endif

    ssize_t halfFilterSize = _effect_area_scr(sigmaX);
    float *filter = (float *)alloca((halfFilterSize + 1) * sizeof(float));
    _make_kernel(filter, sigmaX);
    
    SIMDtype *vFilter = (SIMDtype *)ALIGNED_ALLOCA((halfFilterSize + 1) * sizeof(SIMDtype), sizeof(SIMDtype));
	
    for (ssize_t i = 0; i <= halfFilterSize; ++i)
    {
      #ifdef DO_FIR_IN_FLOAT
        BroadcastSIMD(vFilter[i], filter[i]);
      #else
        BroadcastSIMD(vFilter[i], clip_round_cast<int16_t>(filter[i] * 32768));
      #endif
    }
	
    const ssize_t IDEAL_Y_BLOCK_SIZE = 1;    // pointless for now, but might be needed in the future when SIMD code processes 2 rows at a time
	
    #pragma omp parallel
	{
    #pragma omp for
	for (ssize_t y = 0; y < height; y += IDEAL_Y_BLOCK_SIZE)
	{
        ssize_t yBlockSize = min(height - y, IDEAL_Y_BLOCK_SIZE);

        ssize_t nonBorderStart = min(width, RoundUp(halfFilterSize, ssize_t(sizeof(__m256) / channels / sizeof(InType))));    // so that data for non-border region is vector aligned
        if (nonBorderStart < width - halfFilterSize)
          ConvolveHorizontalFIR<channels, true, false>(out.SubImage(0, y),
                                                       in.SubImage(0, y),
                                                       width, yBlockSize,
                                                       nonBorderStart, width - halfFilterSize,
                                                       vFilter, halfFilterSize);
        ssize_t xStart = 0,
        xEnd = nonBorderStart;
      processEnd:
        ConvolveHorizontalFIR<channels, true, true>(out.SubImage(0, y),
                                                    in.SubImage(0, y),
                                                    width, yBlockSize,
                                                    xStart, xEnd,
                                                    vFilter, halfFilterSize);
        if (xStart == 0)
        {
            // avoid inline happy compiler from inlining another call to ConvolveHorizontalFIR()
            xStart = max(nonBorderStart, width - halfFilterSize);   // don't refilter anything in case the 2 border regions overlap
            xEnd = width;
            goto processEnd;
        }
    }
	}   // omp parallel
}

#ifdef DO_FIR_IN_FLOAT
// in-place (out = in) operation not allowed
template <bool symmetric, bool onBorder, typename OutType, typename InType, typename SIMD_Type>
void ConvolveVerticalFIR(SimpleImage<OutType> out, SimpleImage<InType> in,
                         ssize_t width, ssize_t height,
                         ssize_t yStart, ssize_t yEnd,
                         SIMD_Type *vFilter, int filterSize)
{
    ssize_t y = yStart;
    do
    {
        ssize_t x = 0;
#ifdef __AVX__
        const ssize_t SIMD_WIDTH = 8;
        __m256 vSum;
        goto middle;
        do
        {
            StoreFloats(&out[y][x - SIMD_WIDTH], vSum);  // write out data from previous iteration
        middle:
            if (symmetric)
            {
                __m256 vIn;
                LoadFloats(vIn, &in[y][x]);
                vSum = vFilter[0] * vIn;
                for (ssize_t i = 1; i <= filterSize; ++i)
                {
                    ssize_t srcY = y - i;
                    if (onBorder)
                        srcY = max(ssize_t(0), srcY);
                    __m256 bottom, top;
                    LoadFloats(bottom, &in[srcY][x]);

                    srcY = y + i;
                    if (onBorder)
                        srcY = min(height - 1, srcY);
                    LoadFloats(top, &in[srcY][x]);

                    vSum = vSum + vFilter[i] * (bottom + top);
                }
            }
            else
            {
                // wouldn't be surprised if the smaller & simpler do-while code outweighs the cost of the extra add
                vSum = _mm256_setzero_ps();
                ssize_t i = 0;
                do
                {
                    ssize_t srcY = y - filterSize / 2 + i;
                    if (onBorder)
                        srcY = min(height - 1, max(ssize_t(0), srcY));
                    __m256 vIn;
                    LoadFloats(vIn, &in[srcY][x]);
                    vSum = vSum + vFilter[i] * vIn;
                    ++i;
                } while (i < filterSize);
            }
            x += SIMD_WIDTH;
        } while (x < width);
        StoreFloats<true>(&out[y][x - SIMD_WIDTH], vSum, width - (x - SIMD_WIDTH));
#else
        // for SSE only
        const ssize_t SIMD_WIDTH = 4;
        __m128 vSum;
        goto middle;
        do
        {
            StoreFloats(&out[y][x - SIMD_WIDTH], vSum);    // write out data from previous iteration
        middle:
            if (symmetric)
            {
                __m128 vIn;
                LoadFloats(vIn, &in[y][x]);
                vSum = Cast256To128(vFilter[0]) * vIn;
                for (ssize_t i = 1; i <= filterSize; ++i)
                {
                    ssize_t srcY = y - i;
                    if (onBorder)
                        srcY = max(ssize_t(0), srcY);
                    __m128 bottom, top;
                    LoadFloats(bottom, &in[srcY][x]);

                    srcY = y + i;
                    if (onBorder)
                        srcY = min(height - 1, srcY);
                    LoadFloats(top, &in[srcY][x]);

                    vSum = vSum + Cast256To128(vFilter[i]) * (bottom + top);
                }
            }
            else
            {
                vSum = _mm_setzero_ps();
                ssize_t i = 0;
                do
                {
                    ssize_t srcY = y - filterSize / 2 + i;
                    if (onBorder)
                        srcY = min(height - 1, max(ssize_t(0), srcY));
                    __m128 _vFilter = Cast256To128(vFilter[i]);

                    __m128 vIn;
                    LoadFloats(vIn, &in[srcY][x]);
                    vSum = vSum + _vFilter * vIn;
                    ++i;
                } while (i < filterSize);
            }
            x += SIMD_WIDTH;
        } while (x < width);
        StoreFloats<true>(&out[y][x - SIMD_WIDTH], vSum, width - (x - SIMD_WIDTH));
#endif
        ++y;
    } while (y < yEnd);
}

#else   // DO_FIR_IN_FLOAT

// in-place (out = in) operation not allowed
template <bool symmetric, bool onBorder, typename OutType, typename InType, typename SIMD_Type>
void ConvolveVerticalFIR(SimpleImage<OutType> out, SimpleImage<InType> in,
                         ssize_t width, ssize_t height,
                         ssize_t yStart, ssize_t yEnd,
                         SIMD_Type *vFilter, int filterSize)
{
    ssize_t y = yStart;
    do
    {
        ssize_t x = 0;
    #ifdef __AVX2__
        const ssize_t SIMD_WIDTH = 16;
        __m256i vSum;
        goto middle;
        do
        {
            ScaleAndStoreInt16(&out[y][x - SIMD_WIDTH], vSum);   // store data from previous iteration
        middle:
            if (symmetric)
            {
                __m256i center;
                vSum = _mm256_mulhrs_epi16(vFilter[0], LoadAndScaleToInt16(center, &in[y][x]));
                for (ssize_t i = 1; i <= filterSize; ++i)
                {
                    __m256i filter = vFilter[i];
                    ssize_t srcY = y + i;
                    if (onBorder)
                        srcY = min(srcY, height - 1);
                    __m256i topNeighbor;
                    LoadAndScaleToInt16(topNeighbor, &in[srcY][x]);

                    srcY = y - i;
                    if (onBorder)
                        srcY = max(srcY, ssize_t(0));
                    __m256i bottomNeighbor;
                    LoadAndScaleToInt16(bottomNeighbor, &in[srcY][x]);
                    vSum = _mm256_adds_epi16
                           (
                             vSum,
                             _mm256_mulhrs_epi16
                             (
                               filter,
                               _mm256_adds_epi16(bottomNeighbor, topNeighbor)
                             )
                           );
                }
            }
            else
            {
                throw 0;
            }
            x += SIMD_WIDTH;
        } while (x < width);
        ScaleAndStoreInt16<true>(&out[y][x - SIMD_WIDTH], vSum, width - (x - SIMD_WIDTH));
    #else
        const ssize_t SIMD_WIDTH = 8;
        __m128i vSum;
        goto middle;
        do
        {
            ScaleAndStoreInt16(&out[y][x - SIMD_WIDTH], vSum);   // store data from previous iteration
        middle:
            if (symmetric)
            {
                __m128i center;
                vSum = _mm_mulhrs_epi16(Cast256To128(vFilter[0]), LoadAndScaleToInt16(center, &in[y][x]));
                for (ssize_t i = 1; i <= filterSize; ++i)
                {
                    __m128i filter = Cast256To128(vFilter[i]);
                    ssize_t srcY = y + i;
                    if (onBorder)
                        srcY = min(srcY, height - 1);
                    __m128i topNeighbor;
                    LoadAndScaleToInt16(topNeighbor, &in[srcY][x]);

                    srcY = y - i;
                    if (onBorder)
                        srcY = max(srcY, ssize_t(0));
                    __m128i bottomNeighbor;
                    LoadAndScaleToInt16(bottomNeighbor, &in[srcY][x]);
                    vSum = _mm_adds_epi16
                           (
                             vSum,
                             _mm_mulhrs_epi16
                             (
                               filter,
                               _mm_adds_epi16(bottomNeighbor, topNeighbor)
                             )
                           );
                }
            }
            else
            {
                throw 0;
            }
            x += SIMD_WIDTH;
        } while (x < width);
        ScaleAndStoreInt16<true>(&out[y][x - SIMD_WIDTH], vSum, width - (x - SIMD_WIDTH));
    #endif
        ++y;
    } while (y < yEnd);
}
#endif

// in-place (out = in) operation not allowed
template <typename OutType, typename InType>
void ConvolveVerticalFIR(SimpleImage<OutType> out,
                         SimpleImage<InType> in,
                         ssize_t width, ssize_t height,
                         float sigmaY)
{
#ifdef DO_FIR_IN_FLOAT
  typedef MyTraits<float>::SIMDtype SIMDtype;
#else
  typedef MyTraits<int16_t>::SIMDtype SIMDtype;
#endif
    int halfFilterSize = _effect_area_scr(sigmaY);
    
    float *filter = (float *)alloca((halfFilterSize + 1) * sizeof(float));
    
    _make_kernel(filter, sigmaY);
   
    SIMDtype *vFilter = (SIMDtype *)ALIGNED_ALLOCA((halfFilterSize + 1) * sizeof(SIMDtype), sizeof(SIMDtype));
	
	for (ssize_t i = 0; i <= halfFilterSize; ++i)
	{
    #ifdef DO_FIR_IN_FLOAT
      BroadcastSIMD(vFilter[i], filter[i]);
    #else
      BroadcastSIMD(vFilter[i], clip_round_cast<int16_t>(filter[i] * 32768));
    #endif
	}

	const ssize_t IDEAL_Y_BLOCK_SIZE = 2;    // currently, no advantage to making > 1
	
    #pragma omp parallel
	{
    #pragma omp for
	for (ssize_t y = 0; y < height; y += IDEAL_Y_BLOCK_SIZE)
	{
        ssize_t yBlockSize = min(height - y, IDEAL_Y_BLOCK_SIZE);
        bool onBorder = y < halfFilterSize || y + IDEAL_Y_BLOCK_SIZE + halfFilterSize > height;
        if (onBorder)
        {
          ConvolveVerticalFIR<true, true>(out, in,
                                          width, height,
                                          y, y + yBlockSize,
                                          vFilter, halfFilterSize);
                
        }
        else
        {
          ConvolveVerticalFIR<true, false>(out, in,
                                           width, height,
                                           y, y + yBlockSize,
                                           vFilter, halfFilterSize);
        }
	}
	}   // omp parallel
}

template <int channels, typename OutType, typename InType>
void ConvolveFIR(SimpleImage<OutType> out, SimpleImage<InType> in, ssize_t width, ssize_t height, float sigmaX, float sigmaY)
{
  using namespace std::chrono;
#ifdef DO_FIR_IN_FLOAT
   AlignedImage<float, sizeof(__m256)> horizontalFiltered;
#else
  AlignedImage<int16_t, sizeof(__m256)> horizontalFiltered;
#endif
   horizontalFiltered.Resize(width * channels, height);
   
   const bool DO_TIMING = false;

   high_resolution_clock::time_point t0;
   if (DO_TIMING)
     t0 = high_resolution_clock::now();

   ConvolveHorizontalFIR<channels>(horizontalFiltered, in, width, height, sigmaX);
   
   if (DO_TIMING)
   {
     auto t1 = high_resolution_clock::now();
     cout << "T_horiz=" << duration_cast<milliseconds>(t1 - t0).count() << " ms" << endl;
     t0 = t1;
   }
   
   // todo: use sliding window to reduce cache pollution
   float scale = 1.0f;
#ifndef DO_FIR_IN_FLOAT
   scale = 1.0f / 64;
#endif

   //SaveImage("horizontal_filtered.png", horizontalFiltered, width, height, channels, scale);
   ConvolveVerticalFIR(out, horizontalFiltered, width * channels, height, sigmaY);
   if (DO_TIMING)
     cout << "T_v=" << duration_cast<milliseconds>(high_resolution_clock::now() - t0).count() << " ms" << endl;
}
