#ifndef __SP_COLOR_SELECTOR_H__
#define __SP_COLOR_SELECTOR_H__

#include <gtk/gtkvbox.h>
#include "../color.h"

#include <glib.h>

G_BEGIN_DECLS

typedef struct _SPColorSelector SPColorSelector;
typedef struct _SPColorSelectorClass SPColorSelectorClass;

#define SP_TYPE_COLOR_SELECTOR (sp_color_selector_get_type ())
#define SP_COLOR_SELECTOR(o) (GTK_CHECK_CAST ((o), SP_TYPE_COLOR_SELECTOR, SPColorSelector))
#define SP_COLOR_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_CAST ((k), SP_TYPE_COLOR_SELECTOR, SPColorSelectorClass))
#define SP_IS_COLOR_SELECTOR(o) (GTK_CHECK_TYPE ((o), SP_TYPE_COLOR_SELECTOR))
#define SP_IS_COLOR_SELECTOR_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), SP_TYPE_COLOR_SELECTOR))
#define SP_COLOR_SELECTOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SP_TYPE_COLOR_SELECTOR, SPColorSelectorClass))

struct _SPColorSelector {
	GtkVBox vbox;

	SPColor color;
	gfloat alpha;
};

struct _SPColorSelectorClass {
	GtkVBoxClass parent_class;

	const gchar **name;
	guint submode_count;

	void (* grabbed) (SPColorSelector *rgbsel);
	void (* dragged) (SPColorSelector *rgbsel);
	void (* released) (SPColorSelector *rgbsel);
	void (* changed) (SPColorSelector *rgbsel);

	/* virtual functions */
	void (* set_submode) (SPColorSelector *csel, guint rgba);
	guint (* get_submode) (SPColorSelector *csel);

	void (* set_color_alpha) (SPColorSelector *csel, const SPColor *color, gfloat alpha);
	void (* get_color_alpha) (SPColorSelector *csel, SPColor *color, gfloat *alpha);
};

GType sp_color_selector_get_type (void);

GtkWidget *sp_color_selector_new (GType selector_type, SPColorSpaceType colorspace);

void sp_color_selector_set_submode (SPColorSelector* csel, guint submode);
guint sp_color_selector_get_submode (SPColorSelector* csel);

SPColorSpaceType sp_color_selector_get_colorspace (SPColorSelector* csel);
gboolean sp_color_selector_set_colorspace (SPColorSelector* csel, SPColorSpaceType colorspace);

void sp_color_selector_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha);
void sp_color_selector_get_color_alpha (SPColorSelector *csel, SPColor *color, gfloat *alpha);

G_END_DECLS

#endif
