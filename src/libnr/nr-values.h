#ifndef __NR_VALUES_H__
#define __NR_VALUES_H__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#include <libnr/nr-types.h>

#define NR_EPSILON 1e-18

#define NR_HUGE 1e18
#define NR_HUGE_L (0x7fffffff)
#define NR_HUGE_S (0x7fff)

#ifdef _WIN32
#define NR_EXPORT static
#else
#define NR_EXPORT
#endif

#if defined (__NR_VALUES_C__) || defined (_WIN32)
NR_EXPORT NRMatrix NR_MATRIX_IDENTITY = {{1.0, 0.0, 0.0, 1.0, 0.0, 0.0}};
NR_EXPORT NRRect NR_RECT_EMPTY = {NR_HUGE, NR_HUGE, -NR_HUGE, -NR_HUGE};
NR_EXPORT NRRectL NR_RECT_L_EMPTY = {NR_HUGE_L, NR_HUGE_L, -NR_HUGE_L, -NR_HUGE_L};
NR_EXPORT NRRectS NR_RECT_S_EMPTY = {NR_HUGE_S, NR_HUGE_S, -NR_HUGE_S, -NR_HUGE_S};
#else
extern NRMatrix NR_MATRIX_IDENTITY;
extern NRMatrix NR_MATRIX_IDENTITY;
extern NRRect NR_RECT_EMPTY;
extern NRRectL NR_RECT_L_EMPTY;
extern NRRectS NR_RECT_S_EMPTY;
#endif

#endif
