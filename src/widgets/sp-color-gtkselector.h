#ifndef __SP_COLOR_GTKSELECTOR_H__
#define __SP_COLOR_GTKSELECTOR_H__

#include <gtk/gtkcolorsel.h>
#include "../color.h"
#include "sp-color-selector.h"

#include <glib.h>

G_BEGIN_DECLS

typedef struct _SPColorGtkselector SPColorGtkselector;
typedef struct _SPColorGtkselectorClass SPColorGtkselectorClass;


#define SP_TYPE_COLOR_GTKSELECTOR (sp_color_gtkselector_get_type ())
#define SP_COLOR_GTKSELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_GTKSELECTOR, SPColorGtkselector))
#define SP_COLOR_GTKSELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_GTKSELECTOR, SPColorGtkselectorClass))
#define SP_IS_COLOR_GTKSELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_GTKSELECTOR))
#define SP_IS_COLOR_GTKSELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_GTKSELECTOR))

struct _SPColorGtkselector {
	SPColorSelector base;
	GtkColorSelection* gtk_thing;
};

struct _SPColorGtkselectorClass {
	SPColorSelectorClass parent_class;
};

GType sp_color_gtkselector_get_type (void);

GtkWidget *sp_color_gtkselector_new (GType selector_type, SPColorSpaceType colorspace);

G_END_DECLS

#endif
