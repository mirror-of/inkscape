// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * HSLuv-C: Human-friendly HSL
 *
 * Authors:
 *   2015 Alexei Boronine (original idea, JavaScript implementation)
 *   2015 Roger Tallada (Obj-C implementation)
 *   2017 Martin Mitas (C implementation, based on Obj-C implementation)
 *   2021 Massinissa Derriche (C++ implementation for Inkscape, based on C implementation)
 *
 * Copyright (C) 2021 Authors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef SEEN_HSLUV_H
#define SEEN_HSLUV_H

#include <array>

namespace Hsluv {

// Types
using Triplet = std::array<double, 3>;
class Line {
public:
    Line ();
    Line (double slope, double intercept);
    Line (const Line& other);
    void operator=(const Line& other);
    double slope;
    double intercept;
};

// Functions
/**
 * Return the bounds of the Luv colors in RGB gamut.
 *
 * @param l Lightness. Between 0.0 and 100.0.
 * @return Bounds of Luv colors in RGB gamut.
 */
std::array<Line, 6> getBounds(double l);

/**
 * Convert Luv to RGB.
 *
 * @param l Luminance. Between 0.0 and 100.0.
 * @param u U coordinate.
 * @param v V coordinate.
 * @param[out] pr Red. Between 0.0 and 1.0.
 * @param[out] pg Green. Between 0.0 and 1.0.
 * @param[out] pb Blue. Between 0.0 and 1.0.
 */
void luv_to_rgb(double l, double u, double v, double *pr, double *pg, double *pb);

/**
 * Convert HSLuv to Luv.
 *
 * @param h Hue. Between 0.0 and 360.0.
 * @param s Saturation. Between 0.0 and 100.0.
 * @param l Lightness. Between 0.0 and 100.0.
 * @param[out] pl Luminance. Between 0.0 and 100.0.
 * @param[out] pu U coordinate.
 * @param[out] pv V coordinate.
 */
void hsluv_to_luv(double h, double s, double l, double *pl, double *pu, double *pv);

/**
 * Convert Luv to HSLuv.
 *
 * @param l Luminance. Between 0.0 and 100.0.
 * @param u U coordinate.
 * @param v V coordinate.
 * @param[out] ph Hue. Between 0.0 and 360.0.
 * @param[out] ps Saturation. Between 0.0 and 100.0.
 * @param[out] pl Lightness. Between 0.0 and 100.0.
 */
void luv_to_hsluv(double l, double u, double v, double *ph, double *ps, double *pl);

/**
 * Convert RGB to HSLuv.
 *
 * @param r Red. Between 0.0 and 1.0.
 * @param g Green. Between 0.0 and 1.0.
 * @param b Blue. Between 0.0 and 1.0.
 * @param[out] ph Hue. Between 0.0 and 360.0.
 * @param[out] ps Saturation. Between 0.0 and 100.0.
 * @param[out] pl Lightness. Between 0.0 and 100.0.
 */
void rgb_to_hsluv(double r, double g, double b, double *ph, double *ps, double *pl);

/**
 * Convert HSLuv to RGB.
 *
 * @param h Hue. Between 0.0 and 360.0.
 * @param s Saturation. Between 0.0 and 100.0.
 * @param l Lightness. Between 0.0 and 100.0.
 * @param[out] pr Red. Between 0.0 and 1.0.
 * @param[out] pg Green. Between 0.0 and 1.0.
 * @param[out] pb Blue. Between 0.0 and 1.0.
 */
void hsluv_to_rgb(double h, double s, double l, double *pr, double *pg, double *pb);

} // namespace Hsluv

#endif  // SEEN_HSLUV_H
