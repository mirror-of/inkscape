#define __NR_STROKE_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define noNR_STROKE_VERBOSE

#include <math.h>

#include "nr-rect.h"
#include "nr-matrix.h"
#include "nr-svp-uncross.h"
#include "nr-svp-private.h"

#ifdef NR_STROKE_VERBOSE
#include <stdio.h>
#endif

#include "nr-stroke.h"

typedef struct _NRSVLStrokeBuild NRSVLStrokeBuild;

struct _NRSVLStrokeBuild {
	NRSVL *svl;
	NRFlat *flats;
	NRMatrixF transform;
	unsigned int closed : 1;
	unsigned int cap : 2;
	unsigned int join : 2;
	unsigned int curve : 1;
	float width_2;
	double cosml;
	int npoints;
	NRRectF bbox;
	NRCoord x[4];
	NRCoord y[4];
	NRSVLBuild left, right;
};

static void nr_svl_stroke_build_draw_cap (NRSVLStrokeBuild *svlb, float x0, float y0, float x1, float y1, unsigned int finish);
static void nr_svl_stroke_build_draw_join (NRSVLStrokeBuild *svlb, float x0, float y0, float x1, float y1, float x2, float y2);

static void
nr_svl_stroke_build_start_closed_subpath (NRSVLStrokeBuild *svlb, float x, float y)
{
	x = (float) NR_COORD_X_FROM_ART (x);
	y = (float) NR_COORD_Y_FROM_ART (y);
#ifdef NR_STROKE_VERBOSE
	printf ("CLOSED %g %g\n", x, y);
#endif
	svlb->closed = TRUE;
	svlb->x[0] = svlb->x[3] = x;
	svlb->y[0] = svlb->y[3] = y;
	svlb->npoints = 1;
}

static void
nr_svl_stroke_build_start_open_subpath (NRSVLStrokeBuild *svlb, float x, float y)
{
	x = (float) NR_COORD_X_FROM_ART (x);
	y = (float) NR_COORD_Y_FROM_ART (y);
#ifdef NR_STROKE_VERBOSE
	printf ("OPEN %g %g\n", x, y);
#endif
	svlb->closed = FALSE;
	svlb->x[0] = svlb->x[3] = x;
	svlb->y[0] = svlb->y[3] = y;
	svlb->npoints = 1;
}

static void
nr_svl_stroke_build_lineto (NRSVLStrokeBuild *svlb, float x, float y)
{
	x = (float) NR_COORD_X_FROM_ART (x);
	y = (float) NR_COORD_Y_FROM_ART (y);
#ifdef NR_STROKE_VERBOSE
	printf ("LINETO %g %g\n", x, y);
#endif
	if ((x != svlb->x[3]) || (y != svlb->y[3])) {
		if (svlb->npoints == 1) {
			float len, dx, dy;
			/* Second point on line */
			svlb->x[1] = x;
			svlb->y[1] = y;
			len = hypot (svlb->x[1] - svlb->x[0], svlb->y[1] - svlb->y[0]);
			dx = (svlb->x[1] - svlb->x[0]) / len;
			dy = (svlb->y[1] - svlb->y[0]) / len;
			if (!svlb->closed) {
				/* Draw cap[0,1] if open */
				nr_svl_build_moveto (&svlb->left, svlb->x[0] + dy * svlb->width_2, svlb->y[0] - dx * svlb->width_2);
				nr_svl_build_moveto (&svlb->right, svlb->x[0] + dy * svlb->width_2, svlb->y[0] - dx * svlb->width_2);
				nr_svl_stroke_build_draw_cap (svlb, svlb->x[0], svlb->y[0], svlb->x[1], svlb->y[1], FALSE);
			} else {
				/* Set starting point */
				nr_svl_build_moveto (&svlb->left, svlb->x[0] - dy * svlb->width_2, svlb->y[0] + dx * svlb->width_2);
				nr_svl_build_moveto (&svlb->right, svlb->x[0] + dy * svlb->width_2, svlb->y[0] - dx * svlb->width_2);
			}
		} else {
			/* Draw 2->3 + join 2->3->CP */
			nr_svl_stroke_build_draw_join (svlb, svlb->x[2], svlb->y[2], svlb->x[3], svlb->y[3], x, y);
		}
		svlb->x[2] = svlb->x[3];
		svlb->y[2] = svlb->y[3];
		svlb->x[3] = x;
		svlb->y[3] = y;
		svlb->npoints += 1;
	}
}

static void
nr_svl_stroke_build_finish_subpath (NRSVLStrokeBuild *svlb)
{
#ifdef NR_STROKE_VERBOSE
	printf ("FINISH\n");
#endif
	if (svlb->npoints < 1) return;
	if (svlb->closed) {
		float len, dx, dy;
		/* Draw 2->3 + join 2->3->1 */
		nr_svl_stroke_build_draw_join (svlb, svlb->x[2], svlb->y[2], svlb->x[3], svlb->y[3], svlb->x[1], svlb->y[1]);
		/* And finsih possibly open paths */
		len = hypot (svlb->x[1] - svlb->x[0], svlb->y[1] - svlb->y[0]);
		dx = (svlb->x[1] - svlb->x[0]) / len;
		dy = (svlb->y[1] - svlb->y[0]) / len;
		nr_svl_build_lineto (&svlb->left, svlb->x[0] - dy * svlb->width_2, svlb->y[0] + dx * svlb->width_2);
		nr_svl_build_lineto (&svlb->right, svlb->x[0] + dy * svlb->width_2, svlb->y[0] - dx * svlb->width_2);
	} else {
		/* Draw 2->3 plus cap 2->3 */
		nr_svl_stroke_build_draw_cap (svlb, svlb->x[3], svlb->y[3], svlb->x[2], svlb->y[2], TRUE);
	}
}

static void
nr_svl_stroke_build_finish_segment (NRSVLStrokeBuild *svlb)
{
	nr_svl_build_finish_segment (&svlb->left);
	nr_svl_build_finish_segment (&svlb->right);
}

#define MAX_SUBDIVIDE_DEPTH 10

static void
nr_svl_stroke_build_curveto (NRSVLStrokeBuild *svlb,
			     double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3,
			     float flatness, int level)
{
	double t_x0, t_y0, t_x1, t_y1, t_x2, t_y2, t_x3, t_y3;
	double dx1_0, dy1_0, dx2_0, dy2_0, dx3_0, dy3_0, dx2_3, dy2_3, d3_0_2;
	double s1_q, t1_q, s2_q, t2_q, v2_q;
	double f2, f2_q;
	double x00t, y00t, x0tt, y0tt, xttt, yttt, x1tt, y1tt, x11t, y11t;

	if (level >= MAX_SUBDIVIDE_DEPTH) goto nosubdivide;

	t_x0 = NR_MATRIX_DF_TRANSFORM_X (&svlb->transform, x0, y0);
	t_y0 = NR_MATRIX_DF_TRANSFORM_Y (&svlb->transform, x0, y0);
	t_x1 = NR_MATRIX_DF_TRANSFORM_X (&svlb->transform, x1, y1);
	t_y1 = NR_MATRIX_DF_TRANSFORM_Y (&svlb->transform, x1, y1);
	t_x2 = NR_MATRIX_DF_TRANSFORM_X (&svlb->transform, x2, y2);
	t_y2 = NR_MATRIX_DF_TRANSFORM_Y (&svlb->transform, x2, y2);
	t_x3 = NR_MATRIX_DF_TRANSFORM_X (&svlb->transform, x3, y3);
	t_y3 = NR_MATRIX_DF_TRANSFORM_Y (&svlb->transform, x3, y3);

	dx1_0 = t_x1 - t_x0;
	dy1_0 = t_y1 - t_y0;
	dx2_0 = t_x2 - t_x0;
	dy2_0 = t_y2 - t_y0;
	dx3_0 = t_x3 - t_x0;
	dy3_0 = t_y3 - t_y0;
	dx2_3 = t_x3 - t_x2;
	dy2_3 = t_y3 - t_y2;
	f2 = flatness * flatness;
	d3_0_2 = dx3_0 * dx3_0 + dy3_0 * dy3_0;
	if (d3_0_2 < f2) {
		double d1_0_2, d2_0_2;
		d1_0_2 = dx1_0 * dx1_0 + dy1_0 * dy1_0;
		d2_0_2 = dx2_0 * dx2_0 + dy2_0 * dy2_0;
		if ((d1_0_2 < f2) && (d2_0_2 < f2)) {
			goto nosubdivide;
		} else {
			goto subdivide;
		}
	}
	f2_q = flatness * flatness * d3_0_2;
	s1_q = dx1_0 * dx3_0 + dy1_0 * dy3_0;
	t1_q = dy1_0 * dx3_0 - dx1_0 * dy3_0;
	s2_q = dx2_0 * dx3_0 + dy2_0 * dy3_0;
	t2_q = dy2_0 * dx3_0 - dx2_0 * dy3_0;
	v2_q = dx2_3 * dx3_0 + dy2_3 * dy3_0;
	if ((t1_q * t1_q) > f2_q) goto subdivide;
	if ((t2_q * t2_q) > f2_q) goto subdivide;
	if ((s1_q < 0.0) && ((s1_q * s1_q) > f2_q)) goto subdivide;
	if ((v2_q < 0.0) && ((v2_q * v2_q) > f2_q)) goto subdivide;
	if (s1_q >= s2_q) goto subdivide;

 nosubdivide:
	nr_svl_stroke_build_lineto (svlb, (float) x3, (float) y3);
	return;

 subdivide:
	x00t = (x0 + x1) * 0.5;
	y00t = (y0 + y1) * 0.5;
	x0tt = (x0 + 2 * x1 + x2) * 0.25;
	y0tt = (y0 + 2 * y1 + y2) * 0.25;
	x1tt = (x1 + 2 * x2 + x3) * 0.25;
	y1tt = (y1 + 2 * y2 + y3) * 0.25;
	x11t = (x2 + x3) * 0.5;
	y11t = (y2 + y3) * 0.5;
	xttt = (x0tt + x1tt) * 0.5;
	yttt = (y0tt + y1tt) * 0.5;

	nr_svl_stroke_build_curveto (svlb, x0, y0, x00t, y00t, x0tt, y0tt, xttt, yttt, flatness, level + 1);
	/* Force all joins after first to bevels */
	svlb->curve = TRUE;
	nr_svl_stroke_build_curveto (svlb, xttt, yttt, x1tt, y1tt, x11t, y11t, x3, y3, flatness, level + 1);
}

NRSVL *
nr_bpath_stroke (const NRBPath *path, NRMatrixF *transform,
		 float width,
		 unsigned int cap, unsigned int join, float miterlimit,
		 float flatness)
{
	NRSVLStrokeBuild svlb;
	ArtBpath *bp;
	double x, y, sx, sy;

	/* Initialize NRSVLBuilds */
	svlb.svl = NULL;
	svlb.flats = NULL;
	if (transform) {
		svlb.transform = *transform;
	} else {
		nr_matrix_f_set_identity (&svlb.transform);
	}
	svlb.closed = FALSE;
	svlb.width_2 = width / 2.0;
	svlb.cap = cap;
	svlb.join = join;
	svlb.curve = FALSE;
	svlb.cosml = MIN (cos (miterlimit), 0.9998477);
	svlb.npoints = 0;

	svlb.left.svl = &svlb.svl;
	svlb.left.flats = &svlb.flats;
	svlb.left.refvx = NULL;
	svlb.left.dir = 0;
	svlb.left.reverse = FALSE;
	svlb.left.sx = svlb.left.sy = 0.0;
	nr_rect_f_set_empty (&svlb.left.bbox);

	svlb.right.svl = &svlb.svl;
	svlb.right.flats = &svlb.flats;
	svlb.right.refvx = NULL;
	svlb.right.dir = 0;
	svlb.right.reverse = TRUE;
	svlb.right.sx = svlb.right.sy = 0.0;
	nr_rect_f_set_empty (&svlb.right.bbox);

	x = y = 0.0;
	sx = sy = 0.0;

	for (bp = path->path; bp->code != ART_END; bp++) {
		switch (bp->code) {
		case ART_MOVETO:
			nr_svl_stroke_build_finish_subpath (&svlb);
			sx = x = bp->x3;
			sy = y = bp->y3;
			nr_svl_stroke_build_start_closed_subpath (&svlb, (float) x, (float) y);
			break;
		case ART_MOVETO_OPEN:
			nr_svl_stroke_build_finish_subpath (&svlb);
			sx = x = bp->x3;
			sy = y = bp->y3;
			nr_svl_stroke_build_start_open_subpath (&svlb, (float) x, (float) y);
			break;
		case ART_LINETO:
			sx = x = bp->x3;
			sy = y = bp->y3;
			nr_svl_stroke_build_lineto (&svlb, (float) x, (float) y);
			break;
		case ART_CURVETO:
			x = bp->x3;
			y = bp->y3;
			nr_svl_stroke_build_curveto (&svlb, sx, sy, bp->x1, bp->y1, bp->x2, bp->y2, x, y, flatness, 0);
			/* Restore original join type */
			svlb.curve = FALSE;
			sx = x;
			sy = y;
			break;
		default:
			/* fixme: free lists */
			return NULL;
			break;
		}
	}
	nr_svl_stroke_build_finish_subpath (&svlb);
	nr_svl_stroke_build_finish_segment (&svlb);
	if (svlb.svl) {
		/* NRSVL *s; */
		svlb.svl = nr_svl_uncross_full (svlb.svl, svlb.flats, NR_WIND_RULE_NONZERO);
	} else {
		nr_flat_free_list (svlb.flats);
	}
	/* This happnes in uncross */
	/* nr_flat_free_list (flats); */

	return svlb.svl;
}

NRSVL *
nr_vpath_stroke (const ArtVpath *path, NRMatrixF *transform,
		 float width,
		 unsigned int cap, unsigned int join, float miterlimit,
		 float flatness)
{
	NRSVLStrokeBuild svlb;
	const ArtVpath *p;
	double x, y, sx, sy;

	/* Initialize NRSVLBuilds */
	svlb.svl = NULL;
	svlb.flats = NULL;
	if (transform) {
		svlb.transform = *transform;
	} else {
		nr_matrix_f_set_identity (&svlb.transform);
	}
	svlb.closed = FALSE;
	svlb.width_2 = width / 2.0;
	svlb.cap = cap;
	svlb.join = join;
	svlb.curve = FALSE;
	svlb.cosml = MIN (cos (miterlimit), 0.9998477);
	svlb.npoints = 0;

	svlb.left.svl = &svlb.svl;
	svlb.left.flats = &svlb.flats;
	svlb.left.refvx = NULL;
	svlb.left.dir = 0;
	svlb.left.reverse = FALSE;
	svlb.left.sx = svlb.left.sy = 0.0;
	nr_rect_f_set_empty (&svlb.left.bbox);

	svlb.right.svl = &svlb.svl;
	svlb.right.flats = &svlb.flats;
	svlb.right.refvx = NULL;
	svlb.right.dir = 0;
	svlb.right.reverse = TRUE;
	svlb.right.sx = svlb.right.sy = 0.0;
	nr_rect_f_set_empty (&svlb.right.bbox);

	x = y = 0.0;
	sx = sy = 0.0;

	for (p = path; p->code != ART_END; p++) {
		switch (p->code) {
		case ART_MOVETO:
			nr_svl_stroke_build_finish_subpath (&svlb);
			sx = x = p->x;
			sy = y = p->y;
			nr_svl_stroke_build_start_closed_subpath (&svlb, (float) x, (float) y);
			break;
		case ART_MOVETO_OPEN:
			nr_svl_stroke_build_finish_subpath (&svlb);
			sx = x = p->x;
			sy = y = p->y;
			nr_svl_stroke_build_start_open_subpath (&svlb, (float) x, (float) y);
			break;
		case ART_LINETO:
			sx = x = p->x;
			sy = y = p->y;
			nr_svl_stroke_build_lineto (&svlb, (float) x, (float) y);
			break;
		default:
			/* fixme: free lists */
			return NULL;
			break;
		}
	}
	nr_svl_stroke_build_finish_subpath (&svlb);
	nr_svl_stroke_build_finish_segment (&svlb);
	if (svlb.svl) {
		/* NRSVL *s; */
		svlb.svl = nr_svl_uncross_full (svlb.svl, svlb.flats, NR_WIND_RULE_NONZERO);
	} else {
		nr_flat_free_list (svlb.flats);
	}
	/* This happnes in uncross */
	/* nr_flat_free_list (flats); */

	return svlb.svl;
}

static void
nr_svl_stroke_build_draw_cap (NRSVLStrokeBuild *svlb, float x0, float y0, float x1, float y1, unsigned int finish)
{
	float len, dx, dy;
	len = hypot (x1 - x0, y1 - y0);
	dx = (x1 - x0) / len;
	dy = (y1 - y0) / len;
	if (svlb->cap == NR_STROKE_CAP_BUTT) {
		/* Butt */
		nr_svl_build_lineto (&svlb->left,
				     x0 + dy * svlb->width_2,
				     y0 - dx * svlb->width_2);
		nr_svl_build_lineto (&svlb->left,
				     x0 - dy * svlb->width_2,
				     y0 + dx * svlb->width_2);
		if (finish) {
			nr_svl_build_lineto (&svlb->right,
					     x0 - dy * svlb->width_2,
					     y0 + dx * svlb->width_2);
		}
	} else if (svlb->cap == NR_STROKE_CAP_ROUND) {
		float theta;
		/* Round */
		nr_svl_build_lineto (&svlb->left,
				     x0 + dy * svlb->width_2,
				     y0 - dx * svlb->width_2);
		for (theta = 0.0; theta < M_PI; theta += (M_PI / 32.0)) {
			float ct, st;
			ct = cos (theta);
			st = sin (theta);
			nr_svl_build_lineto (&svlb->left,
					     x0 + ct * dy * svlb->width_2 - st * dx * svlb->width_2,
					     y0 - ct * dx * svlb->width_2 - st * dy * svlb->width_2);
		}
		nr_svl_build_lineto (&svlb->left,
				     x0 - dy * svlb->width_2,
				     y0 + dx * svlb->width_2);
		if (finish) {
			nr_svl_build_lineto (&svlb->right,
					     x0 - dy * svlb->width_2,
					     y0 + dx * svlb->width_2);
		}
	} else {
		/* Square */
		nr_svl_build_lineto (&svlb->left,
				     x0 + dy * svlb->width_2 - dx * svlb->width_2,
				     y0 - dx * svlb->width_2 - dy * svlb->width_2);
		nr_svl_build_lineto (&svlb->left,
				     x0 - dy * svlb->width_2 - dx * svlb->width_2,
				     y0 + dx * svlb->width_2 - dy * svlb->width_2);
		if (finish) {
			nr_svl_build_lineto (&svlb->right,
					     x0 - dy * svlb->width_2 - dx * svlb->width_2,
					     y0 + dx * svlb->width_2 - dy * svlb->width_2);
		}
	}
}

#define NR_STRAIGHT_TRESHOLD -0.99999848
#define SIN_R 0.09801714
#define COS_R 0.99518473

static void
nr_svl_stroke_build_draw_join (NRSVLStrokeBuild *svlb, float x0, float y0, float x1, float y1, float x2, float y2)
{
	float len0, dx0, dy0, px0, py0;
	float len1, dx1, dy1, px1, py1;
	double costheta;
	len0 = hypot (x1 - x0, y1 - y0);
	dx0 = (x1 - x0) / len0;
	dy0 = (y1 - y0) / len0;
	px0 = dy0 * svlb->width_2;
	py0 = -dx0 * svlb->width_2;
	len1 = hypot (x2 - x1, y2 - y1);
	dx1 = (x2 - x1) / len1;
	dy1 = (y2 - y1) / len1;
	px1 = dy1 * svlb->width_2;
	py1 = -dx1 * svlb->width_2;
	costheta = -dx0 * dx1 + -dy0 * dy1;
	if (costheta > NR_STRAIGHT_TRESHOLD) {
		unsigned int join;
		double dx, dy, d2, D_d;
		double dir;
		join = svlb->join;
		if (svlb->curve) join = NR_STROKE_JOIN_ROUND;
		if ((join == NR_STROKE_JOIN_MITER) && (costheta > svlb->cosml)) join = NR_STROKE_JOIN_BEVEL;
		dx = (px0 + px1) / 2.0;
		dy = (py0 + py1) / 2.0;
		d2 = dx * dx + dy * dy;
		D_d = svlb->width_2 * svlb->width_2 / d2;
		dx *= D_d;
		dy *= D_d;
		dir = dx0 * dy1 - dx1 * dy0;
		if (dir > 0.0) {
			double cx, cy, lx0, ly0, lx1, ly1;
			/* Left is inner */
			/* Draw left side */
			/* Find inner intersection point */
			cx = x1 - dx;
			cy = y1 - dy;
			/* Projections to segments */
			lx0 = cx + px0 - x1;
			ly0 = cy + py0 - y1;
			lx1 = cx + px1 - x1;
			ly1 = cy + py1 - y1;
			if (((lx0 * lx0 + ly0 * ly0) < (len0 * len0)) && ((lx1 * lx1 + ly1 * ly1) < (len1 * len1))) {
				/* Can use intersection point */
				nr_svl_build_lineto (&svlb->left, cx, cy);
			} else {
				/* Draw reversed bevel */
				nr_svl_build_lineto (&svlb->left, x1 - px0, y1 - py0);
				nr_svl_build_lineto (&svlb->left, x1 - px1, y1 - py1);
			}
			/* Draw right side */
			if (join == NR_STROKE_JOIN_MITER) {
				/* Miter */
				nr_svl_build_lineto (&svlb->right, x1 + dx, y1 + dy);
			} else if (join == NR_STROKE_JOIN_ROUND) {
				double px, py;
				/* Round */
				nr_svl_build_lineto (&svlb->right, x1 + px0, y1 + py0);
				px = COS_R * px0 + -SIN_R * py0;
				py = SIN_R * px0 + COS_R * py0;
				while ((px * py1 - py * px1) > 0.0) {
					double sx, sy;
					nr_svl_build_lineto (&svlb->right, x1 + px, y1 + py);
					sx = COS_R * px + -SIN_R * py;
					sy = SIN_R * px + COS_R * py;
					px = sx;
					py = sy;
				}
				nr_svl_build_lineto (&svlb->right, x1 + px1, y1 + py1);
			} else {
				/* Bevel */
				nr_svl_build_lineto (&svlb->right, x1 + px0, y1 + py0);
				nr_svl_build_lineto (&svlb->right, x1 + px1, y1 + py1);
			}
		} else {
			double cx, cy, lx0, ly0, lx1, ly1;
			/* Right is inner */
			/* Draw right side */
			/* Find inner intersection point */
			cx = x1 + dx;
			cy = y1 + dy;
			/* Projections to segments */
			lx0 = cx - px0 - x1;
			ly0 = cy - py0 - y1;
			lx1 = cx - px1 - x1;
			ly1 = cy - py1 - y1;
			if (((lx0 * lx0 + ly0 * ly0) < (len0 * len0)) && ((lx1 * lx1 + ly1 * ly1) < (len1 * len1))) {
				/* Can use intersection point */
				nr_svl_build_lineto (&svlb->right, cx, cy);
			} else {
				/* Draw reversed bevel */
				nr_svl_build_lineto (&svlb->right, x1 + px0, y1 + py0);
				nr_svl_build_lineto (&svlb->right, x1 + px1, y1 + py1);
			}
			/* Draw left side */
			if (join == NR_STROKE_JOIN_MITER) {
				/* Miter */
				nr_svl_build_lineto (&svlb->left, x1 - dx, y1 - dy);
			} else if (join == NR_STROKE_JOIN_ROUND) {
				double px, py;
				/* Round */
				nr_svl_build_lineto (&svlb->left, x1 - px0, y1 - py0);
				px = COS_R * -px0 + SIN_R * -py0;
				py = -SIN_R * -px0 + COS_R * -py0;
				while ((px * -py1 - py * -px1) < 0.0) {
					double sx, sy;
					nr_svl_build_lineto (&svlb->left, x1 + px, y1 + py);
					sx = COS_R * px + SIN_R * py;
					sy = -SIN_R * px + COS_R * py;
					px = sx;
					py = sy;
				}
				nr_svl_build_lineto (&svlb->left, x1 - px1, y1 - py1);
			} else {
				/* Bevel */
				/* fixme: */
				nr_svl_build_lineto (&svlb->left, x1 - px0, y1 - py0);
				nr_svl_build_lineto (&svlb->left, x1 - px1, y1 - py1);
			}
		}
	} else {
		/* Straight */
		/* Draw right side */
		nr_svl_build_lineto (&svlb->right,
				     x1 + 0.5 * (dy0 + dy1) * svlb->width_2,
				     y1 - 0.5 * (dx0 + dx1) * svlb->width_2);
		/* Draw left side */
		nr_svl_build_lineto (&svlb->left,
				     x1 - 0.5 * (dy0 + dy1) * svlb->width_2,
				     y1 + 0.5 * (dx0 + dx1) * svlb->width_2);
	}
}


