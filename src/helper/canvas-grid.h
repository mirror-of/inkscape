#ifndef SP_CANVAS_GRID_H
#define SP_CANVAS_GRID_H

/*
 * SPCGrid
 *
 * Generic (and quite unintelligent) grid item for gnome canvas
 *
 * Copyright (C) Lauris Kaplinski 2000
 *
 */

#include <libart_lgpl/art_point.h>
#include "sp-canvas.h"

G_BEGIN_DECLS

#define SP_TYPE_CGRID            (sp_cgrid_get_type ())
#define SP_CGRID(obj)            (GTK_CHECK_CAST ((obj), SP_TYPE_CGRID, SPCGrid))
#define SP_CGRID_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), SP_TYPE_CGRID, SPCGridClass))
#define SP_IS_CGRID(obj)         (GTK_CHECK_TYPE ((obj), SP_TYPE_CGRID))
#define SP_IS_CGRID_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), SP_TYPE_CGRID))


typedef struct _SPCGrid SPCGrid;
typedef struct _SPCGridClass SPCGridClass;

struct _SPCGrid {
	SPCanvasItem item;

	ArtPoint origin;
	ArtPoint spacing;
	guint32 color;

	ArtPoint ow, sw;
};

struct _SPCGridClass {
	SPCanvasItemClass parent_class;
};


/* Standard Gtk function */
GtkType sp_cgrid_get_type (void);

G_END_DECLS

#endif
