#ifndef SEEN_DISPLAY_CURVE_H
#define SEEN_DISPLAY_CURVE_H

/*
 * Wrapper around NArtBpath
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL
 */

#include <glib/gtypes.h>
#include <glib/gslist.h>
#include <libnr/nr-path.h>
namespace NR { class Point; }

struct SPCurve {
    gint refcount;
    NArtBpath *bpath;
    gint end;		/* ART_END position */
    gint length;	/* Num allocated Bpaths */
    gint substart;	/* subpath start */
    gdouble x, y;	/* previous moveto position */
    bool sbpath : 1;	/* Bpath is static */
    bool hascpt : 1;	/* Currentpoint is defined */
    bool posset : 1;	/* Previous was moveto */
    bool moving : 1;	/* Bpath end is moving */
    bool closed : 1;	/* All subpaths are closed */
};

#define SP_CURVE_LENGTH(c) (((SPCurve const *)(c))->end)
#define SP_CURVE_BPATH(c) (((SPCurve const *)(c))->bpath)
#define SP_CURVE_SEGMENT(c,i) (((SPCurve const *)(c))->bpath + (i))

/* Constructors */

SPCurve *sp_curve_new();
SPCurve *sp_curve_new_sized(gint length);
SPCurve *sp_curve_new_from_bpath(NArtBpath *bpath);
SPCurve *sp_curve_new_from_static_bpath(NArtBpath *bpath);
SPCurve *sp_curve_new_from_foreign_bpath(NArtBpath const bpath[]);

SPCurve *sp_curve_ref(SPCurve *curve);
SPCurve *sp_curve_unref(SPCurve *curve);

void sp_curve_finish(SPCurve *curve);
void sp_curve_ensure_space(SPCurve *curve, gint space);
SPCurve *sp_curve_copy(SPCurve *curve);
SPCurve *sp_curve_concat(GSList const *list);
GSList *sp_curve_split(SPCurve *curve);
void sp_curve_transform(SPCurve *curve, NR::Matrix const &);
void sp_curve_transform(SPCurve *curve, NR::translate const &);
void sp_curve_stretch_endpoints(SPCurve *curve, NR::Point const &, NR::Point const &);

/* Methods */

void sp_curve_reset(SPCurve *curve);

void sp_curve_moveto(SPCurve *curve, NR::Point const &p);
void sp_curve_moveto(SPCurve *curve, gdouble x, gdouble y);
void sp_curve_lineto(SPCurve *curve, NR::Point const &p);
void sp_curve_lineto(SPCurve *curve, gdouble x, gdouble y);
void sp_curve_lineto_moving(SPCurve *curve, gdouble x, gdouble y);
void sp_curve_curveto(SPCurve *curve, NR::Point const &p0, NR::Point const &p1, NR::Point const &p2);
void sp_curve_curveto(SPCurve *curve, gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble x2, gdouble y2);
void sp_curve_closepath(SPCurve *curve);
void sp_curve_closepath_current(SPCurve *curve);

SPCurve *sp_curve_append_continuous(SPCurve *c0, SPCurve const *c1, gdouble tolerance);

#define sp_curve_is_empty sp_curve_empty
gboolean sp_curve_empty(SPCurve *curve);
NArtBpath *sp_curve_last_bpath(SPCurve const *curve);
NArtBpath *sp_curve_first_bpath(SPCurve const *curve);
NR::Point sp_curve_first_point(SPCurve const *curve);
NR::Point sp_curve_last_point(SPCurve const *curve);

void sp_curve_append(SPCurve *curve, SPCurve const *curve2, gboolean use_lineto);
SPCurve *sp_curve_reverse(SPCurve *curve);
void sp_curve_backspace(SPCurve *curve);


#endif /* !SEEN_DISPLAY_CURVE_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
