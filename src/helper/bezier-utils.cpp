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
 *
 * Copyright (C) 1990 Philip J. Schneider
 * Copyright (C) 2001 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_HUGE 1e5
#define noBEZIER_DEBUG

#include <math.h>
#include <stdlib.h>
#include "bezier-utils.h"

typedef NRPoint * BezierCurve;

/* Forward declarations */
static void GenerateBezier (NRPoint *b, NRPoint const *d, gdouble const *uPrime, gint len, NRPoint const *tHat1, NRPoint const *tHat2);
static gdouble * Reparameterize(NRPoint const *d, unsigned len, gdouble const *u, BezierCurve bezCurve);
static gdouble NewtonRaphsonRootFind(BezierCurve Q, NRPoint const &P, gdouble u);
static void BezierII (gint degree, NRPoint * V, gdouble t, NRPoint *result);

/*
 *  B0, B1, B2, B3 : Bezier multipliers
 */

#define B0(u) ((1.0 - u) * (1.0 - u) * (1.0 - u))
#define B1(u) (3 * u * (1.0 - u) * (1.0 - u))
#define B2(u) (3 * u * u * (1.0 - u))
#define B3(u) (u * u * u)

static void ChordLengthParameterize(const NRPoint *d, gdouble *u, gint len);
static gdouble ComputeMaxError (const NRPoint *d, gdouble *u, gint len, const BezierCurve bezCurve, gint *splitPoint);

/* Vector operations */

static void sp_vector_add (const NRPoint *a, const NRPoint *b, NRPoint *result);
static void sp_vector_sub (const NRPoint *a, const NRPoint *b, NRPoint *result);
static void sp_vector_scale (const NRPoint *v, gdouble s, NRPoint *result);
static void sp_vector_normalize (NRPoint *v);
static void sp_vector_negate (NRPoint *v);

#define V2Dot(a,b) ((a)->x * (b)->x + (a)->y * (b)->y)

#ifdef BEZIER_DEBUG
#define DOUBLE_ASSERT(x) (g_assert (((x) > -SP_HUGE) && ((x) < SP_HUGE)))
#define BEZIER_ASSERT(b) { \
	DOUBLE_ASSERT(b[0].x); DOUBLE_ASSERT(b[0].y); \
	DOUBLE_ASSERT(b[1].x); DOUBLE_ASSERT(b[1].y); \
	DOUBLE_ASSERT(b[2].x); DOUBLE_ASSERT(b[2].y); \
	DOUBLE_ASSERT(b[3].x); DOUBLE_ASSERT(b[3].y); \
	}
#else
#define DOUBLE_ASSERT(x)
#define BEZIER_ASSERT(b)
#endif

#define MAXPOINTS	1000		/* The most points you can have */

/*
 * sp_bezier_fit_cubic
 *
 * Fit a single-segment Bezier curve to a set of digitized points
 *
 * Returns number of segments generated or -1 on error
 */

gint
sp_bezier_fit_cubic (NRPoint *bezier, const NRPoint *data, gint len, gdouble error)
{
	return sp_bezier_fit_cubic_r (bezier, data, len, error, 1);
}

/*
 *  sp_bezier_fit_cubic_r
 *
 * Fit a multi-segment Bezier curve to a set of digitized points
 *
 * Maximum number of generated segments is 2 ^ (max_depth - 1)
 *
 *    return value:
 *      block size which filled bezier paths by fit-cubic fuction
 *        where 1 block size = 4 * sizeof(NRPoint)
 *      -1 if failed.
 */
gint
sp_bezier_fit_cubic_r (NRPoint *bezier, const NRPoint *data, gint len, gdouble error, gint max_depth)
{
	NRPoint    tHat1;
	NRPoint    tHat2;
	
	g_return_val_if_fail (bezier != NULL, -1);
	g_return_val_if_fail (data != NULL, -1);
	g_return_val_if_fail (len > 0, -1);
	g_return_val_if_fail (max_depth >= 0, -1);

	if (len < 2) return 0;

	sp_darray_left_tangent (data, 0, len, &tHat1);
	sp_darray_right_tangent (data, len - 1, len, &tHat2);
	
	/* call fit-cubic function with recursion */
	return sp_bezier_fit_cubic_full (bezier, data, len, &tHat1, &tHat2, error, max_depth);
}

gint
sp_bezier_fit_cubic_full (NRPoint *bezier, const NRPoint *data, gint len,
			  NRPoint const *tHat1, NRPoint const *tHat2, gdouble error, gint max_depth)
{
	double *u;		/* Parameter values for point */
	double *u_alloca;	/* Just for memory management */
	double *uPrime;		/* Improved parameter values */
	double maxError;	/* Maximum fitting error */
	int splitPoint;		/* Point to split point set at */
	double iterationError;  /* Error below which you try iterating (squared) */
	int maxIterations = 4;	/* Max times to try iterating */
	
	int i;
	
	g_return_val_if_fail (bezier != NULL, -1);
	g_return_val_if_fail (data != NULL, -1);
	g_return_val_if_fail (len > 0, -1);
	g_return_val_if_fail (tHat1 != NULL, -1);
	g_return_val_if_fail (tHat2 != NULL, -1);
	g_return_val_if_fail (max_depth >= 0, -1);

	if (len < 2) return 0;

	iterationError = error * error;
	
	if (len == 2) {
		double dist;
		/* We have 2 points, so just draw straight segment */
		dist = hypot (data[len - 1].x - data[0].x, data[len - 1].y - data[0].y) / 3.0;
		bezier[0] = data[0];
		bezier[3] = data[len - 1];
		bezier[1].x = tHat1->x * dist + bezier[0].x;
		bezier[1].y = tHat1->y * dist + bezier[0].y;
		bezier[2].x = tHat2->x * dist + bezier[3].x;
		bezier[2].y = tHat2->y * dist + bezier[3].y;
		BEZIER_ASSERT (bezier);
		return 1;
	}
	
	/*  Parameterize points, and attempt to fit curve */
	u = (double*)alloca (len * sizeof (gdouble));
	ChordLengthParameterize (data, u, len);
	if (u[len - 1] == 0.0) {
		/* Zero-length path: every point in data[] is the same. */
		return 0;
	}
	GenerateBezier (bezier, data, u, len, tHat1, tHat2);
	
	/*  Find max deviation of points to fitted curve */
	maxError = ComputeMaxError (data, u, len, bezier, &splitPoint);
	
	if (maxError < error) {
		BEZIER_ASSERT (bezier);
		return 1;
	}

	/*  If error not too large, try some reparameterization  */
	/*  and iteration */
	u_alloca = u;
	if (maxError < iterationError) {
		for (i = 0; i < maxIterations; i++) {
			uPrime = Reparameterize(data, len, u, bezier);
			GenerateBezier (bezier, data, uPrime, len, tHat1, tHat2);
			maxError = ComputeMaxError(data, uPrime, len, bezier, &splitPoint);
			if (u != u_alloca)
				g_free(u);
			if (maxError < error) {
				BEZIER_ASSERT (bezier);
				g_free (uPrime);
				return 1;
			}
			u = uPrime;
		}
	}
	if (u != u_alloca)
		g_free(u);
	
	if (max_depth > 1)
	{
		/*
		 *  Fitting failed -- split at max error point and fit recursively
		 */
		NRPoint	tHatCenter; /* Unit tangent vector at splitPoint */
		gint  depth1, depth2;
		
		max_depth--;
		
		sp_darray_center_tangent (data, splitPoint, &tHatCenter);
		depth1 = sp_bezier_fit_cubic_full (bezier, data, splitPoint + 1, tHat1, &tHatCenter, error, max_depth);
		if (depth1 == -1)
		{
#ifdef BEZIER_DEBUG
			g_print ("fit_cubic[1]: fail on max_depth:%d\n", max_depth);
#endif
			return -1;
		}
		sp_vector_negate(&tHatCenter);
		depth2 = sp_bezier_fit_cubic_full (bezier + depth1*4, data + splitPoint, len - splitPoint, &tHatCenter, tHat2, error, max_depth);
		if (depth2 == -1)
		{
#ifdef BEZIER_DEBUG
			g_print ("fit_cubic[2]: fail on max_depth:%d\n", max_depth);
#endif
			return -1;
		}
		
#ifdef BEZIER_DEBUG
		g_print("fit_cubic: success[depth: %d+%d=%d] on max_depth:%d\n",
			depth1, depth2, depth1+depth2, max_depth+1);
#endif
		return depth1 + depth2;
	}
	else
		return -1;
}

/*
 *  GenerateBezier :
 *  Use least-squares method to find Bezier control points for region.
 *
 */
static void
GenerateBezier (NRPoint *bezier, NRPoint const *data, gdouble const *uPrime, gint len, NRPoint const *tHat1, NRPoint const *tHat2)
{
	int 	i;
	NRPoint 	A[MAXPOINTS][2]; /* Precomputed rhs for eqn	*/
	double 	C[2][2];	/* Matrix C		*/
	double 	X[2];		/* Matrix X			*/
	double 	det_C0_C1,	/* Determinants of matrices	*/
    	   	det_C0_X,
		det_X_C1;
	double 	alpha_l,	/* Alpha values, left and right	*/
    	   	alpha_r;
	NRPoint 	tmp;	/* Utility variable		*/

	/* Compute the A's	*/
	for (i = 0; i < len; i++) {
		sp_vector_scale (tHat1, B1 (uPrime[i]), &A[i][0]);
		sp_vector_scale (tHat2, B2 (uPrime[i]), &A[i][1]);
	}
	
	/* Create the C and X matrices	*/
	C[0][0] = 0.0;
	C[0][1] = 0.0;
	C[1][0] = 0.0;
	C[1][1] = 0.0;
	X[0]    = 0.0;
	X[1]    = 0.0;
	
	for (i = 0; i < len; i++) {
		NRPoint tmp1, tmp2, tmp3, tmp4;
		C[0][0] += V2Dot(&A[i][0], &A[i][0]);
		C[0][1] += V2Dot(&A[i][0], &A[i][1]);
		C[1][0] = C[0][1];
		C[1][1] += V2Dot(&A[i][1], &A[i][1]);
		
                sp_vector_scale (&data[len - 1], B2(uPrime[i]), &tmp1);
                sp_vector_scale (&data[len - 1], B3(uPrime[i]), &tmp2);
                sp_vector_add (&tmp1, &tmp2, &tmp3);
                sp_vector_scale (&data[0], B0(uPrime[i]), &tmp1);
                sp_vector_scale (&data[0], B1(uPrime[i]), &tmp2);
                sp_vector_add (&tmp2, &tmp3, &tmp4);
                sp_vector_add (&tmp1, &tmp4, &tmp2);
		sp_vector_sub (&data[i], &tmp2, &tmp);
		
		
		X[0] += V2Dot(&A[i][0], &tmp);
		X[1] += V2Dot(&A[i][1], &tmp);
	}
	
	/* Compute the determinants of C and X	*/
	det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
	det_C0_X  = C[0][0] * X[1]    - C[0][1] * X[0];
	det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];
	
	/* Finally, derive alpha values	*/
	if (det_C0_C1 == 0.0) {
		det_C0_C1 = (C[0][0] * C[1][1]) * 10e-12;
	}
	alpha_l = det_X_C1 / det_C0_C1;
	alpha_r = det_C0_X / det_C0_C1;
	
	/*  First and last control points of the Bezier curve are */
	/*  positioned exactly at the first and last data points */
	bezier[0] = data[0];
	bezier[3] = data[len - 1];

	/*  If alpha negative, use the Wu/Barsky heuristic (see text) */
	/* (if alpha is 0, you get coincident control points that lead to
	 * divide by zero in any subsequent NewtonRaphsonRootFind() call. */
	if (alpha_l < 1.0e-6 || alpha_r < 1.0e-6) {
		gdouble dist;
		
		dist = hypot (data[len - 1].x - data[0].x, data[len - 1].y - data[0].y) / 3.0;
		
		bezier[1].x = tHat1->x * dist + bezier[0].x;
		bezier[1].y = tHat1->y * dist + bezier[0].y;
		bezier[2].x = tHat2->x * dist + bezier[3].x;
		bezier[2].y = tHat2->y * dist + bezier[3].y;

		return;
	}
	
	/*  Control points 1 and 2 are positioned an alpha distance out */
	/*  on the tangent vectors, left and right, respectively */
	bezier[1].x = tHat1->x * alpha_l + bezier[0].x;
	bezier[1].y = tHat1->y * alpha_l + bezier[0].y;
	bezier[2].x = tHat2->x * alpha_r + bezier[3].x;
	bezier[2].y = tHat2->y * alpha_r + bezier[3].y;

	return;
}

/**
 *  Reparameterize:
 *	Given set of points and their parameterization, try to find
 *   a better parameterization.
 *
 *  \param d	Array of digitized points.
 *  \param first,last	Indices defining region.  Inclusive.
 *  \param u	Current parameter values.
 *  \param bezCurve	Current fitted curve.
 */
static gdouble *
Reparameterize(NRPoint const  *d,
	       unsigned        len,
	       gdouble const  *u,
	       BezierCurve     bezCurve)
{
	/*  New parameter values. */
	gdouble	*uPrime = g_new (gdouble, len);

	for (unsigned i = 0; i < len; i++) {
		uPrime[i] = NewtonRaphsonRootFind(bezCurve, d[i], u[i]);
	}
	return (uPrime);
}

/*
 *  NewtonRaphsonRootFind :
 *	Use Newton-Raphson iteration to find better root.
 *  Arguments:
 *      Q : Current fitted curve
 *      P : Digitized point
 *      u : Parameter value for "P"
 *  Return value:
 *      Improved u
 */
static gdouble
NewtonRaphsonRootFind(BezierCurve Q, NRPoint const &P, gdouble u)
{
	double 		numerator, denominator;
	NRPoint 		Q1[3], Q2[2];	/*  Q' and Q''			*/
	NRPoint		Q_u, Q1_u, Q2_u; /*u evaluated at Q, Q', & Q''	*/
	double 		uPrime;		/*  Improved u			*/
	int 		i;

	DOUBLE_ASSERT (u);
	
	/* Compute Q(u)	*/
	BezierII(3, Q, u, &Q_u);
	
	/* Generate control vertices for Q'	*/
	for (i = 0; i <= 2; i++) {
		Q1[i].x = (Q[i+1].x - Q[i].x) * 3.0;
		Q1[i].y = (Q[i+1].y - Q[i].y) * 3.0;
	}
	
	/* Generate control vertices for Q'' */
	for (i = 0; i <= 1; i++) {
		Q2[i].x = (Q1[i+1].x - Q1[i].x) * 2.0;
		Q2[i].y = (Q1[i+1].y - Q1[i].y) * 2.0;
	}
	
	/* Compute Q'(u) and Q''(u)	*/
	BezierII(2, Q1, u, &Q1_u);
	BezierII(1, Q2, u, &Q2_u);
	
	/* Compute f(u)/f'(u) */
	numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
	denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
		(Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
	
	/* u = u - f(u)/f'(u) */
	uPrime = u - (numerator/denominator);
	if (!isfinite (uPrime)) {
		uPrime = u;
	}
	/* TODO: Check that uPrime is actually better. */
	DOUBLE_ASSERT (uPrime);
	return (uPrime);
}

/*
 *  Bezier :
 *  	Evaluate a Bezier curve at a particular parameter value
 * 
 */
static void
BezierII (gint degree, NRPoint * V, gdouble t, NRPoint *Q)
{
	/* NRPoint 	Q;	        Point on curve at parameter t	*/
	int 	i, j;		
	NRPoint 	*Vtemp;	/* Local copy of control points		*/
	
	/* Copy array	*/
	Vtemp = g_new (NRPoint, degree + 1);
	
	for (i = 0; i <= degree; i++) {
		Vtemp[i] = V[i];
	}
	
	/* Triangle computation	*/
	for (i = 1; i <= degree; i++) {	
		for (j = 0; j <= degree-i; j++) {
			Vtemp[j].x = (1.0 - t) * Vtemp[j].x + t * Vtemp[j+1].x;
			Vtemp[j].y = (1.0 - t) * Vtemp[j].y + t * Vtemp[j+1].y;
		}
	}

	*Q = Vtemp[0];
	g_free (Vtemp);
}

/*
 * ComputeLeftTangent, ComputeRightTangent, ComputeCenterTangent :
 *Approximate unit tangents at endpoints and "center" of digitized curve
 */
/** Estimate the (forward) tangent at point d[first + 0.5].

    Unlike the center and right versions, this calculates the tangent in the way one might expect,
    i.e. wrt increasing index into d.
**/
void
sp_darray_left_tangent (const NRPoint *d, int first, int len, NRPoint *tHat)
{
	int second, l2, i;
	second = first + 1;
	l2 = len / 2;
	tHat->x = (d[second].x - d[first].x) * l2;
	tHat->y = (d[second].y - d[first].y) * l2;
	for (i = 1; i < l2; i++) {
		tHat->x += (d[second + i].x - d[first].x) * (l2 - i);
		tHat->y += (d[second + i].y - d[first].y) * (l2 - i);
	}
	sp_vector_normalize (tHat);
}

/** Estimates the (backward) tangent at d[last - 0.5].

    N.B. The tangent is "backwards", i.e. it is with respect to decreasing index rather than
    increasing index.

    Requires: point_ne (d[last],
			d[last - 1]).
*/
void
sp_darray_right_tangent (const NRPoint *d, int last, int len, NRPoint *tHat)
{
	int prev, l2, i;

	prev = last - 1;
	l2 = len / 2;
	tHat->x = (d[prev].x - d[last].x) * l2;
	tHat->y = (d[prev].y - d[last].y) * l2;
	for (i = 1; i < l2; i++) {
		tHat->x += (d[prev - i].x - d[last].x) * (l2 - i);
		tHat->y += (d[prev - i].y - d[last].y) * (l2 - i);
	}
	sp_vector_normalize (tHat);
}

/** Estimates the (backward) tangent at d[center], by averaging the two segments connected to
    d[center] (and then normalizing the result).

    N.B. The tangent is "backwards", i.e. it is with respect to decreasing index rather than
    increasing index.

    Requires: point_ne (d[center - 1],
			d[center + 1]).
*/
void
sp_darray_center_tangent (const NRPoint *d,
			  gint            center,
			  NRPoint       *tHatCenter)
{
	NRPoint	V1, V2;
	g_return_if_fail (center >= 1);

	sp_vector_sub (&d[center-1], &d[center], &V1);
	sp_vector_sub(&d[center], &d[center+1], &V2);
	tHatCenter->x = (V1.x + V2.x)/2.0;
	tHatCenter->y = (V1.y + V2.y)/2.0;
	sp_vector_normalize(tHatCenter);
}

/*
 *  ChordLengthParameterize :
 *	Assign parameter values to digitized points 
 *	using relative distances between points.
 *
 * Parameter array u has to be allocated with the same length as data
 */
static void
ChordLengthParameterize(const NRPoint *d, gdouble *u, gint len)
{
	g_return_if_fail (2 <= len);
	gint i;	

	/* First let u[i] equal the distance travelled along the path from d[0] to d[i]. */
	u[0] = 0.0;
	for (i = 1; i < len; i++) {
		u[i] = u[i-1] + hypot (d[i].x - d[i-1].x, d[i].y - d[i-1].y);
	}

	gdouble tot_len = u[len - 1];
	g_return_if_fail (tot_len != 0);
	for (i = 1; i < len; i++) {
		u[i] = u[i] / tot_len;
	}

#ifdef BEZIER_DEBUG
	g_assert (u[0] == 0.0 && u[len - 1] == 1.0);
	for (i = 1; i < len; i++) {
		g_assert (u[i] >= u[i-1]);
	}
#endif
}




/*
 *  ComputeMaxError :
 *	Find the maximum squared distance of digitized points
 *	to fitted curve.
*/
static gdouble
ComputeMaxError (const NRPoint *d, gdouble *u, gint len, const BezierCurve bezCurve, gint *splitPoint)
{
	int i;
	double maxDist; /* Maximum error */
	double dist; /* Current error */
	NRPoint P; /* Point on curve */
	NRPoint v; /* Vector from point to curve */

	*splitPoint = len / 2;

	maxDist = 0.0;
	for (i = 1; i < len; i++) {
		BezierII (3, bezCurve, u[i], &P);
		sp_vector_sub (&P, &d[i], &v);
		dist = v.x * v.x + v.y * v.y;
		if (dist >= maxDist) {
			maxDist = dist;
			*splitPoint = i;
		}
	}

	return maxDist;
}

static void
sp_vector_add (const NRPoint *a, const NRPoint *b, NRPoint *c)
{
	c->x = a->x + b->x;
	c->y = a->y + b->y;
}

static void
sp_vector_sub (const NRPoint *a, const NRPoint *b, NRPoint *c)
{
	c->x = a->x - b->x;
	c->y = a->y - b->y;
}

static void
sp_vector_scale (const NRPoint *v, gdouble s, NRPoint *result)
{
	result->x = v->x * s;
	result->y = v->y * s;
}

static void
sp_vector_normalize (NRPoint *v)
{
	gdouble len;

	len = hypot (v->x, v->y);
	g_return_if_fail (len != 0);
	v->x /= len;
	v->y /= len;
}

static void
sp_vector_negate (NRPoint *v)
{
	v->x = -v->x;
	v->y = -v->y;
}
