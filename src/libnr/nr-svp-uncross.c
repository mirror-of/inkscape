#define __NR_SVP_UNCROSS_C__

/*
 * Pixel buffer rendering library
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#define noNR_EXTRA_CHECK
#define noNR_VERBOSE

#define NR_COORD_SNAP(v) (floor (NR_QUANT_Y * (v) + 0.5) / NR_QUANT_Y)
#define NR_COORD_SNAP_UP(v) (ceil (NR_QUANT_Y * (v)) / NR_QUANT_Y)
#define NR_COORD_SNAP_DOWN(v) (floor (NR_QUANT_Y * (v)) / NR_QUANT_Y)
#define NR_COORD_TOLERANCE (1.0F / NR_QUANT_X)
#define NR_COORD_TOLERANCE2 (NR_COORD_TOLERANCE * NR_COORD_TOLERANCE)

#include <math.h>
#include <assert.h>
#include <stdio.h>

#include "nr-macros.h"
#include "nr-values.h"
#include "nr-svp-private.h"
#include "nr-svp-uncross.h"

typedef struct _NRSVLSlice NRSVLSlice;

struct _NRSVLSlice {
	NRSVLSlice *next;
	NRSVL *svl;
	NRVertex *vertex;
	NRCoord x;
	NRCoord y;
};

static NRSVL *nr_svl_slice_break (NRSVLSlice *s, double x, double y, NRSVL *svl);
static NRSVL *nr_svl_slice_break_y_and_continue_x (NRSVLSlice *s, double y, double x, NRSVL *svl, double ytest, NRFlat **flats);

static NRSVLSlice *nr_svl_slice_new (NRSVL *svl, NRCoord y);
static void nr_svl_slice_free_one (NRSVLSlice *slice);
static int nr_svl_slice_compare (NRSVLSlice *l, NRSVLSlice *r);
static NRSVLSlice *nr_svl_slice_insert_sorted (NRSVLSlice *start, NRSVLSlice *slice);

static NRSVLSlice *nr_svl_slice_stretch_list (NRSVLSlice *slices, NRCoord y);

static double nr_vertex_segment_distance2 (NRVertex *v, NRVertex *s);
static double nr_segment_intersection (NRVertex *s0, NRVertex *s1, double *x, double *y);

#ifdef NR_EXTRA_CHECK
static void
nr_svl_test_slices (NRSVLSlice *slices, double yslice, const unsigned char *prefix, int colinear, int test, int close)
{
	NRSVLSlice *s;
	int wind;
	wind = 0;
	for (s = slices; s && s->next; s = s->next) {
		NRSVLSlice *cs, *ns;
		double cx0, cy0, cx1, cy1, cx;
		double nx0, ny0, nx1, ny1, nx;
		cs = s;
		ns = s->next;
		cx0 = cs->vertex->x;
		cy0 = cs->vertex->y;
		cx1 = cs->vertex->next->x;
		cy1 = cs->vertex->next->y;
		if (yslice == cy0) {
			cx = cx0;
		} else {
			cx = cx0 + (cx1 - cx0) * (yslice - cy0) / (cy1 - cy0);
		}
		nx0 = ns->vertex->x;
		ny0 = ns->vertex->y;
		nx1 = ns->vertex->next->x;
		ny1 = ns->vertex->next->y;
		if (yslice == ny0) {
			nx = nx0;
		} else {
			nx = nx0 + (nx1 - nx0) * (yslice - ny0) / (ny1 - ny0);
		}
		if (cx > nx) {
			printf ("%s: Slice x %g > %g\n", prefix, cx, nx);
		} else if (cx == nx) {
			double ldx, ldy, rdx, rdy;
			double d;
			ldx = cx1 - cx;
			ldy = cy1 - yslice;
			rdx = nx1 - nx;
			rdy = ny1 - yslice;
			d = ldx * rdy - ldy * rdx;
			if (d == 0.0) {
				if (colinear) printf ("%s: Slice x %g COLINEAR %g\n", prefix, cx, nx);
			} else if (d > 0.0) {
				if (test) printf ("%s: Slice x %g > TEST %g\n", prefix, cx, nx);
			}
		} else if (cx > (nx - NR_EPSILON_F)) {
			if (close) printf ("%s: Slice x %g < EPSILON %g\n", prefix, cx, nx);
		}
		wind += s->svl->wind;
	}
	if (s) wind += s->svl->wind;
	if (wind & 1) printf ("%s: Weird wind %d\n", prefix, wind);
}
#define CHECK_SLICES(s,y,p,c,t,n) nr_svl_test_slices (s,y,p,c,t,n)
#else
#define CHECK_SLICES(s,y,p,c,t,n)
#endif

NRSVL *
nr_svl_uncross_full (NRSVL *svl, NRFlat *flats, unsigned int windrule)
{
	NRSVL *lsvl, *csvl, *nsvl;
	NRFlat *nflat, *fl, *f;
	NRSVLSlice *slices, *s;
	NRCoord yslice, ynew;

	if (!svl) return NULL;
	assert (svl->vertex);

	slices = NULL;

	/* First slicing position */
	yslice = svl->vertex->y;
	nflat = flats;
	/* Drop all flats below initial slice */
	/* Equal can be dropped too in given case */
	fl = NULL;
	for (f = nflat; f && (f->y <= yslice); f = f->next) fl = f;
	if (fl) {
		fl->next = NULL;
		nr_flat_free_list (nflat);
		nflat = f;
	}

	/* fixme: the lsvl stuff is really braindead */
	lsvl = NULL;
	csvl = svl;
	nsvl = svl;

	/* Main iteration */
	while ((slices) || (nsvl)) {
		NRSVLSlice * ss, * cs, * ns;
		NRVertex * newvertex;
		NRSVL * newsvl;
		int wind;

		/* Add svls == starting from yslice to slices list */
		assert (!nsvl || (nsvl->vertex->y >= yslice));
		while ((nsvl) && (nsvl->vertex->y == yslice)) {
			NRSVLSlice * newslice;
			newslice = nr_svl_slice_new (nsvl, yslice);
			slices = nr_svl_slice_insert_sorted (slices, newslice);
			nsvl = nsvl->next;
		}
		/* Now everything should be set up */
		CHECK_SLICES (slices, yslice, "PRE", 0, 0, 1);
		/* Process intersections */
		/* This is bitch */
		ss = NULL;
		cs = slices;
		while (cs && cs->next) {
			ns = cs->next;
			if (cs->x > ns->x) {
				/* Something is seriously messed up */
				/* Try to do, what we can */
				/* Break slices */
				csvl = nr_svl_slice_break (cs, cs->x, yslice, csvl);
				csvl = nr_svl_slice_break (ns, ns->x, yslice, csvl);
				/* Set the new starting point */
				f = nr_flat_new_full (yslice, ns->x, cs->x);
				nflat = nr_flat_insert_sorted (nflat, f);
				cs->vertex->x = ns->x;
				cs->x = cs->vertex->x;
				/* Reorder slices */
				if (ss) {
					assert (ns->next != ss);
					ss->next = ns->next;
				} else {
					slices = ns->next;
				}
				slices = nr_svl_slice_insert_sorted (slices, cs);
				slices = nr_svl_slice_insert_sorted (slices, ns);
				CHECK_SLICES (slices, yslice, "CHECK", 0, 0, 1);
				/* Start the row from the beginning */
				ss = NULL;
				cs = slices;
			} else if (cs->x == ns->x) {
				double order;
				/* Break if either one is new slice */
				if ((cs->y == cs->vertex->y) || (ns->y == ns->vertex->y)) {
					csvl = nr_svl_slice_break (cs, cs->x, yslice, csvl);
					csvl = nr_svl_slice_break (ns, ns->x, yslice, csvl);
				}
				/* test continuation direction */
				order = nr_svl_slice_compare (cs, ns);
				if (fabs (order < 0.01)) {
					double dist2;
					/* Potentially close, test endpoint */
					/* Bitch'o'bitches (Lauris) */
					if (cs->vertex->next->y < ns->vertex->next->y) {
						/* cs is shorter */
						dist2 = nr_vertex_segment_distance2 (cs->vertex->next, ns->vertex);
						if (dist2 < NR_COORD_TOLERANCE2) {
							csvl = nr_svl_slice_break_y_and_continue_x (cs,
												    cs->vertex->next->y,
												    cs->vertex->next->x,
												    csvl, yslice, &nflat);
							csvl = nr_svl_slice_break_y_and_continue_x (ns,
												    cs->vertex->next->y,
												    cs->vertex->next->x,
												    csvl, yslice, &nflat);
							/* fixme: Slight disturbance is possible so we should repeat */
						}
					} else {
						/* ns is equal or shorter */
						dist2 = nr_vertex_segment_distance2 (ns->vertex->next, cs->vertex);
						if (dist2 < NR_COORD_TOLERANCE2) {
							csvl = nr_svl_slice_break_y_and_continue_x (cs,
												    ns->vertex->next->y,
												    ns->vertex->next->x,
												    csvl, yslice, &nflat);
							csvl = nr_svl_slice_break_y_and_continue_x (ns,
												    ns->vertex->next->y,
												    ns->vertex->next->x,
												    csvl, yslice, &nflat);
							/* fixme: Slight disturbance is possible so we should repeat */
						}
					}
					order = nr_svl_slice_compare (cs, ns);
				}
				if (order > 0.0) {
					/* Ensure break */
					csvl = nr_svl_slice_break (cs, cs->x, yslice, csvl);
					csvl = nr_svl_slice_break (ns, ns->x, yslice, csvl);
					/* Swap slices */
					assert (ns->next != cs);
					cs->next = ns->next;
					assert (cs != ns);
					ns->next = cs;
					if (ss) {
						assert (ns != ss);
						ss->next = ns;
					} else {
						slices = ns;
					}
					cs = ns;
					/* fixme: If slices are almost paraller */
					/* we have to ensure they will be broken at endpoint */
					/* otherwise winding changes at end may be unnoticed */
				} else {
					ss = cs;
					cs = ns;
				}
			} else if ((ns->x - cs->x) <= NR_COORD_TOLERANCE) {
				/* Slices are very close at yslice */
				/* Start by breaking slices */
				csvl = nr_svl_slice_break (cs, cs->x, yslice, csvl);
				csvl = nr_svl_slice_break (ns, ns->x, yslice, csvl);
				/* Set the new starting point */
				if (ns->x > cs->x) {
					f = nr_flat_new_full (yslice, cs->x, ns->x);
					nflat = nr_flat_insert_sorted (nflat, f);
				}
				ns->vertex->x = cs->x;
				ns->x = ns->vertex->x;
				/* Reorder slices */
				if (ss) {
					assert (ns->next != ss);
					ss->next = ns->next;
				} else {
					slices = ns->next;
				}
				slices = nr_svl_slice_insert_sorted (slices, cs);
				slices = nr_svl_slice_insert_sorted (slices, ns);
				CHECK_SLICES (slices, yslice, "CHECK", 0, 0, 1);
				ss = NULL;
				cs = slices;
			} else if ((cs->vertex->next->y == ns->vertex->next->y) &&
				   ((ns->vertex->next->x - cs->vertex->next->x) <= NR_COORD_TOLERANCE) &&
				   ((cs->vertex->next->x - ns->vertex->next->x) <= NR_COORD_TOLERANCE)) {
				/* Coincident next vertices */
				csvl = nr_svl_slice_break_y_and_continue_x (cs,
									    cs->vertex->next->y,
									    cs->vertex->next->x,
									    csvl, yslice, &nflat);
				csvl = nr_svl_slice_break_y_and_continue_x (ns,
									    cs->vertex->next->y,
									    cs->vertex->next->x,
									    csvl, yslice, &nflat);
				ss = cs;
				cs = ns;
			} else if ((cs->x > ns->vertex->next->x) || (ns->x < cs->vertex->next->x) ||
				   (cs->vertex->next->x > ns->vertex->next->x)) {
				double d, x, y;
				d = nr_segment_intersection (cs->vertex, ns->vertex, &x, &y);
				if ((d >= 0.0) && (d < NR_COORD_TOLERANCE) && (y >= yslice)) {
					y = NR_COORD_SNAP_DOWN (y);
					/* fixme: */
					if (y <= yslice) {
						/* Slices are very close at yslice */
						/* Start by breaking slices */
						csvl = nr_svl_slice_break (cs, cs->x, yslice, csvl);
						csvl = nr_svl_slice_break (ns, ns->x, yslice, csvl);
						if ((ns->x - cs->x) <= NR_COORD_TOLERANCE) {
							/* Merge intersection into cs */
							x = cs->x;
						}
						if (cs->x != x) {
							double x0, x1;
							x0 = MIN (x, cs->x);
							x1 = MAX (x, cs->x);
							f = nr_flat_new_full (yslice, x0, x1);
							nflat = nr_flat_insert_sorted (nflat, f);
						}
						if (ns->x != cs->x) {
							double x0, x1;
							x0 = MIN (x, ns->x);
							x1 = MAX (x, ns->x);
							f = nr_flat_new_full (yslice, x0, x1);
							nflat = nr_flat_insert_sorted (nflat, f);
						}
						/* Set the new starting point */
						cs->vertex->x = x;
						cs->x = cs->vertex->x;
						ns->vertex->x = cs->x;
						ns->x = ns->vertex->x;
						/* Reorder slices */
						if (ss) {
							assert (ns->next != ss);
							ss->next = ns->next;
						} else {
							slices = ns->next;
						}
						slices = nr_svl_slice_insert_sorted (slices, cs);
						slices = nr_svl_slice_insert_sorted (slices, ns);
						CHECK_SLICES (slices, yslice, "CHECK", 0, 0, 1);
						/* Start the row from the beginning */
						ss = NULL;
						cs = slices;
					} else if ((y <= cs->vertex->next->y) && (y <= ns->vertex->next->y)) {
						if (((y < cs->vertex->next->y) || cs->vertex->next->next) &&
						    ((y < ns->vertex->next->y) || ns->vertex->next->next)) {
							/* Postpone by breaking svl */
							csvl = nr_svl_slice_break_y_and_continue_x (cs, y, x, csvl, yslice, &nflat);
							csvl = nr_svl_slice_break_y_and_continue_x (ns, y, x, csvl, yslice, &nflat);
						}
						/* fixme: Slight disturbance is possible so we should repeat */
						ss = cs;
						cs = ns;
					}
				} else {
					ss = cs;
					cs = ns;
				}
#if 0
				/* (MAX (cs->x, cs->vertex->next->x) > MIN (ns->x, ns->vertex->next->x)) */
				/* Potential intersection */
				double xba, yba, xdc, ydc;
				double d;

				/* Bitch 'o' bitches */
				xba = cs->vertex->next->x - cs->x;
				yba = cs->vertex->next->y - cs->y;
				xdc = ns->vertex->next->x - ns->x;
				ydc = ns->vertex->next->y - ns->y;
				d = xba * ydc - yba * xdc;

				if (fabs (d) > NR_EPSILON_F) {
					double xac, yac, numr, nums;
					double r, s, x, y, dr, ds, dr2, ds2;

					/* Not parallel */
					xac = cs->vertex->x - ns->vertex->x;
					yac = cs->vertex->y - ns->vertex->y;
					numr = yac * xdc - xac * ydc;
					nums = yac * xba - xac * yba;
					r = numr / d;
					s = nums / d;
					x = cs->vertex->x + r * xba;
					y = cs->vertex->y + r * yba;
					dr = 0.0;
					if (r < 0.0) dr = -r;
					if (r > 1.0) dr = r - 1.0;
					dr2 = dr * xba * xba + yba * yba;
					ds = 0.0;
					if (s < 0.0) ds = -s;
					if (s > 1.0) ds = s - 1.0;
					ds2 = ds * xdc * xdc + ydc * ydc;
					y = NR_COORD_SNAP_DOWN (y);
					/* fixme: */
					if ((dr2 < NR_COORD_TOLERANCE2) && (ds2 < NR_COORD_TOLERANCE2) &&
					    (y >= yslice) && (y <= cs->vertex->next->y) && (y <= ns->vertex->next->y)) {
						if (y == yslice) {
							/* Slices are very close at yslice */
							/* Start by breaking slices */
							csvl = nr_svl_slice_break (cs, cs->x, yslice, csvl);
							csvl = nr_svl_slice_break (ns, ns->x, yslice, csvl);
							if ((ns->x - cs->x) <= NR_COORD_TOLERANCE) {
								/* Merge intersection into cs */
								x = cs->x;
							}
							if (cs->x != x) {
								double x0, x1;
								x0 = MIN (x, cs->x);
								x1 = MAX (x, cs->x);
								f = nr_flat_new_full (y, x0, x1);
								nflat = nr_flat_insert_sorted (nflat, f);
							}
							if (ns->x != cs->x) {
								double x0, x1;
								x0 = MIN (x, ns->x);
								x1 = MAX (x, ns->x);
								f = nr_flat_new_full (y, x0, x1);
								nflat = nr_flat_insert_sorted (nflat, f);
							}
							/* Set the new starting point */
							cs->vertex->x = x;
							cs->x = cs->vertex->x;
							ns->vertex->x = cs->x;
							ns->x = ns->vertex->x;
							/* Reorder slices */
							if (ss) {
								assert (ns->next != ss);
								ss->next = ns->next;
							} else {
								slices = ns->next;
							}
							slices = nr_svl_slice_insert_sorted (slices, cs);
							slices = nr_svl_slice_insert_sorted (slices, ns);
							CHECK_SLICES (slices, yslice, "CHECK", 0, 0, 1);
							/* Start the row from the beginning */
							ss = NULL;
							cs = slices;
						} else {
							if (((y <= cs->vertex->next->y) || cs->vertex->next->next) &&
							    ((y <= ns->vertex->next->y) || ns->vertex->next->next)) {
								/* Postpone by breaking svl */
								csvl = nr_svl_slice_break_y_and_continue_x (cs, y, x, csvl, yslice, &nflat);
								csvl = nr_svl_slice_break_y_and_continue_x (ns, y, x, csvl, yslice, &nflat);
							}
							/* fixme: Slight disturbance is possible so we should repeat */
							ss = cs;
							cs = ns;
						}
					} else {
						ss = cs;
						cs = ns;
					}
				} else {
					ss = cs;
					cs = ns;
				}
#endif
			} else {
				ss = cs;
				cs = ns;
			}
		}
		/* Process flats (NB! we advance nflat to first > y) */
		assert (!nflat || (nflat->y >= yslice));

		fl = NULL;
		for (f = nflat; f && (f->y == yslice); f = f->next) {
			for (s = slices; s != NULL; s = s->next) {
				double x0, x1;
				assert (s->vertex->y <= yslice);
				assert (s->vertex->next->y > yslice);
				/* fixme: We can safely use EPSILON here */
				x0 = f->x0 - NR_COORD_TOLERANCE;
				x1 = f->x1 + NR_COORD_TOLERANCE;
				if ((s->x >= x0) && (s->x <= x1)) {
					if (s->vertex->y < yslice) {
						/* Mid-segment intersection */
						/* Create continuation svl */
						newvertex = nr_vertex_new_xy (s->x, s->y);
						newvertex->next = s->vertex->next;
						newsvl = nr_svl_new_vertex_wind (newvertex, s->svl->dir);
						/* Trim starting svl */
						newvertex = nr_vertex_new_xy (s->x, s->y);
						s->vertex->next = newvertex;
						nr_svl_calculate_bbox (s->svl);
						/* Insert new SVL into main list */
						/* new svl slice is included by definition */
						csvl = nr_svl_insert_sorted (csvl, newsvl);
						/* fixme: We should maintain pointer to ssvl */
						/* New svl is inserted before nsvl by definition, so we can ignore management */
						/* Old svl will be excluded by definition, so we can shortcut */
						s->svl = newsvl;
						s->vertex = newsvl->vertex;
						/* s->x and s->y are correct by definition */
					} else if (s->vertex != s->svl->vertex) {
						assert (s->vertex->y == yslice);
						/* Inter-segment intersection */
						/* Winding may change here */
						/* Create continuation svl */
						newvertex = nr_vertex_new_xy (s->x, s->y);
						newvertex->next = s->vertex->next;
						newsvl = nr_svl_new_vertex_wind (newvertex, s->svl->dir);
						/* Trim starting svl */
						s->vertex->next = NULL;
						nr_svl_calculate_bbox (s->svl);
						/* Insert new SVL into main list */
						/* new svl slice is included by definition */
						csvl = nr_svl_insert_sorted (csvl, newsvl);
						/* fixme: We should maintain pointer to ssvl */
						/* New svl is inserted before nsvl by definition, so we can ignore management */
						/* Old svl will be excluded by definition, so we can shortcut */
						s->svl = newsvl;
						s->vertex = newsvl->vertex;
						/* s->x and s->y are correct by definition */
					}
				}
			}
			fl = f;
		}
		if (fl) {
			fl->next = NULL;
			nr_flat_free_list (nflat);
			nflat = f;
		}
		CHECK_SLICES (slices, yslice, "POST", 0, 1, 1);
		/* Calculate winds */
		wind = 0;
		for (s = slices; s != NULL; s = s->next) {
			int cwind;
			cwind = wind + s->svl->dir;
			if (s->y == s->svl->vertex->y) {
				/* Starting SVL */
				if (windrule == NR_WIND_RULE_EVENODD) {
					s->svl->wind = (cwind & 0x1) ? 1 : -1;
				} else {
					if (!wind && cwind) {
						s->svl->wind = 1;
					} else if (wind && !cwind) {
						s->svl->wind = -1;
					} else {
						s->svl->wind = 0;
					}
				}
			}
			wind = cwind;
		}
#ifdef NR_VERBOSE
		if (wind & 1) printf ("Weird final wind: %d\n", wind);
#endif
		/* Calculate next yslice */
		ynew = 1e18;
		for (s = slices; s != NULL; s = s->next) {
			assert (s->vertex->next);
			assert (s->vertex->next->y > yslice);
			if (s->vertex->next->y < ynew) ynew = s->vertex->next->y;
		}
		/* fixme: Keep svl pointers */
		if ((nflat) && (nflat->y < ynew)) ynew = nflat->y;
		nsvl = csvl;
		while ((nsvl) && (nsvl->vertex->y == yslice)) {
			nsvl = nsvl->next;
		}
		if ((nsvl) && (nsvl->vertex->y < ynew)) ynew = nsvl->vertex->y;
		assert (ynew > yslice);
		yslice = ynew;
		/* Stretch existing slices to new position */
		slices = nr_svl_slice_stretch_list (slices, yslice);
		CHECK_SLICES (slices, yslice, "STRETCH", 0, 1, 1);
		/* Advance svl counters */
		if (lsvl) {
			lsvl->next = csvl;
		} else {
			svl = csvl;
		}
		while (csvl && csvl != nsvl) {
			lsvl = csvl;
			csvl = csvl->next;
		}
	}
	if (nflat) nr_flat_free_list (nflat);

	return svl;
}

static NRSVL *
nr_svl_slice_break (NRSVLSlice *s, double x, double y, NRSVL *svl)
{
	NRVertex *newvx;
	NRSVL *newsvl;

	if (s->vertex->y < y) {
		/* Mid-segment intersection */
		/* Create continuation svl */
		newvx = nr_vertex_new_xy (x, y);
		newvx->next = s->vertex->next;
		newsvl = nr_svl_new_vertex_wind (newvx, s->svl->dir);
		assert (newsvl->vertex->y < newsvl->vertex->next->y);
		/* Trim starting svl */
		newvx = nr_vertex_new_xy (x, y);
		s->vertex->next = newvx;
		nr_svl_calculate_bbox (s->svl);
		assert (s->svl->vertex->y < s->svl->vertex->next->y);
		/* Insert new SVL into main list */
		/* new svl slice is included by definition */
		svl = nr_svl_insert_sorted (svl, newsvl);
		/* fixme: We should maintain pointer to ssvl */
		/* New svl is inserted before nsvl by definition, so we can ignore management */
		/* Old svl will be excluded by definition, so we can shortcut */
		s->svl = newsvl;
		s->vertex = newsvl->vertex;
		/* s->x and s->y are correct by definition */
	} else if (s->vertex != s->svl->vertex) {
		assert (s->vertex->y == y);
		/* Inter-segment intersection */
		/* Winding may change here */
		/* Create continuation svl */
		newvx = nr_vertex_new_xy (x, y);
		newvx->next = s->vertex->next;
		newsvl = nr_svl_new_vertex_wind (newvx, s->svl->dir);
		assert (newsvl->vertex->y < newsvl->vertex->next->y);
		/* Trim starting svl */
		s->vertex->next = NULL;
		nr_svl_calculate_bbox (s->svl);
		assert (s->svl->vertex->y < s->svl->vertex->next->y);
		/* Insert new SVL into main list */
		/* new svl slice is included by definition */
		svl = nr_svl_insert_sorted (svl, newsvl);
		/* fixme: We should maintain pointer to ssvl */
		/* New svl is inserted before nsvl by definition, so we can ignore management */
		/* Old svl will be excluded by definition, so we can shortcut */
		s->svl = newsvl;
		s->vertex = newsvl->vertex;
		/* s->x and s->y are correct by definition */
	}
	return svl;
}

static NRSVL *
nr_svl_slice_break_y_and_continue_x (NRSVLSlice *s, double y, double x, NRSVL *svl, double ytest, NRFlat **flats)
{
	NRVertex *newvx;
	NRSVL *newsvl;

	assert (y > s->y);
	assert (y > s->vertex->y);
	assert (y <= s->vertex->next->y);

	if (y < s->vertex->next->y) {
		double dx, dy;
		/* Mid-segment intersection */
		/* Create continuation svl */
		newvx = nr_vertex_new_xy (x, y);
		newvx->next = s->vertex->next;
		newsvl = nr_svl_new_vertex_wind (newvx, s->svl->dir);
		assert (newsvl->vertex->y < newsvl->vertex->next->y);
		assert (newsvl->vertex->y > s->y);
		assert (newsvl->vertex->y > ytest);
		/* Trim starting svl */
		dx = s->vertex->next->x - s->vertex->x;
		dy = s->vertex->next->y - s->vertex->y;
		newvx = nr_vertex_new_xy (s->vertex->x + (y - s->vertex->y) * dx / dy, y);

		/* Set the new starting point */
		if (newvx->x != x) {
			NRFlat *f;
			double x0, x1;
			x0 = MIN (x, newvx->x);
			x1 = MAX (x, newvx->x);
			f = nr_flat_new_full (y, x0, x1);
			*flats = nr_flat_insert_sorted (*flats, f);
		}

		s->vertex->next = newvx;
		nr_svl_calculate_bbox (s->svl);
		assert (s->svl->vertex->y < s->svl->vertex->next->y);
		/* Insert new SVL into list */
		svl = nr_svl_insert_sorted (svl, newsvl);
		assert (svl);
		assert (s->y >= s->vertex->y);
	} else if (s->vertex->next->next) {

		/* Set the new starting point */
		if (s->vertex->next->x != x) {
			NRFlat *f;
			double x0, x1;
			x0 = MIN (x, s->vertex->next->x);
			x1 = MAX (x, s->vertex->next->x);
			f = nr_flat_new_full (y, x0, x1);
			*flats = nr_flat_insert_sorted (*flats, f);
		}

		/* Create continuation svl */
		newvx = nr_vertex_new_xy (x, y);
		newvx->next = s->vertex->next->next;
		newsvl = nr_svl_new_vertex_wind (newvx, s->svl->dir);
		assert (newsvl->vertex->y < newsvl->vertex->next->y);
		assert (newsvl->vertex->y > s->y);
		assert (newsvl->vertex->y > ytest);
		/* Trim starting svl */
		s->vertex->next->next = NULL;
		nr_svl_calculate_bbox (s->svl);
		assert (s->svl->vertex->y < s->svl->vertex->next->y);
		/* Insert new SVL into list */
		svl = nr_svl_insert_sorted (svl, newsvl);
		assert (svl);
		assert (s->y >= s->vertex->y);
	} else {
		/* Still have to place flat */
		if (s->vertex->next->x != x) {
			NRFlat *f;
			double x0, x1;
			x0 = MIN (x, s->vertex->next->x);
			x1 = MAX (x, s->vertex->next->x);
			f = nr_flat_new_full (y, x0, x1);
			*flats = nr_flat_insert_sorted (*flats, f);
		}
	}
	return svl;
}

#if 0
static void
nr_svl_slice_ensure_vertex_at (NRSVLSlice *s, NRCoord x, NRCoord y)
{
	/* Invariant 1 */
	assert (y >= s->y);
	/* Invariant 2 */
	assert (y >= s->vertex->y);
	/* Invariant 3 */
	assert (y <= s->vertex->next->y);

	if (y == s->y) {
		s->x = x;
	}
	if (y == s->vertex->y) {
		s->vertex->x = x;
	} else if (y < s->vertex->next->y) {
		NRVertex *nvx;
		nvx = nr_vertex_new_xy (x, y);
		nvx->next = s->vertex->next;
		s->vertex->next = nvx;
	} else {
		s->vertex->next->x = x;
	}
	/* We expect bbox to be correct (x0 <= x <= x1) */
}
#endif

/*
 * Memory management stuff follows (remember goals?)
 */

/* Slices */

#define NR_SLICE_ALLOC_SIZE 32
static NRSVLSlice * ffslice = NULL;

NRSVLSlice *
nr_svl_slice_new (NRSVL * svl, NRCoord y)
{
	NRSVLSlice * s;
	NRVertex * v;

	assert (svl);
	assert (svl->vertex);
	/* fixme: We try snapped slices - not sure, whether correct */
	assert (y == NR_COORD_SNAP (y));
	/* Slices startpoints are included, endpoints excluded */
	/* g_return_val_if_fail (y >= svl->bbox.y0, NULL); */
	/* g_return_val_if_fail (y < svl->bbox.y1, NULL); */

	s = ffslice;

	if (s == NULL) {
		int i;
		s = nr_new (NRSVLSlice, NR_SLICE_ALLOC_SIZE);
		for (i = 1; i < (NR_SLICE_ALLOC_SIZE - 1); i++) s[i].next = &s[i + 1];
		s[NR_SLICE_ALLOC_SIZE - 1].next = NULL;
		ffslice = s + 1;
	} else {
		ffslice = s->next;
	}

#if 0
	s->prev = NULL;
#endif
	s->next = NULL;
	s->svl = svl;

	v = svl->vertex;
	while ((v->next) && (v->next->y <= y)) v = v->next;
	assert (v->next);

	s->vertex = v;
	if (v->y == y) {
		s->x = v->x;
	} else {
		s->x = NR_COORD_SNAP (v->x + (v->next->x - v->x) * (y - v->y) / (v->next->y - v->y));
	}
	s->y = y;

	return s;
}

void
nr_svl_slice_free_one (NRSVLSlice * slice)
{
	slice->next = ffslice;
#if 0
	if (ffslice) ffslice->prev = slice;
#endif
	ffslice = slice;
#if 0
	slice->prev = NULL;
#endif
}

#if 0
void
nr_svl_slice_free_list (NRSVLSlice * slice)
{
	NRSVLSlice * l;

	if (!slice) return;

	for (l = slice; l->next != NULL; l = l->next);

	l->next = ffslice;
#if 0
	if (ffslice) ffslice->prev = l;
#endif
	ffslice = slice;
#if 0
	slice->prev = NULL;
#endif
}
#endif

NRSVLSlice *
nr_svl_slice_insert_sorted (NRSVLSlice * start, NRSVLSlice * slice)
{
	NRSVLSlice * s, * l;

	assert (start != slice);

	if (!start) {
		slice->next = NULL;
		return slice;
	}
	if (!slice) return start;

	if (nr_svl_slice_compare (slice, start) <= 0) {
		slice->next = start;
		return slice;
	}

	s = start;
	for (l = start->next; l != NULL; l = l->next) {
		if (nr_svl_slice_compare (slice, l) <= 0) {
			assert (l != slice);
			slice->next = l;
			assert (slice != s);
			s->next = slice;
			return start;
		}
		s = l;
	}

	slice->next = NULL;
	assert (slice != s);
	s->next = slice;

	return start;
}

NRSVLSlice *
nr_svl_slice_stretch_list (NRSVLSlice * slices, NRCoord y)
{
	NRSVLSlice * p, * s;

	/* fixme: We try snapped slices - not sure, whether correct */
	assert (y == NR_COORD_SNAP (y));

	p = NULL;
	s = slices;

	while (s) {
		if (s->svl->bbox.y1 <= y) {
			/* Remove exhausted slice */
			if (p) {
				assert (s->next != p);
				p->next = s->next;
				nr_svl_slice_free_one (s);
				s = p->next;
			} else {
				slices = s->next;
				nr_svl_slice_free_one (s);
				s = slices;
			}
		} else {
			NRVertex * v;
			/* Stretch slice */
			v = s->vertex;
			while ((v->next) && (v->next->y <= y)) v = v->next;
			assert (v->next);

			s->vertex = v;
			if (v->y == y) {
				s->x = v->x;
			} else {
#if 0
				s->x = NR_COORD_SNAP (v->x + (v->next->x - v->x) * (y - v->y) / (v->next->y - v->y));
#else
				s->x = v->x + (double) (v->next->x - v->x) * (y - v->y) / (v->next->y - v->y);
#endif
			}
			s->y = y;
			p = s;
			s = s->next;
		}
	}

	return slices;
}

static int
nr_svl_slice_compare (NRSVLSlice *l, NRSVLSlice *r)
{
	double ldx, ldy, rdx, rdy;
	double d;

	assert (l->y == r->y);
	assert (l->vertex->next->y > l->y);
	assert (r->vertex->next->y > r->y);

	if (l->x < r->x) return -1;
	if (l->x > r->x) return 1;

	/* Y is same, X is same, checking for line orders */
	ldx = l->vertex->next->x - l->vertex->x;
	ldy = l->vertex->next->y - l->y;
	rdx = r->vertex->next->x - r->vertex->x;
	rdy = r->vertex->next->y - r->y;

	d = ldx * rdy - ldy * rdx;
	/* fixme: test almost zero cases */
	if (d < 0) return -1;
	if (d > 0) return 1;
	return 0;
}

static double
nr_vertex_segment_distance2 (NRVertex *vx, NRVertex *seg)
{
	double Ax, Ay, Bx, By, Px, Py;
	double Dx, Dy, s;
	double dist2;
	Ax = seg->x;
	Ay = seg->y;
	Bx = seg->next->x;
	By = seg->next->y;
	Px = vx->x;
	Py = vx->y;
	Dx = Bx - Ax;
	Dy = By - Ay;
	s = ((Px - Ax) * Dx + (Py - Ay) * Dy) / (Dx * Dx + Dy * Dy);
	if (s <= 0.0) {
		dist2 = (Px - Ax) * (Px - Ax) + (Py - Ay) * (Py - Ay);
	} else if (s >= 1.0) {
		dist2 = (Px - Bx) * (Px - Bx) + (Py - By) * (Py - By);
	} else {
		double Qx, Qy;
		Qx = Ax + s * Dx;
		Qy = Ay + s * Dy;
		dist2 = (Px - Qx) * (Px - Qx) + (Py - Qy) * (Py - Qy);
	}
	return dist2;
}

static double
nr_segment_intersection (NRVertex *s0, NRVertex *s1, double *x, double *y)
{
	double xba, yba, xdc, ydc, xac, yac;
	double d, numr, nums, r, s;
	double dr, ds, d0_2, d1_2, d_2;

	xba = s0->next->x - s0->x;
	yba = s0->next->y - s0->y;
	xdc = s1->next->x - s1->x;
	ydc = s1->next->y - s1->y;
	d = xba * ydc - yba * xdc;
 
	/* Check for parallel */
	if (fabs (d) < NR_EPSILON_F) return -NR_HUGE_F;

	xac = s0->x - s1->x;
	yac = s0->y - s1->y;

	numr = yac * xdc - xac * ydc;
	nums = yac * xba - xac * yba;
	r = numr / d;
	s = nums / d;
	*x = s0->x + r * xba;
	*y = s0->y + r * yba;

	dr = 0.0;
	if (r < 0.0) dr = -r;
	if (r > 1.0) dr = r - 1.0;
	d0_2 = dr * (xba * xba + yba * yba);
	ds = 0.0;
	if (s < 0.0) ds = -s;
	if (s > 1.0) ds = s - 1.0;
	d1_2 = ds * (xdc * xdc + ydc * ydc);

	d_2 = MAX (d0_2, d1_2);

	return sqrt (d_2);
}

/*
 * Test, whether vertex can be considered to be lying on line
 */

#if 0
static unsigned int
nr_test_point_line (NRVertex *a, NRVertex *b, NRCoord cx, NRCoord cy)
{
	float xba, yba, xac, yac;
	float n;

	/*
	 * L = sqrt (xba * xba + yba * yba)
	 * n = yac * xba - xac * yba
	 * s = n / (L * L)
	 * d = s * L
	 *
	 * We test for d < TOLERANCE
	 * d * d < TOLERANCE * TOLERANCE
	 * s * s * L * L < TOLERANCE * TOLERANCE
	 * n * n / (L * L) < TOLERANCE * TOLERANCE
	 * n * n < TOLERANCE * TOLERANCE * L * L
	 *
	 */

	xba = b->x - a->x;
	yba = b->y - a->y;
	xac = a->x - cx;
	yac = a->y - cy;

	n = yac * xba - xac * yba;

	return (n * n) < (2 * NR_COORD_TOLERANCE * NR_COORD_TOLERANCE * (xba * xba + yba * yba));
}
#endif
