#ifndef __INKSCAPE_CTRLRECT_H__
#define __INKSCAPE_CTRLRECT_H__

/*
 * Simple non-transformed rectangle, usable for rubberband
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 1999-2001 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL
 *
 */

#include <glib.h>
#include "sp-canvas.h"


#define SP_TYPE_CTRLRECT (sp_ctrlrect_get_type ())
#define SP_CTRLRECT(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CTRLRECT, SPCtrlRect))
#define SP_CTRLRECT_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CTRLRECT, SPCtrlRectClass))
#define SP_IS_CTRLRECT(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CTRLRECT))
#define SP_IS_CTRLRECT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CTRLRECT))

struct SPCtrlRect : public SPCanvasItem {
    guint has_fill : 1;
    guint dashed : 1;
    
    NRRect rect;
    gint shadow;
    
    NRRectL area;
    gint shadow_size;
    guint32 border_color;
    guint32 fill_color;
    guint32 shadow_color;
};

struct SPCtrlRectClass : public SPCanvasItemClass {};


/* Standard Gtk function */
GtkType sp_ctrlrect_get_type (void);

void sp_ctrlrect_set_area (SPCtrlRect *rect, double x0, double y0, double x1, double y1);
void sp_ctrlrect_set_color (SPCtrlRect *rect, guint32 border_color, gboolean has_fill, guint32 fill_color);
void sp_ctrlrect_set_shadow (SPCtrlRect *rect, gint shadow_size, guint32 shadow_color);
void sp_ctrlrect_set_dashed (SPCtrlRect *rect, guint dashed);

/* Deprecated */
void sp_ctrlrect_set_rect (SPCtrlRect * rect, NRRect * box);



#endif

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
