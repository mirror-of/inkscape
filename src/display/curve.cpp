#define __CURVE_C__

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

#include <math.h>
#include <string.h>
#include <glib/gmessages.h>

#include <display/curve.h>
#include <libnr/nr-point.h>
#include <libnr/nr-path.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-matrix-ops.h>
#include <libnr/nr-translate-ops.h>

#define SP_CURVE_LENSTEP 32

static bool sp_bpath_good(NArtBpath const bpath[]);
static NArtBpath *sp_bpath_clean(NArtBpath const bpath[]);
static NArtBpath const *sp_bpath_check_subpath(NArtBpath const bpath[]);
static unsigned sp_bpath_length(NArtBpath const bpath[]);
static bool sp_bpath_closed(NArtBpath const bpath[]);

/* Constructors */

SPCurve *
sp_curve_new()
{
    return sp_curve_new_sized(SP_CURVE_LENSTEP);
}

SPCurve *
sp_curve_new_sized(gint length)
{
    g_return_val_if_fail(length > 0, NULL);

    SPCurve *curve = g_new(SPCurve, 1);

    curve->refcount = 1;
    curve->bpath = nr_new(NArtBpath, length);
    curve->bpath->code = NR_END;
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

/**
* \return new SPCurve, or NULL if the curve was not created for some reason.
*/
SPCurve *
sp_curve_new_from_bpath(NArtBpath *bpath)
{
    g_return_val_if_fail(bpath != NULL, NULL);

    if (!sp_bpath_good(bpath)) {
        NArtBpath *new_bpath = sp_bpath_clean(bpath);
        if (new_bpath == NULL) {
            return NULL;
        }
        nr_free(bpath);
        bpath = new_bpath;
    }

    SPCurve *curve = g_new(SPCurve, 1);

    curve->refcount = 1;
    curve->bpath = bpath;
    curve->length = sp_bpath_length(bpath);
    curve->end = curve->length - 1;
    gint i = curve->end;
    for (; i > 0; i--)
        if ((curve->bpath[i].code == NR_MOVETO) ||
            (curve->bpath[i].code == NR_MOVETO_OPEN))
            break;
    curve->substart = i;
    curve->sbpath = FALSE;
    curve->hascpt = FALSE;
    curve->posset = FALSE;
    curve->moving = FALSE;
    curve->closed = sp_bpath_closed(bpath);

    return curve;
}

SPCurve *
sp_curve_new_from_static_bpath(NArtBpath *bpath)
{
    g_return_val_if_fail(bpath != NULL, NULL);

    bool sbpath;
    if (!sp_bpath_good(bpath)) {
        NArtBpath *new_bpath = sp_bpath_clean(bpath);
        g_return_val_if_fail(new_bpath != NULL, NULL);
        sbpath = false;
        bpath = new_bpath;
    } else {
        sbpath = true;
    }

    SPCurve *curve = g_new(SPCurve, 1);

    curve->refcount = 1;
    curve->bpath = bpath;
    curve->length = sp_bpath_length(bpath);
    curve->end = curve->length - 1;
    gint i = curve->end;
    for (; i > 0; i--)
        if ((curve->bpath[i].code == NR_MOVETO) ||
            (curve->bpath[i].code == NR_MOVETO_OPEN))
            break;
    curve->substart = i;
    curve->sbpath = sbpath;
    curve->hascpt = FALSE;
    curve->posset = FALSE;
    curve->moving = FALSE;
    curve->closed = sp_bpath_closed(bpath);

    return curve;
}

SPCurve *sp_curve_new_from_foreign_bpath(NArtBpath const bpath[])
{
    g_return_val_if_fail(bpath != NULL, NULL);

    NArtBpath *new_bpath;
    if (!sp_bpath_good(bpath)) {
        new_bpath = sp_bpath_clean(bpath);
        g_return_val_if_fail(new_bpath != NULL, NULL);
    } else {
        unsigned const len = sp_bpath_length(bpath);
        new_bpath = nr_new(NArtBpath, len);
        memcpy(new_bpath, bpath, len * sizeof(NArtBpath));
    }

    SPCurve *curve = sp_curve_new_from_bpath(new_bpath);

    if (!curve)
        nr_free(new_bpath);

    return curve;
}

SPCurve *
sp_curve_ref(SPCurve *curve)
/* should this be shared with other refcounting code? */
{
    g_return_val_if_fail(curve != NULL, NULL);

    curve->refcount += 1;

    return curve;
}

SPCurve *
sp_curve_unref(SPCurve *curve)
/* should this be shared with other refcounting code? */
{
    g_return_val_if_fail(curve != NULL, NULL);

    curve->refcount -= 1;

    if (curve->refcount < 1) {
        if ((!curve->sbpath) && (curve->bpath)) {
            nr_free(curve->bpath);
        }
        g_free(curve);
    }

    return NULL;
}


void
sp_curve_finish(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(curve->sbpath);

    if (curve->end > 0) {
        NArtBpath *bp = curve->bpath + curve->end - 1;
        if (bp->code == NR_LINETO) {
            curve->end--;
            bp->code = NR_END;
        }
    }

    if (curve->end < (curve->length - 1)) {
        curve->bpath = nr_renew(curve->bpath, NArtBpath, curve->end);
    }

    curve->hascpt = FALSE;
    curve->posset = FALSE;
    curve->moving = FALSE;
}

void
sp_curve_ensure_space(SPCurve *curve, gint space)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(space > 0);

    if (curve->end + space < curve->length)
        return;

    if (space < SP_CURVE_LENSTEP)
        space = SP_CURVE_LENSTEP;

    curve->bpath = nr_renew(curve->bpath, NArtBpath, curve->length + space);

    curve->length += space;
}

SPCurve *
sp_curve_copy(SPCurve *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    return sp_curve_new_from_foreign_bpath(curve->bpath);
}

SPCurve *
sp_curve_concat(GSList const *list)
{
    g_return_val_if_fail(list != NULL, NULL);

    gint length = 0;

    for (GSList const *l = list; l != NULL; l = l->next) {
        SPCurve *c = (SPCurve *) l->data;
        length += c->end;
    }

    SPCurve *new_curve = sp_curve_new_sized(length + 1);

    NArtBpath *bp = new_curve->bpath;

    for (GSList const *l = list; l != NULL; l = l->next) {
        SPCurve *c = (SPCurve *) l->data;
        memcpy(bp, c->bpath, c->end * sizeof(NArtBpath));
        bp += c->end;
    }

    bp->code = NR_END;

    new_curve->end = length;
    gint i;
    for (i = new_curve->end; i > 0; i--) {
        if ((new_curve->bpath[i].code == NR_MOVETO)     ||
            (new_curve->bpath[i].code == NR_MOVETO_OPEN)  )
            break;
    }

    new_curve->substart = i;

    return new_curve;
}

GSList *
sp_curve_split(SPCurve *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    gint p = 0;
    GSList *l = NULL;

    while (p < curve->end) {
        gint i = 1;
        while ((curve->bpath[p + i].code == NR_LINETO) ||
               (curve->bpath[p + i].code == NR_CURVETO))
            i++;
        SPCurve *new_curve = sp_curve_new_sized(i + 1);
        memcpy(new_curve->bpath, curve->bpath + p, i * sizeof(NArtBpath));
        new_curve->end = i;
        new_curve->bpath[i].code = NR_END;
        new_curve->substart = 0;
        new_curve->closed = (new_curve->bpath->code == NR_MOVETO);
        new_curve->hascpt = (new_curve->bpath->code == NR_MOVETO_OPEN);
        l = g_slist_append(l, new_curve);
        p += i;
    }

    return l;
}

template<class M>
static void
tmpl_curve_transform(SPCurve *const curve, M const &m)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->sbpath);

    for (gint i = 0; i < curve->end; i++) {
        NArtBpath *p = curve->bpath + i;
        switch (p->code) {
            case NR_MOVETO:
            case NR_MOVETO_OPEN:
            case NR_LINETO: {
                p->setC(3, p->c(3) * m);
                break;
            }
            case NR_CURVETO:
                for (unsigned i = 1; i <= 3; ++i) {
                    p->setC(i, p->c(i) * m);
                }
                break;
            default:
                g_warning("Illegal pathcode %d", p->code);
                break;
        }
    }
}

void
sp_curve_transform(SPCurve *const curve, NR::Matrix const &m)
{
    tmpl_curve_transform<NR::Matrix>(curve, m);
}

void
sp_curve_transform(SPCurve *const curve, NR::translate const &m)
{
    tmpl_curve_transform<NR::translate>(curve, m);
}


/* Methods */

void
sp_curve_reset(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->sbpath);

    curve->bpath->code = NR_END;
    curve->end = 0;
    curve->substart = 0;
    curve->hascpt = FALSE;
    curve->posset = FALSE;
    curve->moving = FALSE;
    curve->closed = FALSE;
}

/* Several consecutive movetos are ALLOWED */

void
sp_curve_moveto(SPCurve *curve, NR::Point const &p)
{
    sp_curve_moveto(curve, p[NR::X], p[NR::Y]);
}

void
sp_curve_moveto(SPCurve *curve, gdouble x, gdouble y)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->sbpath);
    g_return_if_fail(!curve->moving);

    curve->substart = curve->end;
    curve->hascpt = TRUE;
    curve->posset = TRUE;
    curve->x = x;
    curve->y = y;
}

void
sp_curve_lineto(SPCurve *curve, NR::Point const &p)
{
    sp_curve_lineto(curve, p[NR::X], p[NR::Y]);
}

void
sp_curve_lineto(SPCurve *curve, gdouble x, gdouble y)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->sbpath);
    g_return_if_fail(curve->hascpt);

    if (curve->moving) {
        /* fix endpoint */
        g_return_if_fail(!curve->posset);
        g_return_if_fail(curve->end > 1);
        NArtBpath *bp = curve->bpath + curve->end - 1;
        g_return_if_fail(bp->code == NR_LINETO);
        bp->x3 = x;
        bp->y3 = y;
        curve->moving = FALSE;
        return;
    }

    if (curve->posset) {
        /* start a new segment */
        sp_curve_ensure_space(curve, 2);
        NArtBpath *bp = curve->bpath + curve->end;
        bp->code = NR_MOVETO_OPEN;
        bp->x3 = curve->x;
        bp->y3 = curve->y;
        bp++;
        bp->code = NR_LINETO;
        bp->x3 = x;
        bp->y3 = y;
        bp++;
        bp->code = NR_END;
        curve->end += 2;
        curve->posset = FALSE;
        curve->closed = FALSE;
        return;
    }

    /* add line */

    g_return_if_fail(curve->end > 1);
    sp_curve_ensure_space(curve, 1);
    NArtBpath *bp = curve->bpath + curve->end;
    bp->code = NR_LINETO;
    bp->x3 = x;
    bp->y3 = y;
    bp++;
    bp->code = NR_END;
    curve->end++;
}

void
sp_curve_lineto_moving(SPCurve *curve, gdouble x, gdouble y)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->sbpath);
    g_return_if_fail(curve->hascpt);

    if (curve->moving) {
        /* change endpoint */
        g_return_if_fail(!curve->posset);
        g_return_if_fail(curve->end > 1);
        NArtBpath *bp = curve->bpath + curve->end - 1;
        g_return_if_fail(bp->code == NR_LINETO);
        bp->x3 = x;
        bp->y3 = y;
        return;
    }

    if (curve->posset) {
        /* start a new segment */
        sp_curve_ensure_space(curve, 2);
        NArtBpath *bp = curve->bpath + curve->end;
        bp->code = NR_MOVETO_OPEN;
        bp->x3 = curve->x;
        bp->y3 = curve->y;
        bp++;
        bp->code = NR_LINETO;
        bp->x3 = x;
        bp->y3 = y;
        bp++;
        bp->code = NR_END;
        curve->end += 2;
        curve->posset = FALSE;
        curve->moving = TRUE;
        curve->closed = FALSE;
        return;
    }

    /* add line */

    g_return_if_fail(curve->end > 1);
    sp_curve_ensure_space(curve, 1);
    NArtBpath *bp = curve->bpath + curve->end;
    bp->code = NR_LINETO;
    bp->x3 = x;
    bp->y3 = y;
    bp++;
    bp->code = NR_END;
    curve->end++;
    curve->moving = TRUE;
}

void
sp_curve_curveto(SPCurve *curve, NR::Point const &p0, NR::Point const &p1, NR::Point const &p2)
{
    using NR::X;
    using NR::Y;
    sp_curve_curveto(curve,
                     p0[X], p0[Y],
                     p1[X], p1[Y],
                     p2[X], p2[Y]);
}

void
sp_curve_curveto(SPCurve *curve, gdouble x0, gdouble y0, gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->sbpath);
    g_return_if_fail(curve->hascpt);
    g_return_if_fail(!curve->moving);

    if (curve->posset) {
        /* start a new segment */
        sp_curve_ensure_space(curve, 2);
        NArtBpath *bp = curve->bpath + curve->end;
        bp->code = NR_MOVETO_OPEN;
        bp->x3 = curve->x;
        bp->y3 = curve->y;
        bp++;
        bp->code = NR_CURVETO;
        bp->x1 = x0;
        bp->y1 = y0;
        bp->x2 = x1;
        bp->y2 = y1;
        bp->x3 = x2;
        bp->y3 = y2;
        bp++;
        bp->code = NR_END;
        curve->end += 2;
        curve->posset = FALSE;
        curve->closed = FALSE;
        return;
    }

    /* add curve */

    g_return_if_fail(curve->end > 1);
    sp_curve_ensure_space(curve, 1);
    NArtBpath *bp = curve->bpath + curve->end;
    bp->code = NR_CURVETO;
    bp->x1 = x0;
    bp->y1 = y0;
    bp->x2 = x1;
    bp->y2 = y1;
    bp->x3 = x2;
    bp->y3 = y2;
    bp++;
    bp->code = NR_END;
    curve->end++;
}

void
sp_curve_closepath(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->sbpath);
    g_return_if_fail(curve->hascpt);
    g_return_if_fail(!curve->posset);
    g_return_if_fail(!curve->moving);
    g_return_if_fail(!curve->closed);
    /* We need at last M + C + E */
    g_return_if_fail(curve->end - curve->substart > 1);

    NArtBpath *bs = curve->bpath + curve->substart;
    NArtBpath *be = curve->bpath + curve->end - 1;

    if ((bs->x3 != be->x3) || (bs->y3 != be->y3)) {
        sp_curve_lineto(curve, bs->x3, bs->y3);
    }

    bs = curve->bpath + curve->substart;
    be = curve->bpath + curve->end - 1;

    bs->code = NR_MOVETO;

    curve->closed = TRUE;

    for (bs = curve->bpath; bs->code != NR_END; bs++) {
        if (bs->code == NR_MOVETO_OPEN) {
            curve->closed = FALSE;
        }
    }

    curve->hascpt = FALSE;
}

void
sp_curve_closepath_current(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(!curve->sbpath);
    g_return_if_fail(curve->hascpt);
    g_return_if_fail(!curve->posset);
    g_return_if_fail(!curve->closed);
    /* We need at last M + L + L + E */
    g_return_if_fail(curve->end - curve->substart > 2);

    {
        NArtBpath *bs = curve->bpath + curve->substart;
        NArtBpath *be = curve->bpath + curve->end - 1;

        be->x3 = bs->x3;
        be->y3 = bs->y3;

        bs->code = NR_MOVETO;
    }
    curve->closed = TRUE;

    for (NArtBpath *bp = curve->bpath; bp->code != NR_END; bp++) {
        if (bp->code == NR_MOVETO_OPEN) {
            curve->closed = FALSE;
        }
    }

    curve->hascpt = FALSE;
    curve->moving = FALSE;
}

gboolean
sp_curve_empty(SPCurve *curve)
{
    g_return_val_if_fail(curve != NULL, TRUE);

    return (curve->bpath->code == NR_END);
}

NArtBpath *
sp_curve_last_bpath(SPCurve const *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    if (curve->end == 0) {
        return NULL;
    }

    return curve->bpath + curve->end - 1;
}

NArtBpath *
sp_curve_first_bpath(SPCurve const *curve)
{
    g_return_val_if_fail(curve != NULL, NULL);

    if (curve->end == 0) {
        return NULL;
    }

    return curve->bpath;
}

NR::Point
sp_curve_first_point(SPCurve const *const curve)
{
    NArtBpath *const bpath = sp_curve_first_bpath(curve);
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    return bpath->c(1);
}

NR::Point
sp_curve_last_point(SPCurve const *const curve)
{
    NArtBpath *const bpath = sp_curve_last_bpath(curve);
    g_return_val_if_fail(bpath != NULL, NR::Point(0, 0));
    return bpath->c(3);
}

SPCurve *
sp_curve_reverse(SPCurve *curve)
{
    /* We need at last M + C + E */
    g_return_val_if_fail(curve->end - curve->substart > 1, NULL);

    NArtBpath *bs = curve->bpath + curve->substart;
    NArtBpath *be = curve->bpath + curve->end - 1;

    SPCurve  *new_curve = sp_curve_new_sized(curve->length);

    g_assert(bs->code == NR_MOVETO_OPEN || bs->code == NR_MOVETO);
    g_assert((be+1)->code == NR_END);

    sp_curve_moveto(new_curve, be->x3, be->y3);

    for (NArtBpath *bp = be; bp != bs; bp--) {
        switch (bp->code) {
            case NR_MOVETO_OPEN:
                sp_curve_moveto(new_curve, (bp-1)->x3, (bp-1)->y3);
                break;

            case NR_MOVETO:
                sp_curve_moveto(new_curve, (bp-1)->x3, (bp-1)->y3);
                break;

            case NR_LINETO:
                sp_curve_lineto(new_curve, (bp-1)->x3, (bp-1)->y3);
                break;

            case NR_CURVETO:
                sp_curve_curveto(new_curve, bp->x2, bp->y2, bp->x1, bp->y1, (bp-1)->x3, (bp-1)->y3);
                break;

            case NR_END:
                g_assert_not_reached();
        }
    }

    return new_curve;
}

void
sp_curve_append(SPCurve *curve,
                SPCurve const *curve2,
                gboolean use_lineto)
{
    g_return_if_fail(curve != NULL);
    g_return_if_fail(curve2 != NULL);

    if (curve2->end < 1)
        return;

    NArtBpath const *bs = curve2->bpath;

    bool closed = curve->closed;

    for (NArtBpath const *bp = bs; bp->code != NR_END; bp++) {
        switch (bp->code) {
            case NR_MOVETO_OPEN:
                if (use_lineto && curve->hascpt) {
                    sp_curve_lineto(curve, bp->x3, bp->y3);
                    use_lineto = FALSE;
                } else {
                    if (closed) sp_curve_closepath(curve);
                    sp_curve_moveto(curve, bp->x3, bp->y3);
                }
                closed = false;
                break;

            case NR_MOVETO:
                if (use_lineto && curve->hascpt) {
                    sp_curve_lineto(curve, bp->x3, bp->y3);
                    use_lineto = FALSE;
                } else {
                    if (closed) sp_curve_closepath(curve);
                    sp_curve_moveto(curve, bp->x3, bp->y3);
                }
                closed = true;
                break;

            case NR_LINETO:
                sp_curve_lineto(curve, bp->x3, bp->y3);
                break;

            case NR_CURVETO:
                sp_curve_curveto(curve, bp->x1, bp->y1, bp->x2, bp->y2, bp->x3, bp->y3);
                break;

            case NR_END:
                g_assert_not_reached();
        }
    }

    if (closed) {
        sp_curve_closepath(curve);
    }
}

SPCurve *
sp_curve_append_continuous(SPCurve *c0, SPCurve const *c1, gdouble tolerance)
{
    g_return_val_if_fail(c0 != NULL, NULL);
    g_return_val_if_fail(c1 != NULL, NULL);
    g_return_val_if_fail(!c0->closed, NULL);
    g_return_val_if_fail(!c1->closed, NULL);

    if (c1->end < 1) {
        return c0;
    }

    NArtBpath *be = sp_curve_last_bpath(c0);
    if (be) {
        NArtBpath const *bs = sp_curve_first_bpath(c1);
        if ( bs
             && ( fabs( bs->x3 - be->x3 ) <= tolerance )
             && ( fabs( bs->y3 - be->y3 ) <= tolerance ) )
        {
            /* fixme: Strictly we mess in case of multisegment mixed open/close curves */
            bool closed = false;
            for (bs = bs + 1; bs->code != NR_END; bs++) {
                switch (bs->code) {
                    case NR_MOVETO_OPEN:
                        if (closed) sp_curve_closepath(c0);
                        sp_curve_moveto(c0, bs->x3, bs->y3);
                        closed = false;
                        break;
                    case NR_MOVETO:
                        if (closed) sp_curve_closepath(c0);
                        sp_curve_moveto(c0, bs->x3, bs->y3);
                        closed = true;
                        break;
                    case NR_LINETO:
                        sp_curve_lineto(c0, bs->x3, bs->y3);
                        break;
                    case NR_CURVETO:
                        sp_curve_curveto(c0, bs->x1, bs->y1, bs->x2, bs->y2, bs->x3, bs->y3);
                        break;
                    case NR_END:
                        g_assert_not_reached();
                }
            }
        } else {
            sp_curve_append(c0, c1, TRUE);
        }
    } else {
        sp_curve_append(c0, c1, TRUE);
    }

    return c0;
}

void
sp_curve_backspace(SPCurve *curve)
{
    g_return_if_fail(curve != NULL);

    if (curve->end > 0) {
        curve->end -= 1;
        if (curve->end > 0) {
            NArtBpath *bp = curve->bpath + curve->end - 1;
            if ((bp->code == NR_MOVETO)     ||
                (bp->code == NR_MOVETO_OPEN)  )
            {
                curve->hascpt = TRUE;
                curve->posset = TRUE;
                curve->closed = FALSE;
                curve->x = bp->x3;
                curve->y = bp->y3;
                curve->end -= 1;
            }
        }
        curve->bpath[curve->end].code = NR_END;
    }
}

/* Private methods */

static bool sp_bpath_good(NArtBpath const bpath[])
{
    g_return_val_if_fail(bpath != NULL, FALSE);

    NArtBpath const *bp = bpath;
    while (bp->code != NR_END) {
        bp = sp_bpath_check_subpath(bp);
        if (bp == NULL)
            return false;
    }

    return true;
}

static NArtBpath *sp_bpath_clean(NArtBpath const bpath[])
{
    NArtBpath *new_bpath = nr_new(NArtBpath, sp_bpath_length(bpath));

    NArtBpath const *bp = bpath;
    NArtBpath *np = new_bpath;

    while (bp->code != NR_END) {
        if (sp_bpath_check_subpath(bp)) {
            *np++ = *bp++;
            while ((bp->code == NR_LINETO) ||
                   (bp->code == NR_CURVETO))
                *np++ = *bp++;
        } else {
            bp++;
            while ((bp->code == NR_LINETO) ||
                   (bp->code == NR_CURVETO))
                bp++;
        }
    }

    if (np == new_bpath) {
        nr_free(new_bpath);
        return NULL;
    }

    np->code = NR_END;
    np += 1;

    new_bpath = nr_renew(new_bpath, NArtBpath, np - new_bpath);

    return new_bpath;
}

static NArtBpath const *sp_bpath_check_subpath(NArtBpath const bpath[])
{
    g_return_val_if_fail(bpath != NULL, NULL);

    bool closed;
    if (bpath->code == NR_MOVETO) {
        closed = true;
    } else if (bpath->code == NR_MOVETO_OPEN) {
        closed = false;
    } else {
        return NULL;
    }

    gint len = 0;
    gint i;
    for (i = 1; (bpath[i].code != NR_END) && (bpath[i].code != NR_MOVETO) && (bpath[i].code != NR_MOVETO_OPEN); i++) {
        switch (bpath[i].code) {
            case NR_LINETO:
            case NR_CURVETO:
                len++;
                break;
            default:
                return NULL;
        }
    }

    if (closed) {
        if (len < 1)
            return NULL;

        if ((bpath->x3 != bpath[i-1].x3) || (bpath->y3 != bpath[i-1].y3))
            return NULL;
    } else {
        if (len < 1)
            return NULL;
    }

    return bpath + i;
}

static unsigned sp_bpath_length(NArtBpath const bpath[])
{
    g_return_val_if_fail(bpath != NULL, FALSE);

    unsigned ret = 0;
    while ( bpath[ret].code != NR_END ) {
        ++ret;
    }
    ++ret;

    return ret;
}

/*fixme: this is bogus -- it doesn't check for nr_moveto, which will indicate a closing of the
subpath it's nonsense to talk about a path as a whole being closed, although maybe someone would
want that for some other reason?  Oh, also, if the bpath just ends, then it's *open*.  I hope
nobody is using this code for anything. */
static bool sp_bpath_closed(NArtBpath const bpath[])
{
    g_return_val_if_fail(bpath != NULL, FALSE);

    for (NArtBpath const *bp = bpath; bp->code != NR_END; bp++) {
        if (bp->code == NR_MOVETO_OPEN) {
            return false;
        }
    }

    return true;
}


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
