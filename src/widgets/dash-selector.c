#define __SP_DASH_SELECTOR_C__

/*
 * Optionmenu for selecting dash patterns
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define DASH_PREVIEW_WIDTH 2

#include <string.h>
#include <libnr/nr-values.h>
#include <libnr/nr-macros.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtkspinbutton.h>
#include "../xml/repr-private.h"
#include "../style.h"
#include "dash-selector.h"

typedef struct _SPDashSelectorClass SPDashSelectorClass;

enum {CHANGED, LAST_SIGNAL};

struct _SPDashSelector {
	GtkHBox hbox;

	GtkWidget *dash;
	GtkObject *offset;
};

struct _SPDashSelectorClass {
	GtkHBoxClass parent_class;

	void (* changed) (SPDashSelector *dsel);
};

double dash_0[] = {-1.0};
double dash_1_1[] = {1.0, 1.0, -1.0};
double dash_2_1[] = {2.0, 1.0, -1.0};
double dash_4_1[] = {4.0, 1.0, -1.0};
double dash_1_2[] = {1.0, 2.0, -1.0};
double dash_1_4[] = {1.0, 4.0, -1.0};

double *builtin_dashes[] = {dash_0, dash_1_1, dash_2_1, dash_4_1, dash_1_2, dash_1_4, NULL};

static double **dashes = NULL;

static void sp_dash_selector_class_init (SPDashSelectorClass *klass);
static void sp_dash_selector_init (SPDashSelector *dsel);
static GtkWidget *sp_dash_selector_menu_item_new (SPDashSelector *dsel, double *pattern);
static void sp_dash_selector_dash_activate (GtkObject *object, SPDashSelector *dsel);
static void sp_dash_selector_offset_value_changed (GtkAdjustment *adj, SPDashSelector *dsel);

static GtkHBoxClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

GtkType
sp_dash_selector_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPDashSelector",
			sizeof (SPDashSelector),
			sizeof (SPDashSelectorClass),
			(GtkClassInitFunc) sp_dash_selector_class_init,
			(GtkObjectInitFunc) sp_dash_selector_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_HBOX, &info);
	}
	return type;
}

static void
sp_dash_selector_class_init (SPDashSelectorClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	object_class = (GtkObjectClass *) klass;
	widget_class = (GtkWidgetClass *) klass;

	parent_class = gtk_type_class (GTK_TYPE_HBOX);

	signals[CHANGED] = gtk_signal_new ("changed",
					   GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
					   G_TYPE_FROM_CLASS (klass),
					   GTK_SIGNAL_OFFSET (SPDashSelectorClass, changed),
					   gtk_marshal_NONE__NONE,
					   GTK_TYPE_NONE, 0);
}

static void
sp_dash_selector_init (SPDashSelector *dsel)
{
	GtkWidget *m, *mi, *sb;
	int i;

	dsel->dash = gtk_option_menu_new ();
	gtk_widget_show (dsel->dash);
	gtk_box_pack_start (GTK_BOX (dsel), dsel->dash, FALSE, FALSE, 0);

	m = gtk_menu_new ();
	gtk_widget_show (m);
	for (i = 0; dashes[i]; i++) {
		mi = sp_dash_selector_menu_item_new (dsel, dashes[i]);
		gtk_widget_show (mi);
		gtk_menu_append (GTK_MENU (m), mi);
	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU (dsel->dash), m);

	dsel->offset = gtk_adjustment_new (0.0, 0.0, 10.0, 0.1, 1.0, 1.0);
	sb = gtk_spin_button_new (GTK_ADJUSTMENT (dsel->offset), 0.1, 2);
	gtk_widget_show (sb);
	gtk_box_pack_start (GTK_BOX (dsel), sb, FALSE, FALSE, 0);
	gtk_signal_connect (dsel->offset, "value_changed", GTK_SIGNAL_FUNC (sp_dash_selector_offset_value_changed), dsel);

	gtk_object_set_data (GTK_OBJECT (dsel), "pattern", dashes[0]);
}

GtkWidget *
sp_dash_selector_new (SPRepr *drepr)
{
	GtkWidget *dsel;

	if (!dashes) {
		SPRepr *dr;
		int ndashes;

		ndashes = 0;
		if (drepr) {
			for (dr = drepr->children; dr; dr = dr->next) {
				if (!strcmp (sp_repr_name (dr), "dash")) ndashes += 1;
			}
		}

		if (ndashes > 0) {
			SPStyle *style;
			int pos;
			pos = 0;
			style = sp_style_new ();
			dashes = g_new (double *, ndashes + 1);
			for (dr = drepr->children; dr; dr = dr->next) {
				if (!strcmp (sp_repr_name (dr), "dash")) {
					sp_style_read_from_repr (style, dr);
					if (style->stroke_dash.n_dash > 0) {
						double *d;
						int i;
						dashes[pos] = g_new (double, style->stroke_dash.n_dash + 1);
						d = dashes[pos];
						for (i = 0; i < style->stroke_dash.n_dash; i++) {
							d[i] = style->stroke_dash.dash[i];
						}
						d[i] = -1;
					} else {
						dashes[pos] = dash_0;
					}
					pos += 1;
				}
			}
			sp_style_unref (style);
			dashes[pos] = NULL;
		} else {
			dashes = builtin_dashes;
		}
	}

	dsel = gtk_type_new (SP_TYPE_DASH_SELECTOR);

	return dsel;
}

void
sp_dash_selector_set_dash (SPDashSelector *dsel, int ndash, double *dash, double offset)
{
	int pos;

	pos = 0;

	if (ndash > 0) {
		double delta;
		int i;
		delta = 0.0;
		for (i = 0; i < ndash; i++) delta += dash[i];
		delta = delta / 1000.0;

		for (i = 0; dashes[i]; i++) {
			double *pattern;
			int np;
			pattern = dashes[i];
			np = 0;
			while (pattern[np] >= 0.0) np += 1;
			if (np == ndash) {
				int j;
				for (j = 0; j < ndash; j++) {
					if (!NR_DF_TEST_CLOSE (dash[j], pattern[j], delta)) break;
				}
				if (j == ndash) {
					pos = i;
					break;
				}
			}
		}
	}

	gtk_object_set_data (GTK_OBJECT (dsel), "pattern", dashes[pos]);
	gtk_option_menu_set_history (GTK_OPTION_MENU (dsel->dash), pos);
	gtk_adjustment_set_value (GTK_ADJUSTMENT (dsel->offset), offset);
}

void
sp_dash_selector_get_dash (SPDashSelector *dsel, int *ndash, double **dash, double *offset)
{
	double *pattern;
	int nd;

	pattern = gtk_object_get_data (GTK_OBJECT (dsel), "pattern");

	nd = 0;
	while (pattern[nd] >= 0.0) nd += 1;

	if (nd > 0) {
		if (ndash) *ndash = nd;
		if (dash) {
			*dash = g_new (double, nd);
			memcpy (*dash, pattern, nd * sizeof (double));
		}
		if (offset) *offset = GTK_ADJUSTMENT (dsel->offset)->value;
	} else {
		if (ndash) *ndash = 0;
		if (dash) *dash = NULL;
		if (offset) *offset = 0.0;
	}
}

static GtkWidget *
sp_dash_selector_menu_item_new (SPDashSelector *dsel, double *pattern)
{
	GtkWidget *mi, *px;
	GdkPixmap *pixmap;
	GdkGC *gc;
	gint8 idash[32];
	int ndash;

	mi = gtk_menu_item_new ();

	pixmap = gdk_pixmap_new (GTK_WIDGET (dsel)->window, 64, 16, gdk_visual_get_best_depth ());
	gc = gdk_gc_new (pixmap);

	gdk_rgb_gc_set_foreground (gc, 0xffffffff);
	gdk_draw_rectangle (pixmap, gc, TRUE, 0, 0, 64, 16);
	gdk_rgb_gc_set_foreground (gc, 0x00000000);
	for (ndash = 0; pattern[ndash] >= 0.0; ndash ++) idash[ndash] = (gint8) (DASH_PREVIEW_WIDTH * pattern[ndash] + 0.5);
	if (ndash > 0) {
		gdk_gc_set_line_attributes (gc, DASH_PREVIEW_WIDTH, GDK_LINE_ON_OFF_DASH, GDK_CAP_BUTT, GDK_JOIN_MITER);
		gdk_gc_set_dashes (gc, 0, idash, ndash);
	} else {
		gdk_gc_set_line_attributes (gc, DASH_PREVIEW_WIDTH, GDK_LINE_SOLID, GDK_CAP_BUTT, GDK_JOIN_MITER);
	}
	gdk_draw_line (pixmap, gc, 4, 8, 60, 8);

	gdk_gc_unref (gc);

	px = gtk_pixmap_new (pixmap, NULL);
#if 1
	gdk_pixmap_unref (pixmap);
#endif
	gtk_widget_show (px);
	gtk_container_add (GTK_CONTAINER (mi), px);

	gtk_object_set_data (GTK_OBJECT (mi), "pattern", pattern);
	gtk_signal_connect (GTK_OBJECT (mi), "activate",
			    G_CALLBACK (sp_dash_selector_dash_activate), dsel);

	return mi;
}

static void
sp_dash_selector_dash_activate (GtkObject *object, SPDashSelector *dsel)
{
	double *pattern;

	pattern = gtk_object_get_data (object, "pattern");
	gtk_object_set_data (GTK_OBJECT (dsel), "pattern", pattern);

	gtk_signal_emit (GTK_OBJECT (dsel), signals[CHANGED]);
}

static void
sp_dash_selector_offset_value_changed (GtkAdjustment *adj, SPDashSelector *dsel)
{
	gtk_signal_emit (GTK_OBJECT (dsel), signals[CHANGED]);
}
