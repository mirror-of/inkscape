#define __CURVE_C__

/*
 * Wrapper around ArtBpath
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

#include <math.h>
#include <string.h>
#include <libart_lgpl/art_misc.h>
#include "curve.h"
#include <libnr/nr-types.h>

#define SP_CURVE_LENSTEP 32

static gboolean sp_bpath_good (ArtBpath * bpath);
static ArtBpath *sp_bpath_clean (ArtBpath *bpath);
ArtBpath * sp_bpath_check_subpath (ArtBpath * bpath);
static gint sp_bpath_length (ArtBpath * bpath);
static gboolean sp_bpath_closed (ArtBpath * bpath);

/* Constructors */

SPCurve *
sp_curve_new (void)
{
	SPCurve * curve;

	curve = sp_curve_new_sized (SP_CURVE_LENSTEP);

	return curve;
}

SPCurve *
sp_curve_new_sized (gint length)
{
	g_return_val_if_fail (length > 0, NULL);

	SPCurve * curve = g_new (SPCurve, 1);

	curve->refcount = 1;
	curve->bpath = art_new (ArtBpath, length);
	curve->bpath->code = ART_END;
	curve->end = 0;
	curve->length = length;
	curve->substart = 0;
	curve->sbpath = FALSE;
	curve->hascpt = FALSE;
	curve->posset = FALSE;
	curve->moving = FALSE;
	curve->closed = FALSE;

	return curve;
}

SPCurve *
sp_curve_new_from_bpath (ArtBpath *bpath)
{
	g_return_val_if_fail (bpath != NULL, NULL);

	if (!sp_bpath_good (bpath)) {
		ArtBpath *new_bpath = sp_bpath_clean (bpath);
		g_return_val_if_fail (new_bpath != NULL, NULL);
		art_free (bpath);
		bpath = new_bpath;
	}

	SPCurve *curve = g_new (SPCurve, 1);

	curve->refcount = 1;
	curve->bpath = bpath;
	curve->length = sp_bpath_length (bpath);
	curve->end = curve->length - 1;
	gint i = curve->end;
	for (; i > 0; i--)
		if ((curve->bpath[i].code == ART_MOVETO) || 
		    (curve->bpath[i].code == ART_MOVETO_OPEN))
			break;
	curve->substart = i;
	curve->sbpath = FALSE;
	curve->hascpt = FALSE;
	curve->posset = FALSE;
	curve->moving = FALSE;
	curve->closed = sp_bpath_closed (bpath);

	return curve;
}

SPCurve *
sp_curve_new_from_static_bpath (ArtBpath * bpath)
{
	g_return_val_if_fail (bpath != NULL, NULL);

	gboolean sbpath;
	if (!sp_bpath_good (bpath)) {
		ArtBpath *new_bpath = sp_bpath_clean (bpath);
		g_return_val_if_fail (new_bpath != NULL, NULL);
		sbpath = FALSE;
		bpath = new_bpath;
	} else {
		sbpath = TRUE;
	}

	SPCurve *curve = g_new (SPCurve, 1);

	curve->refcount = 1;
	curve->bpath = bpath;
	curve->length = sp_bpath_length (bpath);
	curve->end = curve->length - 1;
	gint i = curve->end;
	for (; i > 0; i--)
		if ((curve->bpath[i].code == ART_MOVETO) || 
		    (curve->bpath[i].code == ART_MOVETO_OPEN)) 
			break;
	curve->substart = i;
	curve->sbpath = sbpath;
	curve->hascpt = FALSE;
	curve->posset = FALSE;
	curve->moving = FALSE;
	curve->closed = sp_bpath_closed (bpath);

	return curve;
}

SPCurve *
sp_curve_new_from_foreign_bpath (ArtBpath * bpath)
{
	g_return_val_if_fail (bpath != NULL, NULL);

	ArtBpath *new_bpath;
	if (!sp_bpath_good (bpath)) {
		new_bpath = sp_bpath_clean (bpath);
		g_return_val_if_fail (new_bpath != NULL, NULL);
	} else {
		gint len = sp_bpath_length (bpath);
		new_bpath = art_new (ArtBpath, len);
		memcpy (new_bpath, bpath, len * sizeof (ArtBpath));
	}

	SPCurve *curve = sp_curve_new_from_bpath (new_bpath);

	if (!curve)
		art_free (new_bpath);

	return curve;
}

SPCurve *
sp_curve_ref (SPCurve * curve)
/* should this be shared with other refcounting code? */
{
	g_return_val_if_fail (curve != NULL, NULL);

	curve->refcount += 1;

	return curve;
}

SPCurve *
sp_curve_unref (SPCurve * curve)
/* should this be shared with other refcounting code? */
{
	g_return_val_if_fail (curve != NULL, NULL);

	curve->refcount -= 1;

	if (curve->refcount < 1) {
		if ((!curve->sbpath) && (curve->bpath)) art_free (curve->bpath);
		g_free (curve);
	}

	return NULL;
}


void
sp_curve_finish (SPCurve * curve)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (curve->sbpath);

	if (curve->end > 0) {
		ArtBpath * bp = curve->bpath + curve->end - 1;
		if (bp->code == ART_LINETO) {
			curve->end--;
			bp->code = ART_END;
		}
	}

	if (curve->end < (curve->length - 1)) {
		curve->bpath = art_renew (curve->bpath, ArtBpath, curve->end);
	}

	curve->hascpt = FALSE;
	curve->posset = FALSE;
	curve->moving = FALSE;
}

void
sp_curve_ensure_space (SPCurve * curve, gint space)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (space > 0);

	if (curve->end + space < curve->length)
		return;

	if (space < SP_CURVE_LENSTEP)
		space = SP_CURVE_LENSTEP;

	curve->bpath = art_renew (curve->bpath, ArtBpath, curve->length + space);

	curve->length += space;
}

SPCurve *
sp_curve_copy (SPCurve * curve)
{
	g_return_val_if_fail (curve != NULL, NULL);

	return  sp_curve_new_from_foreign_bpath (curve->bpath);
}

SPCurve *
sp_curve_concat (const GSList * list)
{
	g_return_val_if_fail (list != NULL, NULL);

	gint length = 0;

	for (const GSList * l = list; l != NULL; l = l->next) {
		SPCurve * c = (SPCurve *) l->data;
		length += c->end;
	}

	SPCurve * new_curve = sp_curve_new_sized (length + 1);

	ArtBpath * bp = new_curve->bpath;

	for (const GSList * l = list; l != NULL; l = l->next) {
		SPCurve *c = (SPCurve *) l->data;
		memcpy (bp, c->bpath, c->end * sizeof (ArtBpath));
		bp += c->end;
	}

	bp->code = ART_END;

	new_curve->end = length;
	gint i;
	for (i = new_curve->end; i > 0; i--)
		if ((new_curve->bpath[i].code == ART_MOVETO) || 
		    (new_curve->bpath[i].code == ART_MOVETO_OPEN))
			break;
	
	new_curve->substart = i;

	return new_curve;
}

GSList *
sp_curve_split (SPCurve * curve)
{
	g_return_val_if_fail (curve != NULL, NULL);

	gint p = 0;
	GSList *l = NULL;

	while (p < curve->end) {
		gint i = 1;
		while ((curve->bpath[p + i].code == ART_LINETO) || 
		       (curve->bpath[p + i].code == ART_CURVETO))
			i++;
		SPCurve * new_curve = sp_curve_new_sized (i + 1);
		memcpy (new_curve->bpath, curve->bpath + p, i * sizeof (ArtBpath));
		new_curve->end = i;
		new_curve->bpath[i].code = ART_END;
		new_curve->substart = 0;
		new_curve->closed = (new_curve->bpath->code == ART_MOVETO);
		new_curve->hascpt = (new_curve->bpath->code == ART_MOVETO_OPEN);
		l = g_slist_append (l, new_curve);
		p += i;
	}

	return l;
}

SPCurve *
sp_curve_transform (SPCurve *curve, const gdouble t[])
{
	g_return_val_if_fail (curve != NULL, NULL);
	g_return_val_if_fail (!curve->sbpath, NULL);
	g_return_val_if_fail (t != NULL, curve);

	for (gint i = 0; i < curve->end; i++) {
		ArtBpath *p = curve->bpath + i;
		switch (p->code) {
		case ART_MOVETO:
		case ART_MOVETO_OPEN:
		case ART_LINETO:
			p->x3 = t[0] * p->x3 + t[2] * p->y3 + t[4];
			p->y3 = t[1] * p->x3 + t[3] * p->y3 + t[5];
			break;
		case ART_CURVETO:
			p->x1 = t[0] * p->x1 + t[2] * p->y1 + t[4];
			p->y1 = t[1] * p->x1 + t[3] * p->y1 + t[5];
			p->x2 = t[0] * p->x2 + t[2] * p->y2 + t[4];
			p->y2 = t[1] * p->x2 + t[3] * p->y2 + t[5];
			p->x3 = t[0] * p->x3 + t[2] * p->y3 + t[4];
			p->y3 = t[1] * p->x3 + t[3] * p->y3 + t[5];
			break;
		default:
			g_warning ("Illegal pathcode %d", p->code);
			return NULL;
			break;
		}
	}

	return curve;
}

/* Methods */

void
sp_curve_reset (SPCurve * curve)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (!curve->sbpath);

	curve->bpath->code = ART_END;
	curve->end = 0;
	curve->substart = 0;
	curve->hascpt = FALSE;
	curve->posset = FALSE;
	curve->moving = FALSE;
	curve->closed = FALSE;
}

/* Several consecutive movetos are ALLOWED */

void
sp_curve_moveto (SPCurve * curve, NR::Point const &p)
{
  sp_curve_moveto(curve, p[NR::X], p[NR::Y]);
}

void
sp_curve_moveto (SPCurve * curve, gdouble x, gdouble y)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (!curve->sbpath);
	g_return_if_fail (!curve->moving);

	curve->substart = curve->end;
	curve->hascpt = TRUE;
	curve->posset = TRUE;
	curve->x = x;
	curve->y = y;
}

void
sp_curve_lineto (SPCurve * curve, NR::Point const &p)
{
  sp_curve_lineto(curve, p[NR::X], p[NR::Y]);
}

void
sp_curve_lineto (SPCurve * curve, gdouble x, gdouble y)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (!curve->sbpath);
	g_return_if_fail (curve->hascpt);

	if (curve->moving) {
		/* fix endpoint */
		g_return_if_fail (!curve->posset);
		g_return_if_fail (curve->end > 1);
		ArtBpath * bp = curve->bpath + curve->end - 1;
		g_return_if_fail (bp->code == ART_LINETO);
		bp->x3 = x;
		bp->y3 = y;
		curve->moving = FALSE;
		return;
	}

	if (curve->posset) {
		/* start a new segment */
		sp_curve_ensure_space (curve, 2);
		ArtBpath * bp = curve->bpath + curve->end;
		bp->code = ART_MOVETO_OPEN;
		bp->x3 = curve->x;
		bp->y3 = curve->y;
		bp++;
		bp->code = ART_LINETO;
		bp->x3 = x;
		bp->y3 = y;
		bp++;
		bp->code = ART_END;
		curve->end += 2;
		curve->posset = FALSE;
		curve->closed = FALSE;
		return;
	}

	/* add line */

	g_return_if_fail (curve->end > 1);
	sp_curve_ensure_space (curve, 1);
	ArtBpath * bp = curve->bpath + curve->end;
	bp->code = ART_LINETO;
	bp->x3 = x;
	bp->y3 = y;
	bp++;
	bp->code = ART_END;
	curve->end++;
}

void
sp_curve_lineto_moving (SPCurve * curve, gdouble x, gdouble y)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (!curve->sbpath);
	g_return_if_fail (curve->hascpt);

	if (curve->moving) {
		/* change endpoint */
		g_return_if_fail (!curve->posset);
		g_return_if_fail (curve->end > 1);
		ArtBpath * bp = curve->bpath + curve->end - 1;
		g_return_if_fail (bp->code == ART_LINETO);
		bp->x3 = x;
		bp->y3 = y;
		return;
	}

	if (curve->posset) {
		/* start a new segment */
		sp_curve_ensure_space (curve, 2);
		ArtBpath * bp = curve->bpath + curve->end;
		bp->code = ART_MOVETO_OPEN;
		bp->x3 = curve->x;
		bp->y3 = curve->y;
		bp++;
		bp->code = ART_LINETO;
		bp->x3 = x;
		bp->y3 = y;
		bp++;
		bp->code = ART_END;
		curve->end += 2;
		curve->posset = FALSE;
		curve->moving = TRUE;
		curve->closed = FALSE;
		return;
	}

	/* add line */

	g_return_if_fail (curve->end > 1);
	sp_curve_ensure_space (curve, 1);
	ArtBpath * bp = curve->bpath + curve->end;
	bp->code = ART_LINETO;
	bp->x3 = x;
	bp->y3 = y;
	bp++;
	bp->code = ART_END;
	curve->end++;
	curve->moving = TRUE;
}

void
sp_curve_curveto (SPCurve * curve, NR::Point const &p0, NR::Point const &p1, NR::Point const &p2)
{
  using NR::X;
  using NR::Y;
  sp_curve_curveto(curve,
		   p0[X], p0[Y],
		   p1[X], p1[Y],
		   p2[X], p2[Y]);
}

void
sp_curve_curveto (SPCurve * curve, gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (!curve->sbpath);
	g_return_if_fail (curve->hascpt);
	g_return_if_fail (!curve->moving);

	if (curve->posset) {
		/* start a new segment */
		sp_curve_ensure_space (curve, 2);
		ArtBpath * bp = curve->bpath + curve->end;
		bp->code = ART_MOVETO_OPEN;
		bp->x3 = curve->x;
		bp->y3 = curve->y;
		bp++;
		bp->code = ART_CURVETO;
		bp->x1 = x0;
		bp->y1 = y0;
		bp->x2 = x1;
		bp->y2 = y1;
		bp->x3 = x2;
		bp->y3 = y2;
		bp++;
		bp->code = ART_END;
		curve->end += 2;
		curve->posset = FALSE;
		curve->closed = FALSE;
		return;
	}

	/* add curve */

	g_return_if_fail (curve->end > 1);
	sp_curve_ensure_space (curve, 1);
	ArtBpath * bp = curve->bpath + curve->end;
	bp->code = ART_CURVETO;
	bp->x1 = x0;
	bp->y1 = y0;
	bp->x2 = x1;
	bp->y2 = y1;
	bp->x3 = x2;
	bp->y3 = y2;
	bp++;
	bp->code = ART_END;
	curve->end++;
}

void
sp_curve_closepath (SPCurve * curve)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (!curve->sbpath);
	g_return_if_fail (curve->hascpt);
	g_return_if_fail (!curve->posset);
	g_return_if_fail (!curve->moving);
	g_return_if_fail (!curve->closed);
	/* We need at last M + C + E */
	g_return_if_fail (curve->end - curve->substart > 1);

	ArtBpath *bs = curve->bpath + curve->substart;
	ArtBpath *be = curve->bpath + curve->end - 1;

	if ((bs->x3 != be->x3) || (bs->y3 != be->y3)) {
		sp_curve_lineto (curve, bs->x3, bs->y3);
	}

	bs = curve->bpath + curve->substart;
	be = curve->bpath + curve->end - 1;

	bs->code = ART_MOVETO;

	curve->closed = TRUE;

	for (bs = curve->bpath; bs->code != ART_END; bs++)
		if (bs->code == ART_MOVETO_OPEN) curve->closed = FALSE;

	curve->hascpt = FALSE;
}

void
sp_curve_closepath_current (SPCurve * curve)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (!curve->sbpath);
	g_return_if_fail (curve->hascpt);
	g_return_if_fail (!curve->posset);
	g_return_if_fail (!curve->closed);
	/* We need at last M + L + L + E */
	g_return_if_fail (curve->end - curve->substart > 2);
	
	{
		ArtBpath * bs = curve->bpath + curve->substart;
		ArtBpath * be = curve->bpath + curve->end - 1;
		
		be->x3 = bs->x3;
		be->y3 = bs->y3;
		
		bs->code = ART_MOVETO;
	}
	curve->closed = TRUE;

	for (ArtBpath *bp = curve->bpath; bp->code != ART_END; bp++)
		if (bp->code == ART_MOVETO_OPEN)
			curve->closed = FALSE;

	curve->hascpt = FALSE;
	curve->moving = FALSE;
}

gboolean
sp_curve_empty (SPCurve * curve)
{
	g_return_val_if_fail (curve != NULL, TRUE);

	return (curve->bpath->code == ART_END);
}

ArtBpath *
sp_curve_last_bpath (SPCurve const *curve)
{
	g_return_val_if_fail (curve != NULL, NULL);

	if (curve->end == 0) return NULL;

	return curve->bpath + curve->end - 1;
}

ArtBpath *
sp_curve_first_bpath (SPCurve const *curve)
{
	g_return_val_if_fail (curve != NULL, NULL);

	if (curve->end == 0) return NULL;

	return curve->bpath;
}

SPCurve *
sp_curve_reverse (SPCurve *curve)
{
  /* We need at last M + C + E */
  g_return_val_if_fail (curve->end - curve->substart > 1, NULL);

  ArtBpath *bs = curve->bpath + curve->substart;
  ArtBpath *be = curve->bpath + curve->end - 1;

  SPCurve  *new_curve = sp_curve_new_sized (curve->length);

  g_assert (bs->code == ART_MOVETO_OPEN || bs->code == ART_MOVETO);
  g_assert ((be+1)->code == ART_END);

  sp_curve_moveto (new_curve, be->x3, be->y3);

  for (ArtBpath *bp = be; bp != bs; bp--)
    {
      switch (bp->code)
        {
        case ART_MOVETO_OPEN:
          sp_curve_moveto (new_curve, (bp-1)->x3, (bp-1)->y3);
          break;
        case ART_MOVETO:
          sp_curve_moveto (new_curve, (bp-1)->x3, (bp-1)->y3);
          break;
        case ART_LINETO:
          sp_curve_lineto (new_curve, (bp-1)->x3, (bp-1)->y3);
          break;
        case ART_CURVETO:
          sp_curve_curveto (new_curve, bp->x2, bp->y2, bp->x1, bp->y1, (bp-1)->x3, (bp-1)->y3);
          break;
        case ART_END:
          g_assert_not_reached ();
        }
    }

  return new_curve;
}

void
sp_curve_append (SPCurve *curve,
                 SPCurve const *curve2,
                 gboolean use_lineto)
{
	g_return_if_fail (curve != NULL);
	g_return_if_fail (curve2 != NULL);

	if (curve2->end < 1)
		return;

	ArtBpath const *bs = curve2->bpath;

	gboolean closed = curve->closed;

	for (ArtBpath const *bp = bs; bp->code != ART_END; bp++) {
		switch (bp->code) {
		case ART_MOVETO_OPEN:
			if (use_lineto && curve->hascpt) {
				sp_curve_lineto (curve, bp->x3, bp->y3);
				use_lineto = FALSE;
			} else {
				if (closed) sp_curve_closepath (curve);
				sp_curve_moveto (curve, bp->x3, bp->y3);
			}
			closed = FALSE;
			break;
		case ART_MOVETO:
			if (use_lineto && curve->hascpt) {
				sp_curve_lineto (curve, bp->x3, bp->y3);
				use_lineto = FALSE;
			} else {
				if (closed) sp_curve_closepath (curve);
				sp_curve_moveto (curve, bp->x3, bp->y3);
			}
			closed = TRUE;
			break;
		case ART_LINETO:
			sp_curve_lineto (curve, bp->x3, bp->y3);
			break;
		case ART_CURVETO:
			sp_curve_curveto (curve, bp->x1, bp->y1, bp->x2, bp->y2, bp->x3, bp->y3);
			break;
		case ART_END:
			g_assert_not_reached ();
		}
	}

	if (closed) sp_curve_closepath (curve);
}

SPCurve *
sp_curve_append_continuous (SPCurve *c0, SPCurve const *c1, gdouble tolerance)
{
	g_return_val_if_fail (c0 != NULL, NULL);
	g_return_val_if_fail (c1 != NULL, NULL);
	g_return_val_if_fail (!c0->closed, NULL);
	g_return_val_if_fail (!c1->closed, NULL);

	if (c1->end < 1)
		return c0;

	ArtBpath *be = sp_curve_last_bpath (c0);
	if (be) {
		ArtBpath const *bs = sp_curve_first_bpath (c1);
		if (bs && (fabs (bs->x3 - be->x3) <= tolerance) && (fabs (bs->y3 - be->y3) <= tolerance)) {
			/* fixme: Strictly we mess in case of multisegment mixed open/close curves */
			gboolean closed = FALSE;
			for (bs = bs + 1; bs->code != ART_END; bs++) {
				switch (bs->code) {
				case ART_MOVETO_OPEN:
					if (closed) sp_curve_closepath (c0);
					sp_curve_moveto (c0, bs->x3, bs->y3);
					closed = FALSE;
					break;
				case ART_MOVETO:
					if (closed) sp_curve_closepath (c0);
					sp_curve_moveto (c0, bs->x3, bs->y3);
					closed = TRUE;
					break;
				case ART_LINETO:
					sp_curve_lineto (c0, bs->x3, bs->y3);
					break;
				case ART_CURVETO:
					sp_curve_curveto (c0, bs->x1, bs->y1, bs->x2, bs->y2, bs->x3, bs->y3);
					break;
				case ART_END:
					g_assert_not_reached ();
				}
			}
		} else {
			sp_curve_append (c0, c1, TRUE);
		}
	} else {
		sp_curve_append (c0, c1, TRUE);
	}

	return c0;
}

void
sp_curve_backspace (SPCurve *curve)
{
	g_return_if_fail (curve != NULL);

	if (curve->end > 0) {
		curve->end -= 1;
		if (curve->end > 0) {
			ArtBpath *bp = curve->bpath + curve->end - 1;
			if ((bp->code == ART_MOVETO) || 
			    (bp->code == ART_MOVETO_OPEN)) {
				curve->hascpt = TRUE;
				curve->posset = TRUE;
				curve->closed = FALSE;
				curve->x = bp->x3;
				curve->y = bp->y3;
				curve->end -= 1;
			}
		}
		curve->bpath[curve->end].code = ART_END;
	}
}

/* Private methods */

static gboolean
sp_bpath_good (ArtBpath *bpath)
{
	g_return_val_if_fail (bpath != NULL, FALSE);

	if (bpath->code == ART_END) return TRUE;

	ArtBpath *bp = bpath;

	while (bp->code != ART_END) {
		bp = sp_bpath_check_subpath (bp);
		if (bp == NULL)
			return FALSE;
	}

	return TRUE;
}

static ArtBpath *
sp_bpath_clean (ArtBpath *bpath)
{
	ArtBpath *new_bpath = art_new (ArtBpath, sp_bpath_length(bpath));

	ArtBpath *bp = bpath;
	ArtBpath *np = new_bpath;

	while (bp->code != ART_END) {
		if (sp_bpath_check_subpath (bp)) {
			*np++ = *bp++;
			while ((bp->code == ART_LINETO) || 
			       (bp->code == ART_CURVETO))
				*np++ = *bp++;
		} else {
			bp++;
			while ((bp->code == ART_LINETO) || 
			       (bp->code == ART_CURVETO))
				bp++;
		}
	}

	if (np == new_bpath) {
		art_free (new_bpath);
		return NULL;
	}

	np->code = ART_END;
	np += 1;

	new_bpath = art_renew (new_bpath, ArtBpath, np - new_bpath);

	return new_bpath;
}

ArtBpath *
sp_bpath_check_subpath (ArtBpath * bpath)
{
	g_return_val_if_fail (bpath != NULL, NULL);

	gboolean closed;
	if (bpath->code == ART_MOVETO) {
		closed = TRUE;
	} else if (bpath->code == ART_MOVETO_OPEN) {
		closed = FALSE;
	} else {
		return NULL;
	}

	gint len = 0;
	gint i;
	for (i = 1; (bpath[i].code != ART_END) && (bpath[i].code != ART_MOVETO) && (bpath[i].code != ART_MOVETO_OPEN); i++) {
		switch (bpath[i].code) {
			case ART_LINETO:
			case ART_CURVETO:
				len++;
				break;
			default:
				return NULL;
		}
	}

	if (closed) {
		if (len < 1) return NULL;
		if ((bpath->x3 != bpath[i-1].x3) || (bpath->y3 != bpath[i-1].y3)) return NULL;
	} else {
		if (len < 1) return NULL;
	}

	return bpath + i;
}

static gint
sp_bpath_length (ArtBpath * bpath)
{
	g_return_val_if_fail (bpath != NULL, FALSE);

	gint l;
	for (l = 0; bpath[l].code != ART_END; l++) ;

	l++;

	return l;
}

static gboolean
sp_bpath_closed (ArtBpath * bpath)
{
	g_return_val_if_fail (bpath != NULL, FALSE);

	for (ArtBpath *bp = bpath; bp->code != ART_END; bp++)
		if (bp->code == ART_MOVETO_OPEN) return FALSE;

	return TRUE;
}
