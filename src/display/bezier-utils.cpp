#define __SP_BEZIER_UTILS_C__

/*
 * Bezier interpolation for inkscape drawing code
 *
 * Original code published in:
 *   An Algorithm for Automatically Fitting Digitized Curves
 *   by Philip J. Schneider
 *  "Graphics Gems", Academic Press, 1990
 *
 * Authors:
 *   Philip J. Schneider
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Peter Moulder <pmoulder@mail.csse.monash.edu.au>
 *
 * Copyright (C) 1990 Philip J. Schneider
 * Copyright (C) 2001 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2003,2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_HUGE 1e5
#define noBEZIER_DEBUG

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>
#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif
#include <stdlib.h>
#include "bezier-utils.h"
#include <libnr/nr-point-fns.h>
#include <glib.h>

typedef NR::Point BezierCurve[];

/* Forward declarations */
static void generate_bezier(NR::Point b[], NR::Point const d[], gdouble const uPrime[], unsigned len, NR::Point const &tHat1, NR::Point const &tHat2);
static void reparameterize(NR::Point const d[], unsigned len, double u[], BezierCurve const bezCurve);
static gdouble NewtonRaphsonRootFind(BezierCurve const Q, NR::Point const &P, gdouble u);
static NR::Point bezier_pt(unsigned degree, NR::Point const V[], gdouble t);
static NR::Point sp_darray_left_tangent(NR::Point const d[], unsigned length);
static NR::Point sp_darray_right_tangent(NR::Point const d[], unsigned length);
static NR::Point sp_darray_center_tangent(NR::Point const d[], unsigned center, unsigned length);
static unsigned copy_without_nans_or_adjacent_duplicates(NR::Point const src[], unsigned src_len, NR::Point dest[]);
static void chord_length_parameterize(NR::Point const d[], gdouble u[], unsigned len);
static double compute_max_error(NR::Point const d[], double const u[], unsigned len, BezierCurve const bezCurve, unsigned *splitPoint);
static double compute_error(NR::Point const &d, double const u, BezierCurve const bezCurve);


/*
 *  B0, B1, B2, B3 : Bezier multipliers
 */

#define B0(u) ( ( 1.0 - u )  *  ( 1.0 - u )  *  ( 1.0 - u ) )
#define B1(u) ( 3 * u  *  ( 1.0 - u )  *  ( 1.0 - u ) )
#define B2(u) ( 3 * u * u  *  ( 1.0 - u ) )
#define B3(u) ( u * u * u )

#ifdef BEZIER_DEBUG
# define DOUBLE_ASSERT(x) g_assert( ( (x) > -SP_HUGE ) && ( (x) < SP_HUGE ) )
# define BEZIER_ASSERT(b) do { \
           DOUBLE_ASSERT((b)[0][NR::X]); DOUBLE_ASSERT((b)[0][NR::Y]);  \
           DOUBLE_ASSERT((b)[1][NR::X]); DOUBLE_ASSERT((b)[1][NR::Y]);  \
           DOUBLE_ASSERT((b)[2][NR::X]); DOUBLE_ASSERT((b)[2][NR::Y]);  \
           DOUBLE_ASSERT((b)[3][NR::X]); DOUBLE_ASSERT((b)[3][NR::Y]);  \
         } while(0)
#else
# define DOUBLE_ASSERT(x) do { } while(0)
# define BEZIER_ASSERT(b) do { } while(0)
#endif


/*
 * sp_bezier_fit_cubic
 *
 * Fit a single-segment Bezier curve to a set of digitized points
 *
 * Returns number of segments generated or -1 on error
 */

gint
sp_bezier_fit_cubic(NR::Point *bezier, NR::Point const *data, gint len, gdouble error)
{
    return sp_bezier_fit_cubic_r(bezier, data, len, error, 1);
}

/**
 * Fit a multi-segment Bezier curve to a set of digitized points.
 *
 * Maximum number of generated segments is 2 ^ (max_depth - 1).
 * \a bezier must be large enough for n. segments * 4 elements.
 *
 * return value: number of segments generated, or -1 on error.
 */
gint
sp_bezier_fit_cubic_r(NR::Point *cbezier, NR::Point const *data, gint len, gdouble error, gint max_depth)
{
    g_return_val_if_fail(cbezier != NULL, -1);
    g_return_val_if_fail(data != NULL, -1);
    g_return_val_if_fail(len > 0, -1);
    g_return_val_if_fail(unsigned(max_depth) < 30, -1);

    NR::Point *uniqued_data = g_new(NR::Point, len);
    unsigned uniqued_len = copy_without_nans_or_adjacent_duplicates(data, len, uniqued_data);

    if ( uniqued_len < 2 ) {
        g_free(uniqued_data);
        return 0;
    }

    NR::Point tHat1 = sp_darray_left_tangent(uniqued_data, uniqued_len);
    NR::Point tHat2 = sp_darray_right_tangent(uniqued_data, uniqued_len);

    /* Even our hacked up bezier fitter can fit n points perfectly with n-1 segments. */
    guint32 bezier_npts = (guint32) 4 << ( max_depth - 1 );
    while ( bezier_npts >= ( uniqued_len - 1 ) * 8 ) {
        max_depth--;
        bezier_npts /= 2;
    }
    NR::Point *bezier = g_new(NR::Point, bezier_npts);

    /* call fit-cubic function with recursion */
    gint ret = sp_bezier_fit_cubic_full(bezier, uniqued_data, uniqued_len, tHat1, tHat2, error, max_depth);
    g_free(uniqued_data);
    for (gint i = 0; i < ret * 4; ++i) {
        cbezier[i] = NR::Point(bezier[i]);
    }
    g_free(bezier);
    return ret;
}

static unsigned
copy_without_nans_or_adjacent_duplicates(NR::Point const src[], unsigned src_len, NR::Point dest[])
{
    unsigned si = 0;
    for (;;) {
        if ( si == src_len ) {
            return 0;
        }
        if (!isnan(src[si][NR::X]) &&
            !isnan(src[si][NR::Y])) {
            dest[0] = NR::Point(src[si]);
            ++si;
            break;
        }
    }
    unsigned di = 0;
    for (; si < src_len; ++si) {
        NR::Point const src_pt = NR::Point(src[si]);
        if ( src_pt != dest[di]
             && !isnan(src_pt[NR::X])
             && !isnan(src_pt[NR::Y])) {
            dest[++di] = src_pt;
        }
    }
    unsigned dest_len = di + 1;
    g_assert( dest_len <= src_len );
    return dest_len;
}

gint
sp_bezier_fit_cubic_full(NR::Point *bezier, NR::Point const data[], gint len,
                         NR::Point const &tHat1, NR::Point const &tHat2,
                         double const error, gint max_depth)
{
    int const maxIterations = 4;   /* Max times to try iterating */

    g_return_val_if_fail(bezier != NULL, -1);
    g_return_val_if_fail(data != NULL, -1);
    g_return_val_if_fail(len > 0, -1);
    g_return_val_if_fail(max_depth >= 0, -1);
    g_return_val_if_fail(error >= 0.0, -1);

    if ( len < 2 ) return 0;

    if ( len == 2 ) {
        /* We have 2 points, which can be fitted trivially. */
        bezier[0] = data[0];
        bezier[3] = data[len - 1];
        double const dist = ( L2( data[len - 1]
                                  - data[0] )
                              / 3.0 );
        if (isnan(dist)) {
            /* Numerical problem, fall back to straight line segment. */
            bezier[1] = bezier[0];
            bezier[2] = bezier[3];
        } else {
            bezier[1] = bezier[0] + dist * tHat1;
            bezier[2] = bezier[3] + dist * tHat2;
        }
        BEZIER_ASSERT(bezier);
        return 1;
    }

    /*  Parameterize points, and attempt to fit curve */
    unsigned splitPoint;   /* Point to split point set at. */
    {
        double *u = g_new(double, len);
        chord_length_parameterize(data, u, len);
        if ( u[len - 1] == 0.0 ) {
            /* Zero-length path: every point in data[] is the same. */
            g_free(u);
            return 0;
        }

        generate_bezier(bezier, data, u, len, tHat1, tHat2);
        reparameterize(data, len, u, bezier);

        /* Find max deviation of points to fitted curve. */
        double maxError = compute_max_error(data, u, len, bezier, &splitPoint);

        if ( maxError <= error ) {
            BEZIER_ASSERT(bezier);
            g_free(u);
            return 1;
        }

        /* If error not too large, then try some reparameterization and iteration. */
        if ( maxError <= error * 9.0 ) {
            for (int i = 0; i < maxIterations; i++) {
                generate_bezier(bezier, data, u, len, tHat1, tHat2);
                reparameterize(data, len, u, bezier);
                maxError = compute_max_error(data, u, len, bezier, &splitPoint);
                if ( maxError <= error ) {
                    BEZIER_ASSERT(bezier);
                    g_free(u);
                    return 1;
                }
            }
        }
        g_free(u);
    }

    if ( max_depth > 1 ) {
        /*
         *  Fitting failed -- split at max error point and fit recursively
         */
        max_depth--;

        /* Unit tangent vector at splitPoint. */
        NR::Point tHatCenter = sp_darray_center_tangent(data, splitPoint, len);
        gint const nsegs1 = sp_bezier_fit_cubic_full(bezier, data, splitPoint + 1, tHat1, tHatCenter, error, max_depth);
        if ( nsegs1 < 0 ) {
#ifdef BEZIER_DEBUG
            g_print("fit_cubic[1]: fail on max_depth:%d\n", max_depth);
#endif
            return -1;
        }
        tHatCenter = -tHatCenter;
        gint const nsegs2 = sp_bezier_fit_cubic_full(bezier + nsegs1*4, data + splitPoint, len - splitPoint, tHatCenter, tHat2, error, max_depth);
        if ( nsegs2 < 0 ) {
#ifdef BEZIER_DEBUG
            g_print("fit_cubic[2]: fail on max_depth:%d\n", max_depth);
#endif
            return -1;
        }

#ifdef BEZIER_DEBUG
        g_print("fit_cubic: success[nsegs: %d+%d=%d] on max_depth:%d\n",
                nsegs1, nsegs2, nsegs1 + nsegs2, max_depth + 1);
#endif
        return nsegs1 + nsegs2;
    } else {
        return -1;
    }
}

/*
 *  generate_bezier :
 *  Use least-squares method to find Bezier control points for region.
 *
 */
static void
generate_bezier(NR::Point bezier[],
                NR::Point const data[], gdouble const uPrime[], unsigned len,
                NR::Point const &tHat1, NR::Point const &tHat2)
{
    double C[2][2];   /* Matrix C. */
    double X[2];      /* Matrix X. */
    double det_C0_C1,  /* Determinants of matrices. */
           det_C0_X,
           det_X_C1;

    /* Create the C and X matrices. */
    C[0][0] = 0.0;
    C[0][1] = 0.0;
    C[1][0] = 0.0;
    C[1][1] = 0.0;
    X[0]    = 0.0;
    X[1]    = 0.0;

    /* First and last control points of the Bezier curve are positioned exactly at the first and
       last data points. */
    bezier[0] = data[0];
    bezier[3] = data[len - 1];

    for (unsigned i = 0; i < len; i++) {
        /* Bezier control point coefficients. */
        double const b0 = B0(uPrime[i]);
        double const b1 = B1(uPrime[i]);
        double const b2 = B2(uPrime[i]);
        double const b3 = B3(uPrime[i]);

        /* rhs for eqn */
        NR::Point const a1 = b1 * tHat1;
        NR::Point const a2 = b2 * tHat2;

        C[0][0] += dot(a1, a1);
        C[0][1] += dot(a1, a2);
        C[1][0] = C[0][1];
        C[1][1] += dot(a2, a2);

        /* Additional offset to the data point from the predicted point if we were to set bezier[1]
           to bezier[0] and bezier[2] to bezier[3]. */
        NR::Point const shortfall
            = ( data[i]
                - ( ( b0 + b1 ) * bezier[0] )
                - ( ( b2 + b3 ) * bezier[3] ) );
        X[0] += dot(a1, shortfall);
        X[1] += dot(a2, shortfall);
    }

    /* Compute the determinants of C and X. */
    det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
    det_C0_X  = C[0][0] * X[1]    - C[0][1] * X[0];
    det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];

    /* Finally, derive left & right alpha values. */
    if ( det_C0_C1 == 0.0 ) {
        det_C0_C1 = ( C[0][0] * C[1][1] ) * 10e-12;
    }
    double alpha_l = det_X_C1 / det_C0_C1;
    double alpha_r = det_C0_X / det_C0_C1;

    /* If alpha negative, use the Wu/Barsky heuristic (see text).  (If alpha is 0, you get
       coincident control points that lead to divide by zero in any subsequent
       NewtonRaphsonRootFind() call.) */
    /* TODO: Check whether this special-casing is necessary now that NewtonRaphsonRootFind handles
       non-positive denominator. */
    if ( alpha_l < 1.0e-6 ||
         alpha_r < 1.0e-6   )
    {
        alpha_l = alpha_r = ( L2( data[len - 1]
                                  - data[0] )
                              / 3.0 );
    }

    /* Control points 1 and 2 are positioned an alpha distance out on the tangent vectors, left and
       right, respectively. */
    bezier[1] = alpha_l * tHat1 + bezier[0];
    bezier[2] = alpha_r * tHat2 + bezier[3];

    return;
}

static double lensq(NR::Point const p) {
    return dot(p, p);
}

/**
 * Given set of points and their parameterization, try to find a better assignment of parameter
 * values for the points.
 *
 *  \param d  Array of digitized points.
 *  \param u  Current parameter values.
 *  \param bezCurve  Current fitted curve.
 *  \param len  Number of values in both d and u arrays.
 *              Also the size of the array that is allocated for return.
 */
static void
reparameterize(NR::Point const d[],
               unsigned const len,
               double u[],
               BezierCurve const bezCurve)
{
    g_assert( 2 <= len );

    unsigned const last = len - 1;
    g_assert( bezCurve[0] == d[0] );
    g_assert( bezCurve[3] == d[last] );
    g_assert( u[0] == 0.0 );
    g_assert( u[last] == 1.0 );
    /* Otherwise, consider including 0 and last in the below loop. */

    for (unsigned i = 1; i < last; i++) {
        u[i] = NewtonRaphsonRootFind(bezCurve, d[i], u[i]);
    }
}

/**
 *  Use Newton-Raphson iteration to find better root.
 *  Arguments:
 *      Q : Current fitted curve
 *      P : Digitized point
 *      u : Parameter value for "P"
 *  Return value:
 *      Improved u
 */
static gdouble
NewtonRaphsonRootFind(BezierCurve const Q, NR::Point const &P, gdouble const u)
{
    g_assert( 0.0 <= u );
    g_assert( u <= 1.0 );

    /* Generate control vertices for Q'. */
    NR::Point Q1[3];
    for (unsigned i = 0; i < 3; i++) {
        Q1[i] = 3.0 * ( Q[i+1] - Q[i] );
    }

    /* Generate control vertices for Q''. */
    NR::Point Q2[2];
    for (unsigned i = 0; i < 2; i++) {
        Q2[i] = 2.0 * ( Q1[i+1] - Q1[i] );
    }

    /* Compute Q(u), Q'(u) and Q''(u). */
    NR::Point const Q_u  = bezier_pt(3, Q, u);
    NR::Point const Q1_u = bezier_pt(2, Q1, u);
    NR::Point const Q2_u = bezier_pt(1, Q2, u);

    /* Compute f(u)/f'(u), where f is the derivative wrt u of distsq(u) = 0.5 * the square of the
       distance from P to Q(u).  Here we're using Newton-Raphson to find a stationary point in the
       distsq(u), hopefully corresponding to a local minimum in distsq (and hence a local minimum
       distance from P to Q(u)). */
    NR::Point const diff = Q_u - P;
    double numerator = dot(diff, Q1_u);
    double denominator = dot(Q1_u, Q1_u) + dot(diff, Q2_u);

    double improved_u;
    if ( denominator > 0. ) {
        /* One iteration of Newton-Raphson:
           improved_u = u - f(u)/f'(u) */
        improved_u = u - ( numerator / denominator );
    } else {
        /* Using Newton-Raphson would move in the wrong direction (towards a local maximum rather
           than local minimum), so we move an arbitrary amount in the right direction. */
        if ( numerator > 0. ) {
            improved_u = u * .98 - .01;
        } else if ( numerator < 0. ) {
            /* Deliberately asymmetrical, to reduce the chance of cycling. */
            improved_u = .031 + u * .98;
        } else {
            improved_u = u;
        }
    }

    if (!finite(improved_u)) {
        improved_u = u;
    } else if ( improved_u < 0.0 ) {
        improved_u = 0.0;
    } else if ( improved_u > 1.0 ) {
        improved_u = 1.0;
    }

    /* Ensure that improved_u isn't actually worse. */
    {
        double const diff_lensq = lensq(diff);
        for (double proportion = .125; ; proportion += .125) {
            if ( lensq( bezier_pt(3, Q, improved_u) - P ) > diff_lensq ) {
                if ( proportion > 1.0 ) {
                    //g_warning("found proportion %g", proportion);
                    improved_u = u;
                    break;
                }
                improved_u = ( ( 1 - proportion ) * improved_u  +
                               proportion         * u            );
            } else {
                break;
            }
        }
    }

    DOUBLE_ASSERT(improved_u);
    return improved_u;
}

/** Evaluate a Bezier curve at parameter value \a t.
 * \param degree The degree of the Bezier curve: 3 for cubic, 2 for quadratic etc.
 * \param V The control points for the Bezier curve.  Must have (\a degree+1)
 *    elements.
 * \param t The "parameter" value, specifying whereabouts along the curve to
 *    evaluate.  Typically in the range [0.0, 1.0].
 *
 * Let s = 1 - t.
 * BezierII(1, V) gives (s, t) * V, i.e. t of the way
 * from V[0] to V[1].
 * BezierII(2, V) gives (s**2, 2*s*t, t**2) * V.
 * BezierII(3, V) gives (s**3, 3 s**2 t, 3s t**2, t**3) * V.
 *
 * The derivative of BezierII(i, V) with respect to t
 * is i * BezierII(i-1, V'), where for all j, V'[j] =
 * V[j + 1] - V[j].
 */
static NR::Point
bezier_pt(unsigned const degree, NR::Point const V[], gdouble const t)
{
    /** Pascal's triangle. */
    static int const pascal[4][4] = {{1},
                                     {1, 1},
                                     {1, 2, 1},
                                     {1, 3, 3, 1}};
    g_assert( degree < G_N_ELEMENTS(pascal) );
    gdouble const s = 1.0 - t;

    /* Calculate powers of t and s. */
    double spow[4];
    double tpow[4];
    spow[0] = 1.0; spow[1] = s;
    tpow[0] = 1.0; tpow[1] = t;
    for (unsigned i = 1; i < degree; ++i) {
        spow[i + 1] = spow[i] * s;
        tpow[i + 1] = tpow[i] * t;
    }

    NR::Point ret = spow[degree] * V[0];
    for (unsigned i = 1; i <= degree; ++i) {
        ret += pascal[degree][i] * spow[degree - i] * tpow[i] * V[i];
    }
    return ret;
}

/*
 * ComputeLeftTangent, ComputeRightTangent, ComputeCenterTangent :
 * Approximate unit tangents at endpoints and "center" of digitized curve
 */

/** Estimate the (forward) tangent at point d[first + 0.5].

    Unlike the center and right versions, this calculates the tangent in the way one might expect,
    i.e. wrt increasing index into d.

    Requires: len >= 2 && d[0] != d[1].
**/
static NR::Point
sp_darray_left_tangent(NR::Point const d[], unsigned const len)
{
    g_assert( len >= 2 );
    g_assert( d[0] != d[1] );
    return unit_vector( d[1] - d[0] );
}

/** Estimates the (backward) tangent at d[last - 0.5].

    N.B. The tangent is "backwards", i.e. it is with respect to decreasing index rather than
    increasing index.

    Requires: len >= 2 && d[len - 1] != d[len - 2].
*/
static NR::Point
sp_darray_right_tangent(NR::Point const d[], unsigned const len)
{
    g_assert( len >= 2 );
    unsigned const last = len - 1;
    unsigned const prev = last - 1;
    g_assert( d[last] != d[prev] );
    return unit_vector( d[prev] - d[last] );
}

/** Estimates the (backward) tangent at d[center], by averaging the two segments connected to
    d[center] (and then normalizing the result).

    N.B. The tangent is "backwards", i.e. it is with respect to decreasing index rather than
    increasing index.

    Requires: (0 < center < len - 1) && d is uniqued (at least in the immediate vicinity of
    \a center).
*/
static NR::Point
sp_darray_center_tangent(NR::Point const d[],
                         unsigned const center,
                         unsigned const len)
{
    g_assert( center != 0 );
    g_assert( center < len - 1 );

    NR::Point ret;
    if ( d[center + 1] == d[center - 1] ) {
        /* Rotate 90 degrees in an arbitrary direction. */
        NR::Point const diff = d[center] - d[center - 1];
        ret = rot90(diff);
    } else {
        ret = d[center - 1] - d[center + 1];
    }
    ret.normalize();
    return ret;
}


/**
 *  Assign parameter values to digitized points using relative distances between points.
 *
 *  Parameter array u must have space for \a len items.
 */
static void
chord_length_parameterize(NR::Point const d[], gdouble u[], unsigned const len)
{
    g_return_if_fail( 2 <= len );

    /* First let u[i] equal the distance travelled along the path from d[0] to d[i]. */
    u[0] = 0.0;
    for (unsigned i = 1; i < len; i++) {
        double const dist = L2( d[i] - d[i-1] );
        u[i] = u[i-1] + dist;
    }

    /* Then scale to [0.0 .. 1.0]. */
    gdouble tot_len = u[len - 1];
    g_return_if_fail( tot_len != 0 );
    if (finite(tot_len)) {
        for (unsigned i = 1; i < len; ++i) {
            u[i] /= tot_len;
        }
    } else {
        /* We could do better, but this probably never happens anyway. */
        for (unsigned i = 1; i < len; ++i) {
            u[i] = i / (gdouble) ( len - 1 );
        }
    }

#ifdef BEZIER_DEBUG
    g_assert( u[0] == 0.0 && u[len - 1] == 1.0 );
    for (unsigned i = 1; i < len; i++) {
        g_assert( u[i] >= u[i-1] );
    }
#endif
}




/**
 *  Find the maximum squared distance of digitized points to fitted curve, and (if this maximum
 *  error is non-zero) set \a *splitPoint to the corresponding index.
 *
 *  Requires: 2 <= len.
 */
static gdouble
compute_max_error(NR::Point const d[], double const u[], unsigned const len, BezierCurve const bezCurve,
                  unsigned *splitPoint)
{
    g_assert( 2 <= len );
    unsigned const last = len - 1;
    g_assert( bezCurve[0] == d[0] );
    g_assert( bezCurve[3] == d[last] );
    g_assert( u[0] == 0.0 );
    g_assert( u[last] == 1.0 );
    /* I.e. assert that the error for the first & last points is zero.
       Otherwise we should include those points in the below loop. */

    double maxDistsq = 0.0; /* Maximum error */
    for (unsigned i = 1; i < last; i++) {
        double const distsq = compute_error(d[i], u[i], bezCurve);
        if ( distsq > maxDistsq ) {
            maxDistsq = distsq;
            *splitPoint = i;
        }
    }

    return maxDistsq;
}

static double compute_error(NR::Point const &d, double const u, BezierCurve const bezCurve) {
    NR::Point const P = bezier_pt(3, bezCurve, u);
    NR::Point const diff = P - d;
    return dot(diff, diff);
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
