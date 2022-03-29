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

#include "hsluv.h"

#include <limits>
#include <cmath>
#include <algorithm>

namespace Hsluv {

/* for RGB */
static const Triplet m[3] = {
    {  3.24096994190452134377, -1.53738317757009345794, -0.49861076029300328366 },
    { -0.96924363628087982613,  1.87596750150772066772,  0.04155505740717561247 },
    {  0.05563007969699360846, -0.20397695888897656435,  1.05697151424287856072 }
};

/* for XYZ */
static const Triplet m_inv[3] = {
    {  0.41239079926595948129,  0.35758433938387796373,  0.18048078840183428751 },
    {  0.21263900587151035754,  0.71516867876775592746,  0.07219231536073371500 },
    {  0.01933081871559185069,  0.11919477979462598791,  0.95053215224966058086 }
};

static const double REF_U = 0.19783000664283680764;
static const double REF_V = 0.46831999493879100370;

// CIE LUV constants
static const double KAPPA = 903.29629629629629629630;
static const double EPSILON = 0.00885645167903563082;

// Types
Line::Line() : slope(0), intercept(0) {}
Line::Line(double slope, double intercept) : slope(slope), intercept(intercept) {}
Line::Line (const Line& other) : slope(other.slope), intercept(other.intercept) {}
void Line::operator=(const Line& other)
{
    slope = other.slope;
    intercept = other.intercept;
}

/**
 * Calculate the bounds of the Luv colors in RGB gamut.
 *
 * @param l Lightness. Between 0.0 and 100.0.
 * @return Bounds of Luv colors in RGB gamut.
 */
std::array<Line, 6> getBounds(double l)
{
    std::array<Line, 6> bounds;

    double tl = l + 16.0;
    double sub1 = (tl * tl * tl) / 1560896.0;
    double sub2 = (sub1 > EPSILON ? sub1 : (l / KAPPA));
    int channel;
    int t;

    for(channel = 0; channel < 3; channel++) {
        double m1 = m[channel][0];
        double m2 = m[channel][1];
        double m3 = m[channel][2];

        for (t = 0; t < 2; t++) {
            double top1 = (284517.0 * m1 - 94839.0 * m3) * sub2;
            double top2 = (838422.0 * m3 + 769860.0 * m2 + 731718.0 * m1) * l * sub2 -  769860.0 * t * l;
            double bottom = (632260.0 * m3 - 126452.0 * m2) * sub2 + 126452.0 * t;

            bounds[channel * 2 + t].slope = top1 / bottom;
            bounds[channel * 2 + t].intercept = top2 / bottom;
        }
    }

    return bounds;
}

/**
 * Calculate the length of a ray at a given angle until it intersects with the
 * passed in line.
 *
 * @param theta The angle of the ray.
 * @param line The line to test.
 * @return The length of the ray.
 */
static double rayLengthUntilIntersect(double theta, const Line &line)
{
    return line.intercept / (std::sin(theta) - line.slope * std::cos(theta));
}

/**
 * Calculate the maximum in gamut chromaticity for the given luminance and hue.
 *
 * @param l Luminance.
 * @param h Hue.
 * @return The maximum chromaticity.
 */
static double maxChromaForLh(double l, double h)
{
    double min_len = std::numeric_limits<double>::max();
    double hrad = h * 0.01745329251994329577; /* (2 * pi / 360) */
    std::array<Line, 6> bounds = getBounds(l);
    int i;

    for (i = 0; i < 6; i++) {
        double len = rayLengthUntilIntersect(hrad, bounds[i]);

        if (len >= 0  &&  len < min_len)
            min_len = len;
    }

    return min_len;
}

/**
 * Calculate the dot product of the given arrays.
 *
 * @param t1 The first array.
 * @param t2 The second array.
 * @return The resulting dot product.
 */
static double dotProduct(const Triplet &t1, const Triplet &t2)
{
    return (t1[0] * t2[0] + t1[1] * t2[1] + t1[2] * t2[2]);
}

/**
 * Convenience function used for RGB conversions.
 *
 * @param c Value.
 * @return RGB color component.
 */
static double fromLinear(double c)
{
    if (c <= 0.0031308)
        return 12.92 * c;
    else
        return 1.055 * std::pow(c, 1.0 / 2.4) - 0.055;
}

/**
 * Convenience function used for RGB conversions.
 *
 * @param c Value.
 * @return XYZ color component.
 */
static double toLinear(double c)
{
    if (c > 0.04045)
        return std::pow((c + 0.055) / 1.055, 2.4);
    else
        return c / 12.92;
}

/**
 * @overload
 * @param t RGB color components.
 * @return XYZ color components.
 */
static Triplet toLinear(const Triplet &t)
{
    return {
        toLinear(t[0]),
        toLinear(t[1]),
        toLinear(t[2])
    };
}

/**
 * Convert a color from the the XYZ colorspace to the RGB colorspace.
 *
 * @param in_out[in,out] The XYZ color converted to a RGB color.
 */
static void xyz2rgb(Triplet *in_out)
{
    double r = fromLinear(dotProduct(m[0], *in_out));
    double g = fromLinear(dotProduct(m[1], *in_out));
    double b = fromLinear(dotProduct(m[2], *in_out));

    (*in_out)[0] = r;
    (*in_out)[1] = g;
    (*in_out)[2] = b;
}

/**
 * Convert a color from the the RGB colorspace to the XYZ colorspace.
 *
 * @param in_out[in,out] The RGB color converted to a XYZ color.
 */
static void rgb2xyz(Triplet *in_out)
{
    Triplet rgbl = toLinear(*in_out);

    double x = dotProduct(m_inv[0], rgbl);
    double y = dotProduct(m_inv[1], rgbl);
    double z = dotProduct(m_inv[2], rgbl);

    (*in_out)[0] = x;
    (*in_out)[1] = y;
    (*in_out)[2] = z;
}

/**
 * Utility function used to convert from the XYZ colorspace to CIELuv.
 * https://en.wikipedia.org/wiki/CIELUV
 *
 * @param y Y component of the XYZ color.
 * @return Luminance component of Luv color.
 */
static double y2l(double y)
{
    if (y <= EPSILON)
        return y * KAPPA;
    else
        return 116.0 * std::cbrt(y) - 16.0;
}

/**
 * Utility function used to convert from CIELuv colorspace to XYZ.
 *
 * @param l Luminance component of Luv color.
 * @return Y component of the XYZ color.
 */
static double l2y(double l)
{
    if (l <= 8.0) {
        return l / KAPPA;
    }
    else {
        double x = (l + 16.0) / 116.0;
        return (x * x * x);
    }
}

/**
 * Convert a color from the the XYZ colorspace to the Luv colorspace.
 *
 * @param in_out[in,out] The XYZ color converted to a Luv color.
 */
static void xyz2luv(Triplet* in_out)
{
    double var_u = (4.0 * (*in_out)[0]) / ((*in_out)[0] + (15.0 * (*in_out)[1]) + (3.0 * (*in_out)[2]));
    double var_v = (9.0 * (*in_out)[1]) / ((*in_out)[0] + (15.0 * (*in_out)[1]) + (3.0 * (*in_out)[2]));
    double l = y2l((*in_out)[1]);
    double u = 13.0 * l * (var_u - REF_U);
    double v = 13.0 * l * (var_v - REF_V);

    (*in_out)[0] = l;
    if (l < 0.00000001) {
        (*in_out)[1] = 0.0;
        (*in_out)[2] = 0.0;
    }
    else {
        (*in_out)[1] = u;
        (*in_out)[2] = v;
    }
}

/**
 * Convert a color from the the Luv colorspace to the XYZ colorspace.
 *
 * @param in_out[in,out] The Luv color converted to a XYZ color.
 */
static void luv2xyz(Triplet* in_out)
{
    if((*in_out)[0] <= 0.00000001) {
        /* Black will create a divide-by-zero error. */
        (*in_out)[0] = 0.0;
        (*in_out)[1] = 0.0;
        (*in_out)[2] = 0.0;
        return;
    }

    double var_u = (*in_out)[1] / (13.0 * (*in_out)[0]) + REF_U;
    double var_v = (*in_out)[2] / (13.0 * (*in_out)[0]) + REF_V;
    double y = l2y((*in_out)[0]);
    double x = -(9.0 * y * var_u) / ((var_u - 4.0) * var_v - var_u * var_v);
    double z = (9.0 * y - (15.0 * var_v * y) - (var_v * x)) / (3.0 * var_v);

    (*in_out)[0] = x;
    (*in_out)[1] = y;
    (*in_out)[2] = z;
}

/**
 * Convert a color from the the Luv colorspace to the LCH colorspace.
 *
 * @param in_out[in,out] The Luv color converted to a LCH color.
 */
static void luv2lch(Triplet* in_out)
{
    double l = (*in_out)[0];
    double u = (*in_out)[1];
    double v = (*in_out)[2];
    double h;
    double c = std::sqrt(u * u + v * v);

    /* Grays: disambiguate hue */
    if(c < 0.00000001) {
        h = 0;
    } else {
        h = std::atan2(v, u) * 57.29577951308232087680;  /* (180 / pi) */
        if(h < 0.0)
            h += 360.0;
    }

    (*in_out)[0] = l;
    (*in_out)[1] = c;
    (*in_out)[2] = h;
}

/**
 * Convert a color from the the LCH colorspace to the Luv colorspace.
 *
 * @param in_out[in,out] The LCH color converted to a Luv color.
 */
static void lch2luv(Triplet* in_out)
{
    double hrad = (*in_out)[2] * 0.01745329251994329577;  /* (pi / 180.0) */
    double u = std::cos(hrad) * (*in_out)[1];
    double v = std::sin(hrad) * (*in_out)[1];

    (*in_out)[1] = u;
    (*in_out)[2] = v;
}

/**
 * Convert a color from the the HSLuv colorspace to the LCH colorspace.
 *
 * @param in_out[in,out] The HSLuv color converted to a LCH color.
 */
static void hsluv2lch(Triplet* in_out)
{
    double h = (*in_out)[0];
    double s = (*in_out)[1];
    double l = (*in_out)[2];
    double c;

    /* White and black: disambiguate chroma */
    if(l > 99.9999999 || l < 0.00000001)
        c = 0.0;
    else
        c = maxChromaForLh(l, h) / 100.0 * s;

    /* Grays: disambiguate hue */
    if (s < 0.00000001)
        h = 0.0;

    (*in_out)[0] = l;
    (*in_out)[1] = c;
    (*in_out)[2] = h;
}

/**
 * Convert a color from the the LCH colorspace to the HSLuv colorspace.
 *
 * @param in_out[in,out] The LCH color converted to a HSLuv color.
 */
static void lch2hsluv(Triplet* in_out)
{
    double l = (*in_out)[0];
    double c = (*in_out)[1];
    double h = (*in_out)[2];
    double s;

    /* White and black: disambiguate saturation */
    if(l > 99.9999999 || l < 0.00000001)
        s = 0.0;
    else
        s = c / maxChromaForLh(l, h) * 100.0;

    /* Grays: disambiguate hue */
    if (c < 0.00000001)
        h = 0.0;

    (*in_out)[0] = h;
    (*in_out)[1] = s;
    (*in_out)[2] = l;
}

// Interface functions
void luv_to_rgb(double l, double u, double v, double *pr, double *pg, double *pb)
{
    Triplet tmp {l, u, v};

    luv2xyz(&tmp);
    xyz2rgb(&tmp);

    *pr = std::clamp(tmp[0], 0.0, 1.0);
    *pg = std::clamp(tmp[1], 0.0, 1.0);
    *pb = std::clamp(tmp[2], 0.0, 1.0);
}

void hsluv_to_luv(double h, double s, double l, double *pl, double *pu, double *pv)
{
    Triplet tmp {h, s, l};

    hsluv2lch(&tmp);
    lch2luv(&tmp);

    *pl = tmp[0];
    *pu = tmp[1];
    *pv = tmp[2];
}

void luv_to_hsluv(double l, double u, double v, double *ph, double *ps, double *pl)
{
    Triplet tmp {l, u, v};

    luv2lch(&tmp);
    lch2hsluv(&tmp);

    *ph = tmp[0];
    *ps = tmp[1];
    *pl = tmp[2];
}

void rgb_to_hsluv(double r, double g, double b, double *ph, double *ps, double *pl)
{
    Triplet tmp {r, g, b};

    rgb2xyz(&tmp);
    xyz2luv(&tmp);
    luv2lch(&tmp);
    lch2hsluv(&tmp);

    *ph = tmp[0];
    *ps = tmp[1];
    *pl = tmp[2];
}

void hsluv_to_rgb(double h, double s, double l, double *pr, double *pg, double *pb)
{
    Triplet tmp {h, s, l};

    hsluv2lch(&tmp);
    lch2luv(&tmp);
    luv2xyz(&tmp);
    xyz2rgb(&tmp);

    *pr = std::clamp(tmp[0], 0.0, 1.0);
    *pg = std::clamp(tmp[1], 0.0, 1.0);
    *pb = std::clamp(tmp[2], 0.0, 1.0);
}

} // namespace Hsluv
