#ifndef __HELPER_FORWARD_H__
#define __HELPER_FORWARD_H__

/*
 * Forward declarations
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glib-object.h>

struct SPCanvas;
struct SPCanvasClass;
struct SPCanvasItem;
struct SPCanvasItemClass;
struct SPCanvasGroup;
struct SPCanvasGroupClass;

#define SP_TYPE_CANVAS_ITEM (sp_canvas_item_get_type ())
#define SP_CANVAS_ITEM(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CANVAS_ITEM, SPCanvasItem))
#define SP_IS_CANVAS_ITEM(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CANVAS_ITEM))
#define SP_CANVAS_ITEM_GET_CLASS(o) (GTK_CHECK_GET_CLASS ((o), SP_TYPE_CANVAS_ITEM, SPCanvasItemClass))

GType sp_canvas_item_get_type (void);

#define SP_TYPE_CANVAS_GROUP (sp_canvas_group_get_type ())
#define SP_CANVAS_GROUP(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CANVAS_GROUP, SPCanvasGroup))
#define SP_IS_CANVAS_GROUP(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CANVAS_GROUP))

GType sp_canvas_group_get_type (void);

#define SP_TYPE_CANVAS (sp_canvas_get_type ())
#define SP_CANVAS(obj) (GTK_CHECK_CAST ((obj), SP_TYPE_CANVAS, SPCanvas))
#define SP_IS_CANVAS(obj) (GTK_CHECK_TYPE ((obj), SP_TYPE_CANVAS))

GType sp_canvas_get_type (void);

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
