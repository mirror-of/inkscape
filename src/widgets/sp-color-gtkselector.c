#include <config.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtktable.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkcolorsel.h>
#include "../color.h"
#include "../helper/sp-intl.h"
#include "../dialogs/dialog-events.h"
#include "sp-color-gtkselector.h"

enum {
	GRABBED,
	DRAGGED,
	RELEASED,
	CHANGED,
	LAST_SIGNAL
};


static void sp_color_gtkselector_class_init (SPColorGtkselectorClass *klass);
static void sp_color_gtkselector_init (SPColorGtkselector *csel);
static void sp_color_gtkselector_destroy (GtkObject *object);

static void sp_color_gtkselector_show_all (GtkWidget *widget);
static void sp_color_gtkselector_hide_all (GtkWidget *widget);

static void sp_color_gtkselector_gtk_changed (GtkColorSelection *colorselection, SPColorGtkselector *gtksel);

static void sp_color_gtkselector_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha);



static SPColorSelectorClass *parent_class;
static guint cs_signals[LAST_SIGNAL] = {0};

#define XPAD 4
#define YPAD 1

GType
sp_color_gtkselector_get_type (void)
{
	static GType type = 0;
	if (!type) {
		static const GTypeInfo info = {
			sizeof (SPColorGtkselectorClass),
			NULL, /* base_init */
			NULL, /* base_finalize */
			(GClassInitFunc) sp_color_gtkselector_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (SPColorGtkselector),
			0,	  /* n_preallocs */
			(GInstanceInitFunc) sp_color_gtkselector_init,
		};

		type = g_type_register_static (SP_TYPE_COLOR_SELECTOR,
									   "SPColorGtkselector",
									   &info,
									   INK_STATIC_CAST( GTypeFlags, 0));
	}
	return type;
}

static void
sp_color_gtkselector_class_init (SPColorGtkselectorClass *klass)
{
	static const gchar* nameset[] = {_("GTK+"), 0};
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;
	SPColorSelectorClass *selector_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;
	selector_class = SP_COLOR_SELECTOR_CLASS (klass);

	parent_class = SP_COLOR_SELECTOR_CLASS (g_type_class_peek_parent (klass));

	selector_class->name = nameset;
	selector_class->submode_count = 1;

	cs_signals[GRABBED] =  gtk_signal_lookup ("grabbed", SP_TYPE_COLOR_SELECTOR);
	cs_signals[DRAGGED] =  gtk_signal_lookup ("dragged", SP_TYPE_COLOR_SELECTOR);
	cs_signals[RELEASED] = gtk_signal_lookup ("released", SP_TYPE_COLOR_SELECTOR);
	cs_signals[CHANGED] =  gtk_signal_lookup ("changed", SP_TYPE_COLOR_SELECTOR);

	selector_class->set_color_alpha = sp_color_gtkselector_set_color_alpha;
	selector_class->get_color_alpha = NULL;

	object_class->destroy = sp_color_gtkselector_destroy;

	widget_class->show_all = sp_color_gtkselector_show_all;
	widget_class->hide_all = sp_color_gtkselector_hide_all;
}

void sp_color_gtkselector_init (SPColorGtkselector *csel)
{
	GtkWidget *gtksel;

	gtksel = gtk_color_selection_new();
	gtk_widget_show (gtksel);
	csel->gtk_thing = GTK_COLOR_SELECTION (gtksel);
	gtk_box_pack_start (GTK_BOX (csel), gtksel, TRUE, TRUE, 0);

	gtk_signal_connect (GTK_OBJECT (gtksel), "color-changed", GTK_SIGNAL_FUNC (sp_color_gtkselector_gtk_changed), csel);
}

static void
sp_color_gtkselector_destroy (GtkObject *object)
{
	SPColorGtkselector *csel;

	csel = SP_COLOR_GTKSELECTOR (object);

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

static void
sp_color_gtkselector_show_all (GtkWidget *widget)
{
	gtk_widget_show (widget);
}

static void
sp_color_gtkselector_hide_all (GtkWidget *widget)
{
	gtk_widget_hide (widget);
}

GtkWidget *
sp_color_gtkselector_new (GType selector_type, SPColorSpaceType colorspace)
{
	SPColorGtkselector *csel;
	SPColor color;

	csel = (SPColorGtkselector*)gtk_type_new (SP_TYPE_COLOR_GTKSELECTOR);

	sp_color_set_rgb_rgba32 (&color, 0);
	sp_color_selector_set_color_alpha (SP_COLOR_SELECTOR(csel), &color, 1.0);

	return GTK_WIDGET (csel);
}

void
sp_color_gtkselector_set_color_alpha (SPColorSelector *csel, const SPColor *color, gfloat alpha)
{
	SPColorGtkselector* gsel;
	GdkColor gcolor;
	float rgb[3];
	g_return_if_fail (csel != NULL);
	g_return_if_fail (SP_IS_COLOR_GTKSELECTOR (csel));

	gsel = SP_COLOR_GTKSELECTOR (csel);


	sp_color_copy (&csel->color, color);
	csel->alpha = alpha;

	sp_color_get_rgb_floatv (color, rgb);
	gcolor.pixel = 0;
	gcolor.red = INK_STATIC_CAST (guint16, rgb[0] * 65535);
	gcolor.green = INK_STATIC_CAST (guint16, rgb[1] * 65535);
	gcolor.blue = INK_STATIC_CAST (guint16, rgb[2] * 65535);

	gtk_color_selection_set_current_alpha (gsel->gtk_thing, (guint16)(65535 * alpha));
	gtk_color_selection_set_current_color (gsel->gtk_thing, &gcolor);
}

static void sp_color_gtkselector_gtk_changed (GtkColorSelection *colorselection, SPColorGtkselector *gtksel)
{
	SPColor ourColor;
	GdkColor color;
	guint16 alpha;
	SPColorSelector *csel;

	gtk_color_selection_get_current_color (colorselection, &color);
	alpha = gtk_color_selection_get_current_alpha (colorselection);

	sp_color_set_rgb_float (&ourColor, (color.red / 65535.0), (color.green / 65535.0), (color.blue / 65535.0));
	csel = SP_COLOR_SELECTOR (gtksel);
	sp_color_copy (&csel->color, &ourColor);
	csel->alpha = INK_STATIC_CAST(gfloat, alpha / 65535.0);

	gtk_signal_emit (GTK_OBJECT (csel), cs_signals[CHANGED]);
}

