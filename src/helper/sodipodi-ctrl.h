#ifndef SODIPODI_CTRL_H
#define SODIPODI_CTRL_H

/* sodipodi-ctrl
 *
 * It is simply small square, which does not scale nor rotate
 *
 */

#include <gtk/gtkenums.h>
#include "sp-canvas.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define SP_TYPE_CTRL            (sp_ctrl_get_type ())
#define SP_CTRL(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_CTRL, SPCtrl))
#define SP_CTRL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CTRL, SPCtrlClass))
#define SP_IS_CTRL(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_CTRL))
#define SP_IS_CTRL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CTRL))

typedef struct _SPCtrl SPCtrl;
typedef struct _SPCtrlClass SPCtrlClass;

typedef enum {
	SP_CTRL_SHAPE_SQUARE,
	SP_CTRL_SHAPE_DIAMOND,
	SP_CTRL_SHAPE_CIRCLE,
	SP_CTRL_SHAPE_CROSS,
	SP_CTRL_SHAPE_BITMAP,
	SP_CTRL_SHAPE_IMAGE
} SPCtrlShapeType;


typedef enum {
	SP_CTRL_MODE_COLOR,
	SP_CTRL_MODE_XOR
} SPCtrlModeType;

struct _SPCtrl {
	SPCanvasItem item;

	SPCtrlShapeType shape;
	SPCtrlModeType mode;
	GtkAnchorType anchor;
	gint span;
	guint defined : 1;
	guint shown   : 1;
        guint build   : 1;
	guint filled  : 1;
	guint stroked : 1;
	guint32 fill_color;
	guint32 stroke_color;

	ArtIRect box;			/* NB! x1 & y1 are included */
	guchar *cache;
	GdkPixbuf * pixbuf;
};

struct _SPCtrlClass {
	SPCanvasItemClass parent_class;
};



/* Standard Gtk function */
GtkType sp_ctrl_get_type (void);

void sp_ctrl_moveto (SPCtrl * ctrl, double x, double y);

G_END_DECLS

#endif
