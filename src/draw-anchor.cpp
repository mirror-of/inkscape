/** \file Anchors. */

/*
 * Initial author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2000 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtk/gtktypeutils.h>

#include "draw-anchor.h"
#include "desktop.h"
#include "desktop-affine.h"
#include "desktop-handles.h"
#include "event-context.h"
#include "message.h"
#include "display/sodipodi-ctrl.h"
#include "display/sp-canvas.h"
#include "helper/sp-intl.h"
#include "libnr/nr-point-fns.h"
#include "libnr/nr-point-ops.h"


SPDrawAnchor *
sp_draw_anchor_new(SPDrawContext *dc, SPCurve *curve, gboolean start, NR::Point delta)
{
    SPDesktop *dt = SP_EVENT_CONTEXT_DESKTOP(dc);

    dt->messageStack()->flashF(Inkscape::NORMAL_MESSAGE, _("Creating anchor at (%g,%g)"),
                               delta[NR::X], delta[NR::Y]);

    SPDrawAnchor *a = g_new(SPDrawAnchor, 1);

    a->dc = dc;
    a->curve = curve;
    a->start = start;
    a->active = FALSE;
    a->dp = delta;
    a->wp = sp_desktop_d2w_xy_point(dt, delta);
    a->ctrl = sp_canvas_item_new(SP_DT_CONTROLS(dt), SP_TYPE_CTRL,
                                 "size", 4.0,
                                 "filled", 0,
                                 "fill_color", 0xff00007f,
                                 "stroked", 1,
                                 "stroke_color", 0x000000ff,
                                 NULL);

    SP_CTRL(a->ctrl)->moveto(delta);

    return a;
}

SPDrawAnchor *
sp_draw_anchor_destroy(SPDrawAnchor *anchor)
{
    if (anchor->ctrl) {
        gtk_object_destroy(GTK_OBJECT(anchor->ctrl));
    }
    g_free(anchor);
    return NULL;
}

#define A_SNAP 4.0

SPDrawAnchor *
sp_draw_anchor_test(SPDrawAnchor *anchor, NR::Point w, gboolean activate)
{
    if ( activate && ( NR::LInfty( w - anchor->wp ) <= A_SNAP ) ) {
        if (!anchor->active) {
            sp_canvas_item_set((GtkObject *) anchor->ctrl, "filled", TRUE, NULL);
            anchor->active = TRUE;
        }
        return anchor;
    }

    if (anchor->active) {
        sp_canvas_item_set((GtkObject *) anchor->ctrl, "filled", FALSE, NULL);
        anchor->active = FALSE;
    }
    return NULL;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
