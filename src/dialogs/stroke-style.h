#ifndef __SP_STROKE_STYLE_H__
#define __SP_STROKE_STYLE_H__

/**
 * \brief  Stroke style dialog
 *
 * Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 */

#include <glib.h>

#include <gtk/gtkwidget.h>

#include "forward.h"
#include "helper/canvas-bpath.h"

GtkWidget *sp_stroke_style_paint_widget_new (void);
GtkWidget *sp_stroke_style_line_widget_new (void);

void sp_stroke_style_paint_system_color_set ( GtkWidget *widget, 
                                              SPColor *color, 
                                              float opacity );

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
