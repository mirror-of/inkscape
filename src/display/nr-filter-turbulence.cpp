/*
 * feTurbulence filter primitive renderer
 *
 * Authors:
 *   World Wide Web Consortium <http://www.w3.org/>
 *   Felipe Corrêa da Silva Sanches <juca@members.fsf.org>
 *
 * This file has a considerable amount of code adapted from
 *  the W3C SVG filter specs, available at:
 *  http://www.w3.org/TR/SVG11/filters.html#feTurbulence
 *
 * W3C original code is licensed under the terms of
 *  the (GPL compatible) W3C® SOFTWARE NOTICE AND LICENSE:
 *  http://www.w3.org/Consortium/Legal/2002/copyright-software-20021231
 *
 * Copyright (C) 2007 authors
 * Released under GNU GPL version 2 (or later), read the file 'COPYING' for more information
 */

#include "display/cairo-templates.h"
#include "display/cairo-utils.h"
#include "display/nr-filter.h"
#include "display/nr-filter-turbulence.h"
#include "display/nr-filter-units.h"
#include "display/nr-filter-utils.h"
#include <math.h>
#include "SIMD_functions.h"

namespace Inkscape {
namespace Filters{

typedef __v4si VINT;

#ifndef __SSE4_1__
// only works for non-negative numbers, seems to be OK for turbulence function
__m128 _mm_floor_ps(__m128 x)
{
  return _mm_cvtepi32_ps(_mm_cvttps_epi32(x));
}

#else
__m128i Select(__m128i a, __m128i b, __m128i selectors)
{
  return  _mm_blendv_epi8(a, b, selectors);
}
#endif

__m128 Abs(__m128 x)
{
  return _mm_castsi128_ps(_mm_castps_si128(x) & _mm_set1_epi32(0x7fffffff));
}

inline VINT
premul_alpha_SIMD(const VINT color, const VINT alpha)
{
  VINT temp = alpha * color + (VINT)_mm_set1_epi32(128);
  return (temp + (temp >> 8)) >> 8;
}


VINT Clamp(__m128i x)
{
  return (VINT)_mm_min_epi32(_mm_max_epi32(x, _mm_set1_epi32(0)), _mm_set1_epi32(255));
}

#ifndef __SSE4_1__
int _mm_extract_epi32(__m128i v, int lane)
{
    union
    {
        int elements[4];
        __m128i vec;
    };
    vec = v;
    return elements[lane];
}
#endif

template <typename IntType>
VINT Gather(IntType *base, VINT _offsets)
{
  __m128i offsets = (__m128i)offsets;
  return _mm_set_epi32(base[_mm_extract_epi32(offsets, 3)], base[_mm_extract_epi32(offsets, 2)], base[_mm_extract_epi32(offsets, 1)], base[_mm_extract_epi32(offsets, 0)]);
}

// todo: use traits so that only 1 gather function needed
__m128 GatherFloat(const float *base, VINT _offsets)
{
  __m128i offsets = (__m128i)offsets;
  return _mm_set_ps(base[_mm_extract_epi32(offsets, 3)], base[_mm_extract_epi32(offsets, 2)], base[_mm_extract_epi32(offsets, 1)], base[_mm_extract_epi32(offsets, 0)]);
}

// specialized
__m128 GatherFloat(const float *base, int offsets[4])
{
  return _mm_set_ps(base[offsets[3]], base[offsets[2]], base[offsets[1]], base[offsets[0]]);
}

#ifdef __AVX2__
#include <immintrin.h>

typedef __v8si VINT8;

__m256i Select256(__m256i a, __m256i b, __m256i selectors)
{
  return _mm256_blendv_epi8(a, b, selectors);
}

__m256 Abs256(__m256 x)
{
	return _mm256_castsi256_ps(_mm256_castps_si256(x) & _mm256_set1_epi32(0x7fffffff));
}

inline VINT8
premul_alpha_SIMD256(const VINT8 color, const VINT8 alpha)
{
    VINT8 temp = alpha * color + (VINT8)_mm256_set1_epi32(128);
    return (temp + (temp >> 8)) >> 8;
}
VINT8 Clamp256(__m256i x)
{
  return (VINT8)_mm256_min_epi32(_mm256_max_epi32(x, _mm256_set1_epi32(0)), _mm256_set1_epi32(255));
}

template <typename IntType>
__m256i Gather256(IntType *base, VINT8 offsets)
{
  return  _mm256_i32gather_epi32(base, offsets, 4);
}

__m256 GatherFloat256(const float *base, VINT8 offsets)
{
  return _mm256_i32gather_ps(base, (__m256i)offsets, 4);
}

#endif

class TurbulenceGenerator {
public:
    TurbulenceGenerator() :
        _tile(),
        _baseFreq(),
        _latticeSelector(),
        _gradient(),
        _seed(0),
        _octaves(0),
        _stitchTiles(false),
        _wrapx(0),
        _wrapy(0),
        _wrapw(0),
        _wraph(0),
        _inited(false),
        _fractalnoise(false)
    {}

    void init(long seed, Geom::Rect const &tile, Geom::Point const &freq, bool stitch,
        bool fractalnoise, int octaves)
    {
        // setup random number generator
        _setupSeed(seed);

        // set values
        _tile = tile;
        _baseFreq = freq;
        _stitchTiles = stitch;
        _fractalnoise = fractalnoise;
        _octaves = octaves;

        int i;
        for (int k = 0; k < 4; ++k) {
            for (i = 0; i < BSize; ++i) {
                _latticeSelector[i] = i;

                do {
                  _gradient[k][0][i] = static_cast<double>(_random() % (BSize*2) - BSize) / BSize;
                _gradient[k][1][i] = static_cast<double>(_random() % (BSize*2) - BSize) / BSize;
                } while(_gradient[k][0][i] == 0 && _gradient[k][1][i] == 0);
                // normalize gradient
                double s = hypot(_gradient[k][0][i], _gradient[k][1][i]);
                _gradient[k][0][i] /= s;
                _gradient[k][1][i] /= s;
            }
        }
        while (--i) {
            // shuffle lattice selectors
            int j = _random() % BSize;
            std::swap(_latticeSelector[i], _latticeSelector[j]);
        }

        // fill out the remaining part of the gradient
        for (i = 0; i < BSize + 2; ++i)
        {
            _latticeSelector[BSize + i] = _latticeSelector[i];

            for(int k = 0; k < 4; ++k) {
                _gradient[k][0][BSize + i] = _gradient[k][0][i];
                _gradient[k][1][BSize + i] = _gradient[k][1][i];
            }
        }

        // When stitching tiled turbulence, the frequencies must be adjusted
        // so that the tile borders will be continuous.
        if (_stitchTiles) {
            if (_baseFreq[Geom::X] != 0.0)
            {
                double freq = _baseFreq[Geom::X];
                double lo = floor(_tile.width() * freq) / _tile.width();
                double hi = ceil(_tile.width() * freq) / _tile.width();
                _baseFreq[Geom::X] = freq / lo < hi / freq ? lo : hi;
            }
            if (_baseFreq[Geom::Y] != 0.0)
            {
                double freq = _baseFreq[Geom::Y];
                double lo = floor(_tile.height() * freq) / _tile.height();
                double hi = ceil(_tile.height() * freq) / _tile.height();
                _baseFreq[Geom::Y] = freq / lo < hi / freq ? lo : hi;
            }

            _wrapw = _tile.width() * _baseFreq[Geom::X] + 0.5;
            _wraph = _tile.height() * _baseFreq[Geom::Y] + 0.5;
            _wrapx = _tile.left() * _baseFreq[Geom::X] + PerlinOffset + _wrapw;
            _wrapy = _tile.top() * _baseFreq[Geom::Y] + PerlinOffset + _wraph;
        }
        _inited = true;
    }

    G_GNUC_PURE
    guint32 turbulencePixel(Geom::Point const &p) const {
        int wrapx = _wrapx, wrapy = _wrapy, wrapw = _wrapw, wraph = _wraph;

        double pixel[4];
        double x = p[Geom::X] * _baseFreq[Geom::X];
        double y = p[Geom::Y] * _baseFreq[Geom::Y];
        double ratio = 1.0;

        for (int k = 0; k < 4; ++k)
            pixel[k] = 0.0;

        for(int octave = 0; octave < _octaves; ++octave)
        {
            double tx = x + PerlinOffset;
            double bx = floor(tx);
            double rx0 = tx - bx, rx1 = rx0 - 1.0;
            int bx0 = bx, bx1 = bx0 + 1;

            double ty = y + PerlinOffset;
            double by = floor(ty);
            double ry0 = ty - by, ry1 = ry0 - 1.0;
            int by0 = by, by1 = by0 + 1;

            if (_stitchTiles) {
                if (bx0 >= wrapx) bx0 -= wrapw;
                if (bx1 >= wrapx) bx1 -= wrapw;
                if (by0 >= wrapy) by0 -= wraph;
                if (by1 >= wrapy) by1 -= wraph;
            }
            bx0 &= BMask;
            bx1 &= BMask;
            by0 &= BMask;
            by1 &= BMask;

            int i = _latticeSelector[bx0];
            int j = _latticeSelector[bx1];
            int b00 = _latticeSelector[i + by0];
            int b01 = _latticeSelector[i + by1];
            int b10 = _latticeSelector[j + by0];
            int b11 = _latticeSelector[j + by1];

            double sx = _scurve(rx0);
            double sy = _scurve(ry0);

            double result[4];
            // channel numbering: R=0, G=1, B=2, A=3
            for (int k = 0; k < 4; ++k) {
                double a = _lerp(sx, rx0 * _gradient[k][0][b00] + ry0 * _gradient[k][1][b00],
                                     rx1 * _gradient[k][0][b10] + ry0 * _gradient[k][1][b10]);

                double b = _lerp(sx, rx0 * _gradient[k][0][b01] + ry1 * _gradient[k][1][b01],
                                     rx1 * _gradient[k][0][b11] + ry1 * _gradient[k][1][b01]);
                result[k] = _lerp(sy, a, b);
            }

            if (_fractalnoise) {
                for (int k = 0; k < 4; ++k)
                    pixel[k] += result[k] / ratio;
            } else {
                for (int k = 0; k < 4; ++k)
                    pixel[k] += fabs(result[k]) / ratio;
            }

            x *= 2;
            y *= 2;
            ratio *= 2;

            if(_stitchTiles)
            {
                // Update stitch values. Subtracting PerlinOffset before the multiplication and
                // adding it afterward simplifies to subtracting it once.
                wrapw *= 2;
                wraph *= 2;
                wrapx = wrapx*2 - PerlinOffset;
                wrapy = wrapy*2 - PerlinOffset;
            }
        }

        if (_fractalnoise) {
            guint32 r = CLAMP_D_TO_U8((pixel[0]*255.0 + 255.0) / 2);
            guint32 g = CLAMP_D_TO_U8((pixel[1]*255.0 + 255.0) / 2);
            guint32 b = CLAMP_D_TO_U8((pixel[2]*255.0 + 255.0) / 2);
            guint32 a = CLAMP_D_TO_U8((pixel[3]*255.0 + 255.0) / 2);
            r = premul_alpha(r, a);
            g = premul_alpha(g, a);
            b = premul_alpha(b, a);
            ASSEMBLE_ARGB32(pxout, a,r,g,b);
            return pxout;
        } else {
            guint32 r = CLAMP_D_TO_U8(pixel[0]*255.0);
            guint32 g = CLAMP_D_TO_U8(pixel[1]*255.0);
            guint32 b = CLAMP_D_TO_U8(pixel[2]*255.0);
            guint32 a = CLAMP_D_TO_U8(pixel[3]*255.0);
            r = premul_alpha(r, a);
            g = premul_alpha(g, a);
            b = premul_alpha(b, a);
            ASSEMBLE_ARGB32(pxout, a,r,g,b);
            return pxout;
        }
    }
    union VINT_Union
    {
      VINT vec;
      int v[4];
    };

    G_GNUC_PURE
    __m128i turbulencePixelSIMD4(__m128 x, __m128 y) const {

        int wrapx = _wrapx, wrapy = _wrapy, wrapw = _wrapw, wraph = _wraph;

        __m128 pixel[4];

        x *= _mm_set1_ps(_baseFreq[Geom::X]);
        y *= _mm_set1_ps(_baseFreq[Geom::Y]);

        __m128 ratio = _mm_set1_ps(1.0f);

        for (int k = 0; k < 4; ++k)
            pixel[k] = _mm_set1_ps(0);

        VINT bmask = (VINT)_mm_set1_epi32(BMask);
        VINT v4n_1 = (VINT)_mm_set1_epi32(1);
        __m128 v4f_1 = _mm_set1_ps(1.0f);
        __m128 v4f_PerlinOffset = _mm_set1_ps(PerlinOffset);


        for(int octave = 0; octave < _octaves; ++octave)
        {
            __m128 tx = x + v4f_PerlinOffset;
            __m128 bx = _mm_floor_ps(tx);
            __m128 rx0 = tx - bx, rx1 = rx0 - v4f_1;
            VINT bx0 = (VINT)_mm_cvtps_epi32(bx), bx1 = bx0 + v4n_1;

            __m128 ty = y + v4f_PerlinOffset;
            __m128 by = _mm_floor_ps(ty);
            __m128 ry0 = ty - by, ry1 = ry0 - v4f_1;
            VINT by0 = (VINT)_mm_cvtps_epi32(by), by1 = by0 + v4n_1;

            if (_stitchTiles) {
            	VINT wrapx_minus_one = (VINT)_mm_set1_epi32(wrapx - 1),
                     wrapy_minus_one = (VINT)_mm_set1_epi32(wrapy - 1);
            	bx0 = Select(bx0, bx0 - (VINT)_mm_set1_epi32(wrapw), _mm_cmpgt_epi32((__m128i)bx0, (__m128i)wrapx_minus_one));
            	bx1 = Select(bx1, bx1 - (VINT)_mm_set1_epi32(wrapw), _mm_cmpgt_epi32((__m128i)bx1, (__m128i)wrapx_minus_one));
            	by0 = Select(by0, by0 - (VINT)_mm_set1_epi32(wraph), _mm_cmpgt_epi32((__m128i)by0, (__m128i)wrapy_minus_one));
            	by1 = Select(by1, by1 - (VINT)_mm_set1_epi32(wraph), _mm_cmpgt_epi32((__m128i)by1, (__m128i)wrapy_minus_one));
            }

            bx0 &= bmask;
            bx1 &= bmask;
            by0 &= bmask;
            by1 &= bmask;

            VINT i = Gather(_latticeSelector, bx0);
            VINT j = Gather(_latticeSelector, bx1);

            VINT_Union b00, b01, b10, b11;
            b00.vec = Gather(_latticeSelector, i + by0);
            b01.vec =  Gather(_latticeSelector, i + by1);
            b10.vec =  Gather(_latticeSelector, j + by0);
            b11.vec =  Gather(_latticeSelector, j + by1);

            __m128 sx = _scurve(rx0),
                   sy = _scurve(ry0);

            // channel numbering: R=0, G=1, B=2, A=3
            for (int k = 0; k < 4; ++k)
            {
                __m128 a = _lerp(sx, rx0 * GatherFloat(_gradient[k][0], b00.v) + ry0 * GatherFloat(_gradient[k][1], b00.v),
                                     rx1 * GatherFloat(_gradient[k][0], b10.v) + ry0 * GatherFloat(_gradient[k][1], b10.v));

                __m128 b = _lerp(sx, rx0 * GatherFloat(_gradient[k][0], b01.v) + ry1 * GatherFloat(_gradient[k][1], b01.v),
                                     rx1 * GatherFloat(_gradient[k][0], b11.v) + ry1 * GatherFloat(_gradient[k][1], b11.v));
                __m128 result = _lerp(sy, a, b);
                if (_fractalnoise)
                  pixel[k] += result * ratio;
                else
                  pixel[k] += Abs(result) * ratio;
            }

            x = x + x;   //saves from having to load a constant for multiply
            y = y + y;
            ratio *= _mm_set1_ps(0.5f);

            if(_stitchTiles)
            {
                // Update stitch values. Subtracting PerlinOffset before the multiplication and
                // adding it afterward simplifies to subtracting it once.
                wrapw *= 2;
                wraph *= 2;
                wrapx = wrapx*2 - PerlinOffset;
                wrapy = wrapy*2 - PerlinOffset;
            }
        }

        if (_fractalnoise) {
            VINT r = Clamp(_mm_cvtps_epi32(pixel[0] * _mm_set1_ps(127.5f) + _mm_set1_ps(127.5f)));
            VINT g = Clamp(_mm_cvtps_epi32(pixel[1] * _mm_set1_ps(127.5f) + _mm_set1_ps(127.5f)));
            VINT b = Clamp(_mm_cvtps_epi32(pixel[2] * _mm_set1_ps(127.5f) + _mm_set1_ps(127.5f)));
            VINT a = Clamp(_mm_cvtps_epi32(pixel[3] * _mm_set1_ps(127.5f) + _mm_set1_ps(127.5f)));
            r = premul_alpha_SIMD(r, a);
            g = premul_alpha_SIMD(g, a);
            b = premul_alpha_SIMD(b, a);
            return b | (VINT)_mm_slli_si128((__m128i)g, 1) | (VINT)_mm_slli_si128((__m128i)r, 2) | (VINT)_mm_slli_si128((__m128i)a, 3);


        } else {
            VINT r = Clamp(_mm_cvtps_epi32(pixel[0]*_mm_set1_ps(255.0f)));
            VINT g = Clamp(_mm_cvtps_epi32(pixel[1]*_mm_set1_ps(255.0f)));
            VINT b = Clamp(_mm_cvtps_epi32(pixel[2]*_mm_set1_ps(255.0f)));
            VINT a = Clamp(_mm_cvtps_epi32(pixel[3]*_mm_set1_ps(255.0f)));
            r = premul_alpha_SIMD(r, a);
            g = premul_alpha_SIMD(g, a);
            b = premul_alpha_SIMD(b, a);
            return b | (VINT)_mm_slli_si128((__m128i)g, 1) | (VINT)_mm_slli_si128((__m128i)r, 2) | (VINT)_mm_slli_si128((__m128i)a, 3);
        }
    }
#ifdef __AVX2__
    G_GNUC_PURE
       __m256i turbulencePixelSIMD8(__m256 x, __m256 y) const {

            int wrapx = _wrapx, wrapy = _wrapy, wrapw = _wrapw, wraph = _wraph;

            __m256 pixel[4];

            x *= _mm256_set1_ps(_baseFreq[Geom::X]);
            y *= _mm256_set1_ps(_baseFreq[Geom::Y]);

            __m256 ratio = _mm256_set1_ps(1.0f);

            for (int k = 0; k < 4; ++k)
                pixel[k] = _mm256_set1_ps(0);

            VINT8 bmask = _mm256_set1_epi32(BMask);
            VINT8 v4n_1 = _mm256_set1_epi32(1);
            __m256 v4f_1 = _mm256_set1_ps(1.0f);
            __m256 v4f_PerlinOffset = _mm256_set1_ps(PerlinOffset);


            for(int octave = 0; octave < _octaves; ++octave)
            {
                __m256 tx = x + v4f_PerlinOffset;
                __m256 bx = _mm256_floor_ps(tx);
                __m256 rx0 = tx - bx, rx1 = rx0 - v4f_1;
                VINT8 bx0 = _mm256_cvtps_epi32(bx), bx1 = bx0 + v4n_1;

                __m256 ty = y + v4f_PerlinOffset;
                __m256 by = _mm256_floor_ps(ty);
                __m256 ry0 = ty - by, ry1 = ry0 - v4f_1;
                VINT8 by0 = _mm256_cvtps_epi32(by), by1 = by0 + v4n_1;

                if (_stitchTiles) {
                	VINT8 wrapx_minus_one = _mm256_set1_epi32(wrapx - 1),
                	     wrapy_minus_one = _mm256_set1_epi32(wrapy - 1);
                	bx0 = Select256(bx0, bx0 - (VINT8)_mm256_set1_epi32(wrapw), _mm256_cmpgt_epi32(bx0, wrapx_minus_one));
                	bx1 = Select256(bx1, bx1 - (VINT8)_mm256_set1_epi32(wrapw), _mm256_cmpgt_epi32(bx1, wrapx_minus_one));
                	by0 = Select256(by0, by0 - (VINT8)_mm256_set1_epi32(wraph), _mm256_cmpgt_epi32(by0, wrapy_minus_one));
                	by1 = Select256(by1, by1 - (VINT8)_mm256_set1_epi32(wraph), _mm256_cmpgt_epi32(by1, wrapy_minus_one));
                }

                bx0 &= bmask;
                bx1 &= bmask;
                by0 &= bmask;
                by1 &= bmask;

                VINT8 i = Gather256(_latticeSelector, bx0);
                VINT8 j = Gather256(_latticeSelector, bx1);

                VINT8 b00, b01, b10, b11;
                b00 = Gather256(_latticeSelector, i + by0);
                b01 =  Gather256(_latticeSelector, i + by1);
                b10 =  Gather256(_latticeSelector, j + by0);
                b11 =  Gather256(_latticeSelector, j + by1);

                __m256 sx = _scurve256(rx0),
                       sy = _scurve256(ry0);

                // channel numbering: R=0, G=1, B=2, A=3
                for (int k = 0; k < 4; ++k)
                {
                    __m256 a = _lerp256(sx, rx0 * GatherFloat256(_gradient[k][0], b00) + ry0 * GatherFloat256(_gradient[k][1], b00),
                                         rx1 * GatherFloat256(_gradient[k][0], b10) + ry0 * GatherFloat256(_gradient[k][1], b10)),
                           b = _lerp256(sx, rx0 * GatherFloat256(_gradient[k][0], b01) + ry1 * GatherFloat256(_gradient[k][1], b01),
                                         rx1 * GatherFloat256(_gradient[k][0], b11) + ry1 * GatherFloat256(_gradient[k][1], b11)),
                           result = _lerp256(sy, a, b);
                    if (_fractalnoise)
                      pixel[k] += result * ratio;
                    else
                      pixel[k] += Abs256(result) * ratio;
                }

                x = x + x;   //saves from having to load a constant for multiply
                y = y + y;
                ratio *= _mm256_set1_ps(0.5f);

                if(_stitchTiles)
                {
                    // Update stitch values. Subtracting PerlinOffset before the multiplication and
                    // adding it afterward simplifies to subtracting it once.
                    wrapw *= 2;
                    wraph *= 2;
                    wrapx = wrapx*2 - PerlinOffset;
                    wrapy = wrapy*2 - PerlinOffset;
                }
            }

            if (_fractalnoise) {
                VINT8 r = Clamp256(_mm256_cvtps_epi32(pixel[0] * _mm256_set1_ps(127.5f) + _mm256_set1_ps(127.5f)));
                VINT8 g = Clamp256(_mm256_cvtps_epi32(pixel[1] * _mm256_set1_ps(127.5f) + _mm256_set1_ps(127.5f)));
                VINT8 b = Clamp256(_mm256_cvtps_epi32(pixel[2] * _mm256_set1_ps(127.5f) + _mm256_set1_ps(127.5f)));
                VINT8 a = Clamp256(_mm256_cvtps_epi32(pixel[3] * _mm256_set1_ps(127.5f) + _mm256_set1_ps(127.5f)));
                r = premul_alpha_SIMD256(r, a);
                g = premul_alpha_SIMD256(g, a);
                b = premul_alpha_SIMD256(b, a);
                return b | (VINT8)_mm256_slli_epi32(g, 8) | (VINT8)_mm256_slli_epi32(r, 16) | (VINT8)_mm256_slli_epi32(a, 24);


            } else {
                VINT8 r = Clamp256(_mm256_cvtps_epi32(pixel[0]*_mm256_set1_ps(255.0f)));
                VINT8 g = Clamp256(_mm256_cvtps_epi32(pixel[1]*_mm256_set1_ps(255.0f)));
                VINT8 b = Clamp256(_mm256_cvtps_epi32(pixel[2]*_mm256_set1_ps(255.0f)));
                VINT8 a = Clamp256(_mm256_cvtps_epi32(pixel[3]*_mm256_set1_ps(255.0f)));
                r = premul_alpha_SIMD256(r, a);
                g = premul_alpha_SIMD256(g, a);
                b = premul_alpha_SIMD256(b, a);
                return b | (VINT8)_mm256_slli_epi32(g, 8) | (VINT8)_mm256_slli_epi32(r, 16) | (VINT8)_mm256_slli_epi32(a, 24);
            }
        }
#endif
    //G_GNUC_PURE
    /*guint32 turbulencePixel(Geom::Point const &p) const {
        if (!_fractalnoise) {
            guint32 r = CLAMP_D_TO_U8(turbulence(0, p)*255.0);
            guint32 g = CLAMP_D_TO_U8(turbulence(1, p)*255.0);
            guint32 b = CLAMP_D_TO_U8(turbulence(2, p)*255.0);
            guint32 a = CLAMP_D_TO_U8(turbulence(3, p)*255.0);
            r = premul_alpha(r, a);
            g = premul_alpha(g, a);
            b = premul_alpha(b, a);
            ASSEMBLE_ARGB32(pxout, a,r,g,b);
            return pxout;
        } else {
            guint32 r = CLAMP_D_TO_U8((turbulence(0, p)*255.0 + 255.0) / 2);
            guint32 g = CLAMP_D_TO_U8((turbulence(1, p)*255.0 + 255.0) / 2);
            guint32 b = CLAMP_D_TO_U8((turbulence(2, p)*255.0 + 255.0) / 2);
            guint32 a = CLAMP_D_TO_U8((turbulence(3, p)*255.0 + 255.0) / 2);
            r = premul_alpha(r, a);
            g = premul_alpha(g, a);
            b = premul_alpha(b, a);
            ASSEMBLE_ARGB32(pxout, a,r,g,b);
            return pxout;
        }
    }*/

    bool ready() const { return _inited; }
    void dirty() { _inited = false; }

private:
    void _setupSeed(long seed) {
        _seed = seed;
        if (_seed <= 0) _seed = -(_seed % (RAND_m - 1)) + 1;
        if (_seed > RAND_m - 1) _seed = RAND_m - 1;
    }
    long _random() {
        /* Produces results in the range [1, 2**31 - 2].
         * Algorithm is: r = (a * r) mod m
         * where a = 16807 and m = 2**31 - 1 = 2147483647
         * See [Park & Miller], CACM vol. 31 no. 10 p. 1195, Oct. 1988
         * To test: the algorithm should produce the result 1043618065
         * as the 10,000th generated number if the original seed is 1. */
        _seed = RAND_a * (_seed % RAND_q) - RAND_r * (_seed / RAND_q);
        if (_seed <= 0) _seed += RAND_m;
        return _seed;
    }
    static inline double _scurve(double t) {
        return t * t * (3.0 - 2.0*t);
    }
    static inline double _lerp(double t, double a, double b) {
        return a + t * (b-a);
    }
    static inline __m128 _scurve(__m128 t)
    {
    	return t * t * (_mm_set1_ps(3.0f) - _mm_set1_ps(2.0f) * t);
    }
    static inline __m128 _lerp(__m128 t, __m128 a, __m128 b)
    {
    	return a + t * (b - a);
    }
#ifdef __AVX2__
    static inline __m256 _scurve256(__m256 t)
    {
     	return t * t * (_mm256_set1_ps(3.0f) - _mm256_set1_ps(2.0f) * t);
    }
    static inline __m256 _lerp256(__m256 t, __m256 a, __m256 b)
    {
     	return a + t * (b - a);
    }
#endif
    // random number generator constants
    static long const
        RAND_m = 2147483647, // 2**31 - 1
        RAND_a = 16807, // 7**5; primitive root of m
        RAND_q = 127773, // m / a
        RAND_r = 2836; // m % a

    // other constants
    static int const BSize = 0x100;
    static int const BMask = 0xff;

    static double constexpr PerlinOffset = 4096.0;

    Geom::Rect _tile;
    Geom::Point _baseFreq;
    int _latticeSelector[2*BSize + 2];
    float _gradient[4][2][2*BSize + 2];
    long _seed;
    int _octaves;
    bool _stitchTiles;
    int _wrapx;
    int _wrapy;
    int _wrapw;
    int _wraph;
    bool _inited;
    bool _fractalnoise;
};

FilterTurbulence::FilterTurbulence()
    : gen(new TurbulenceGenerator())
    , XbaseFrequency(0)
    , YbaseFrequency(0)
    , numOctaves(1)
    , seed(0)
    , updated(false)
    , fTileWidth(10) //guessed
    , fTileHeight(10) //guessed
    , fTileX(1) //guessed
    , fTileY(1) //guessed
{
    omp_init_lock(&lock);
}

FilterPrimitive * FilterTurbulence::create() {
    return new FilterTurbulence();
}

FilterTurbulence::~FilterTurbulence()
{
    delete gen;
}

void FilterTurbulence::set_baseFrequency(int axis, double freq){
    if (axis==0) XbaseFrequency=freq;
    if (axis==1) YbaseFrequency=freq;
    gen->dirty();
}

void FilterTurbulence::set_numOctaves(int num){
    numOctaves = num;
    gen->dirty();
}

void FilterTurbulence::set_seed(double s){
    seed = s;
    gen->dirty();
}

void FilterTurbulence::set_stitchTiles(bool st){
    stitchTiles = st;
    gen->dirty();
}

void FilterTurbulence::set_type(FilterTurbulenceType t){
    type = t;
    gen->dirty();
}

void FilterTurbulence::set_updated(bool /*u*/)
{
}

struct Turbulence {
    Turbulence(TurbulenceGenerator const &gen, Geom::Affine const &trans, int x0, int y0)
        : _gen(gen)
        , _trans(trans)
        , _x0(x0), _y0(y0)
    {}
    guint32 operator()(int x, int y) {
        Geom::Point point(x + _x0, y + _y0);
        point *= _trans;
        return _gen.turbulencePixel(point);
    }
    __m128i CalculateSIMD4(int x, int y)
    {
      Geom::Point transformed = Geom::Point(x + _x0, y + _y0) * _trans;
      __m128 transformedX = _mm_set1_ps(transformed[Geom::X]),
             transformedY = _mm_set1_ps(transformed[Geom::Y]);
      // calculate the 3 other transformed points (x + 1, x + 2, x + 3) by taking advantage of linearity of transform
      __m128 temp = _mm_set_ps(3, 2, 1, 0);

      transformedX = transformedX + temp * _mm_set1_ps(_trans[0]);
      transformedY = transformedY + temp * _mm_set1_ps(_trans[1]);

      __m128i opt = _gen.turbulencePixelSIMD4(transformedX, transformedY);
      return opt;
      for (int i = 0; i < 4; ++i)
      {
    	  union
    	  {
    	    int my;
    	    unsigned char bgra[4];
    	  } actual;
    	  actual.my = ((int *)&opt)[i];
          union
          {
    		  int my;
    		  unsigned char bgra[4];
          } ref;
          ref.my = _gen.turbulencePixel(Geom::Point(x + i + _x0, y + _y0) * _trans);
    	  if (actual.my != ref.my)
    	  {
    		  printf("z");
    	  }
      }
    }
#ifdef __AVX2__
    __m256i CalculateSIMD8(int x, int y)
        {
          Geom::Point transformed = Geom::Point(x + _x0, y + _y0) * _trans;
          __m256 transformedX = _mm256_set1_ps(transformed[Geom::X]),
                 transformedY = _mm256_set1_ps(transformed[Geom::Y]);
          // calculate the 7 other transformed points (x + 1, x + 2, x + 3) by taking advantage of linearity of transform
          __m256 temp = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);

          transformedX = transformedX + temp * _mm256_set1_ps(_trans[0]);
          transformedY = transformedY + temp * _mm256_set1_ps(_trans[1]);

          __m256i opt = _gen.turbulencePixelSIMD8(transformedX, transformedY);
          return opt;
          for (int i = 0; i < 4; ++i)
          {
        	  union
        	  {
        	    int my;
        	    unsigned char bgra[4];
        	  } actual;
        	  actual.my = ((int *)&opt)[i];
              union
              {
        		  int my;
        		  unsigned char bgra[4];
              } ref;
              ref.my = _gen.turbulencePixel(Geom::Point(x + i + _x0, y + _y0) * _trans);
        	  if (actual.my != ref.my)
        	  {
        		  printf("z");
        	  }
          }
        }
#endif
private:
    TurbulenceGenerator const &_gen;
    Geom::Affine _trans;
    int _x0, _y0;
};

void FilterTurbulence::render_cairo(FilterSlot &slot)
{
    cairo_surface_t *input = slot.getcairo(_input);
    cairo_surface_t *out = ink_cairo_surface_create_same_size(input, CAIRO_CONTENT_COLOR_ALPHA);

    // It is probably possible to render at a device scale greater than one
    // but for the moment rendering at a device scale of one is the easiest.
    // cairo_image_surface_get_width() returns width in pixels but
    // cairo_surface_create_similar() requires width in device units so divide by device scale.
    // We are rendering at a device scale of 1... so divide by device scale again!
    double x_scale = 0;
    double y_scale = 0;
    cairo_surface_get_device_scale(input, &x_scale, &y_scale);
    int width  = ceil(cairo_image_surface_get_width( input)/x_scale/x_scale);
    int height = ceil(cairo_image_surface_get_height(input)/y_scale/y_scale);
    cairo_surface_t *temp = cairo_surface_create_similar (input, CAIRO_CONTENT_COLOR_ALPHA, width, height);
    cairo_surface_set_device_scale( temp, 1, 1 );

    // color_interpolation_filter is determined by CSS value (see spec. Turbulence).
    if( _style ) {
        set_cairo_surface_ci(out, (SPColorInterpolation)_style->color_interpolation_filters.computed );
    }

    omp_set_lock(&lock);
    if (!gen->ready()) {
        Geom::Point ta(fTileX, fTileY);
        Geom::Point tb(fTileX + fTileWidth, fTileY + fTileHeight);
        gen->init(seed, Geom::Rect(ta, tb),
            Geom::Point(XbaseFrequency, YbaseFrequency), stitchTiles,
            type == TURBULENCE_FRACTALNOISE, numOctaves);
    }
    omp_unset_lock(&lock);

    Geom::Affine unit_trans = slot.get_units().get_matrix_primitiveunits2pb().inverse();
    Geom::Rect slot_area = slot.get_slot_area();
    double x0 = slot_area.min()[Geom::X];
    double y0 = slot_area.min()[Geom::Y];
    ink_cairo_surface_synthesize_SIMD(temp, Turbulence(*gen, unit_trans, x0, y0));

    // cairo_surface_write_to_png( temp, "turbulence0.png" );

    cairo_t *ct = cairo_create(out);
    cairo_set_source_surface(ct, temp, 0, 0);
    cairo_paint(ct);
    cairo_destroy(ct);

    cairo_surface_destroy(temp);

    cairo_surface_mark_dirty(out);

    slot.set(_output, out);
    cairo_surface_destroy(out);
}

double FilterTurbulence::complexity(Geom::Affine const &)
{
    return 5.0;
}

} /* namespace Filters */
} /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
