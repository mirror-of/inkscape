#define __SP_GRADIENT_VECTOR_C__

/*
 * Gradient vector selection widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkframe.h>
#include <gtk/gtktable.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkwindow.h>
#include "macros.h"
#include "../helper/sp-intl.h"
#include "../widgets/gradient-image.h"
#include "../sodipodi.h"
#include "../document-private.h"
#include "../sp-gradient.h"
#include "../gradient-chemistry.h"
#include "gradient-vector.h"

enum {
	VECTOR_SET,
	LAST_SIGNAL
};

static void sp_gradient_vector_selector_class_init (SPGradientVectorSelectorClass *klass);
static void sp_gradient_vector_selector_init (SPGradientVectorSelector *gvs);
static void sp_gradient_vector_selector_destroy (GtkObject *object);

static void sp_gvs_gradient_release (SPGradient *gr, SPGradientVectorSelector *gvs);
static void sp_gvs_defs_release (SPObject *defs, SPGradientVectorSelector *gvs);
static void sp_gvs_defs_modified (SPObject *defs, guint flags, SPGradientVectorSelector *gvs);

static void sp_gvs_rebuild_gui_full (SPGradientVectorSelector *gvs);
static void sp_gvs_gradient_activate (GtkMenuItem *mi, SPGradientVectorSelector *gvs);

static GtkVBoxClass *parent_class;
static guint signals[LAST_SIGNAL] = {0};

GtkType
sp_gradient_vector_selector_get_type (void)
{
	static GtkType type = 0;
	if (!type) {
		GtkTypeInfo info = {
			"SPGradientVectorSelector",
			sizeof (SPGradientVectorSelector),
			sizeof (SPGradientVectorSelectorClass),
			(GtkClassInitFunc) sp_gradient_vector_selector_class_init,
			(GtkObjectInitFunc) sp_gradient_vector_selector_init,
			NULL, NULL, NULL
		};
		type = gtk_type_unique (GTK_TYPE_VBOX, &info);
	}
	return type;
}

static void
sp_gradient_vector_selector_class_init (SPGradientVectorSelectorClass *klass)
{
	GtkObjectClass *object_class;

	object_class = GTK_OBJECT_CLASS (klass);

	parent_class = gtk_type_class (GTK_TYPE_VBOX);

	signals[VECTOR_SET] = gtk_signal_new ("vector_set",
					      GTK_RUN_LAST,
					      GTK_CLASS_TYPE(object_class),
					      GTK_SIGNAL_OFFSET (SPGradientVectorSelectorClass, vector_set),
					      gtk_marshal_NONE__POINTER,
					      GTK_TYPE_NONE, 1,
					      GTK_TYPE_POINTER);

	object_class->destroy = sp_gradient_vector_selector_destroy;
}

static void
sp_gradient_vector_selector_init (SPGradientVectorSelector *gvs)
{
	gvs->idlabel = TRUE;

	gvs->doc = NULL;
	gvs->gr = NULL;

	gvs->menu = gtk_option_menu_new ();
	gtk_widget_show (gvs->menu);
	gtk_box_pack_start (GTK_BOX (gvs), gvs->menu, TRUE, TRUE, 0);
}

static void
sp_gradient_vector_selector_destroy (GtkObject *object)
{
	SPGradientVectorSelector *gvs;

	gvs = SP_GRADIENT_VECTOR_SELECTOR (object);

	if (gvs->gr) {
  		sp_signal_disconnect_by_data (gvs->gr, gvs);
		gvs->gr = NULL;
	}

	if (gvs->doc) {
  		sp_signal_disconnect_by_data (SP_DOCUMENT_DEFS (gvs->doc), gvs);
		gvs->doc = NULL;
	}

	if (((GtkObjectClass *) (parent_class))->destroy)
		(* ((GtkObjectClass *) (parent_class))->destroy) (object);
}

GtkWidget *
sp_gradient_vector_selector_new (SPDocument *doc, SPGradient *gr)
{
	GtkWidget *gvs;

	g_return_val_if_fail (!doc || SP_IS_DOCUMENT (doc), NULL);
	g_return_val_if_fail (!gr || (doc != NULL), NULL);
	g_return_val_if_fail (!gr || SP_IS_GRADIENT (gr), NULL);
	g_return_val_if_fail (!gr || (SP_OBJECT_DOCUMENT (gr) == doc), NULL);

	gvs = gtk_type_new (SP_TYPE_GRADIENT_VECTOR_SELECTOR);

	if (doc) {
		sp_gradient_vector_selector_set_gradient (SP_GRADIENT_VECTOR_SELECTOR (gvs), doc, gr);
	} else {
		sp_gvs_rebuild_gui_full (SP_GRADIENT_VECTOR_SELECTOR (gvs));
	}

	return gvs;
}

void
sp_gradient_vector_selector_set_gradient (SPGradientVectorSelector *gvs, SPDocument *doc, SPGradient *gr)
{
	static gboolean suppress = FALSE;

	g_return_if_fail (gvs != NULL);
	g_return_if_fail (SP_IS_GRADIENT_VECTOR_SELECTOR (gvs));
	g_return_if_fail (!doc || SP_IS_DOCUMENT (doc));
	g_return_if_fail (!gr || (doc != NULL));
	g_return_if_fail (!gr || SP_IS_GRADIENT (gr));
	g_return_if_fail (!gr || (SP_OBJECT_DOCUMENT (gr) == doc));
	g_return_if_fail (!gr || SP_GRADIENT_HAS_STOPS (gr));

	if (doc != gvs->doc) {
		/* Disconnect signals */
		if (gvs->gr) {
  			sp_signal_disconnect_by_data (gvs->gr, gvs);
		g_signal_handlers_disconnect_matched (G_OBJECT(gvs->gr), G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, gvs);
			gvs->gr = NULL;
		}
		if (gvs->doc) {
  			sp_signal_disconnect_by_data (SP_DOCUMENT_DEFS (gvs->doc), gvs);
			gvs->doc = NULL;
		}
		/* Connect signals */
		if (doc) {
			g_signal_connect (G_OBJECT (SP_DOCUMENT_DEFS (doc)), "release", G_CALLBACK (sp_gvs_defs_release), gvs);
			g_signal_connect (G_OBJECT (SP_DOCUMENT_DEFS (doc)), "modified", G_CALLBACK (sp_gvs_defs_modified), gvs);
		}
		if (gr) {
			g_signal_connect (G_OBJECT (gr), "release", G_CALLBACK (sp_gvs_gradient_release), gvs);
		}
		gvs->doc = doc;
		gvs->gr = gr;
		sp_gvs_rebuild_gui_full (gvs);
		if (!suppress) g_signal_emit (G_OBJECT (gvs), signals[VECTOR_SET], 0, gr);
	} else if (gr != gvs->gr) {
		/* Harder case - keep document, rebuild menus and stuff */
		/* fixme: (Lauris) */
		suppress = TRUE;
		sp_gradient_vector_selector_set_gradient (gvs, NULL, NULL);
		sp_gradient_vector_selector_set_gradient (gvs, doc, gr);
		suppress = FALSE;
		g_signal_emit (G_OBJECT (gvs), signals[VECTOR_SET], 0, gr);
	}
	/* The case of setting NULL -> NULL is not very interesting */
}

SPDocument *
sp_gradient_vector_selector_get_document (SPGradientVectorSelector *gvs)
{
	g_return_val_if_fail (gvs != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT_VECTOR_SELECTOR (gvs), NULL);

	return gvs->doc;
}

SPGradient *
sp_gradient_vector_selector_get_gradient (SPGradientVectorSelector *gvs)
{
	g_return_val_if_fail (gvs != NULL, NULL);
	g_return_val_if_fail (SP_IS_GRADIENT_VECTOR_SELECTOR (gvs), NULL);

	return gvs->gr;
}

static void
sp_gvs_rebuild_gui_full (SPGradientVectorSelector *gvs)
{
	GtkWidget *m;
	GSList *gl;
	gint pos, idx;

	/* Clear old menu, if there is any */
	if (gtk_option_menu_get_menu (GTK_OPTION_MENU (gvs->menu))) {
		gtk_option_menu_remove_menu (GTK_OPTION_MENU (gvs->menu));
	}

	/* Create new menu widget */
	m = gtk_menu_new ();
	gtk_widget_show (m);

	/* Pick up all gradients with vectors */
	gl = NULL;
	if (gvs->gr) {
		const GSList *gradients, *l;
		gradients = sp_document_get_resource_list (SP_OBJECT_DOCUMENT (gvs->gr), "gradient");
		for (l = gradients; l != NULL; l = l->next) {
			if (SP_GRADIENT_HAS_STOPS (l->data)) {
				gl = g_slist_prepend (gl, l->data);
			}
		}
	}
	gl = g_slist_reverse (gl);

	pos = idx = 0;

	if (!gvs->doc) {
		GtkWidget *i;
		i = gtk_menu_item_new_with_label (_("No document selected"));
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
		gtk_widget_set_sensitive (gvs->menu, FALSE);
	} else if (!gl) {
		GtkWidget *i;
		i = gtk_menu_item_new_with_label (_("No gradients in document"));
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
		gtk_widget_set_sensitive (gvs->menu, FALSE);
	} else if (!gvs->gr) {
		GtkWidget *i;
		i = gtk_menu_item_new_with_label (_("No gradient selected"));
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
		gtk_widget_set_sensitive (gvs->menu, FALSE);
	} else {
		while (gl) {
			SPGradient *gr;
			GtkWidget *i, *w;
			gr = SP_GRADIENT (gl->data);
			gl = g_slist_remove (gl, gr);

			/* We have to know: */
			/* Gradient destroy */
			/* Gradient name change */
			i = gtk_menu_item_new ();
			gtk_widget_show (i);
			g_object_set_data (G_OBJECT (i), "gradient", gr);
			g_signal_connect (G_OBJECT (i), "activate", G_CALLBACK (sp_gvs_gradient_activate), gvs);

			w = sp_gradient_image_new (gr);
			gtk_widget_show (w);

			if (gvs->idlabel) {
				GtkWidget *hb, *l;
				hb = gtk_hbox_new (FALSE, 4);
				gtk_widget_show (hb);
				l = gtk_label_new (SP_OBJECT_ID (gr));
				gtk_widget_show (l);
				gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
				gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
				gtk_box_pack_start (GTK_BOX (hb), w, FALSE, FALSE, 0);
				w = hb;
			}

			gtk_container_add (GTK_CONTAINER (i), w);

			gtk_menu_append (GTK_MENU (m), i);

			if (gr == gvs->gr) pos = idx;
			idx += 1;
		}
		gtk_widget_set_sensitive (gvs->menu, TRUE);
	}

	gtk_option_menu_set_menu (GTK_OPTION_MENU (gvs->menu), m);
	/* Set history */
	gtk_option_menu_set_history (GTK_OPTION_MENU (gvs->menu), pos);
}

static void
sp_gvs_gradient_activate (GtkMenuItem *mi, SPGradientVectorSelector *gvs)
{
	SPGradient *gr, *norm;

	gr = g_object_get_data (G_OBJECT (mi), "gradient");
	/* Hmmm... bad things may happen here, if actual gradient is something new */
	/* Namely - menuitems etc. will be fucked up */
	/* Hmmm - probably we can just re-set it as menuitem data (Lauris) */

	g_print ("SPGradientVectorSelector: gradient %s activated\n", SP_OBJECT_ID (gr));

	norm = sp_gradient_ensure_vector_normalized (gr);
	if (norm != gr) {
		g_print ("SPGradientVectorSelector: become %s after normalization\n", SP_OBJECT_ID (norm));
		/* But be careful that we do not have gradient saved anywhere else */
		g_object_set_data (G_OBJECT (mi), "gradient", norm);
	}

	/* fixme: Really we would want to use _set_vector */
	/* Detach old */
	if (gvs->gr) {
  		sp_signal_disconnect_by_data (gvs->gr, gvs);
		gvs->gr = NULL;
	}
	/* Attach new */
	if (norm) {
		g_signal_connect (G_OBJECT (norm), "release", G_CALLBACK (sp_gvs_gradient_release), gvs);
		/* fixme: Connect 'modified'? (Lauris) */
		/* fixme: I think we do not need it (Lauris) */
		gvs->gr = norm;
	}

	g_signal_emit (G_OBJECT (gvs), signals[VECTOR_SET], 0, norm);

	if (norm != gr) {
		/* We do extra undo push here */
		/* If handler has already done it, it is just NOP */
		sp_document_done (SP_OBJECT_DOCUMENT (norm));
	}
}

static void
sp_gvs_gradient_release (SPGradient *gr, SPGradientVectorSelector *gvs)
{
	/* fixme: (Lauris) */
	/* fixme: Not sure, what to do here (Lauris) */
	gvs->gr = NULL;
}

static void
sp_gvs_defs_release (SPObject *defs, SPGradientVectorSelector *gvs)
{
	gvs->doc = NULL;
	/* Disconnect gradient as well */
	if (gvs->gr) {
  		sp_signal_disconnect_by_data (gvs->gr, gvs);
		gvs->gr = NULL;
	}

	/* Rebuild GUI */
	sp_gvs_rebuild_gui_full (gvs);
}

static void
sp_gvs_defs_modified (SPObject *defs, guint flags, SPGradientVectorSelector *gvs)
{
	/* fixme: (Lauris) */
	/* fixme: We probably have to check some flags here (Lauris) */
	/* fixme: Not exactly sure, what we have to do here (Lauris) */
}


#include "../widgets/sp-color-selector.h"

#define PAD 4

static GtkWidget *sp_gradient_vector_widget_new (SPGradient *gradient);

static void sp_gradient_vector_widget_load_gradient (GtkWidget *widget, SPGradient *gradient);
static void sp_gradient_vector_dialog_close (GtkWidget *widget, GtkWidget *dialog);
static gint sp_gradient_vector_dialog_delete (GtkWidget *widget, GdkEvent *event, GtkWidget *dialog);

static void sp_gradient_vector_widget_destroy (GtkObject *object, gpointer data);
static void sp_gradient_vector_gradient_release (SPGradient *gradient, GtkWidget *widget);
static void sp_gradient_vector_gradient_modified (SPGradient *gradient, guint flags, GtkWidget *widget);
static void sp_gradient_vector_color_dragged (SPColorSelector *csel, GtkObject *object);
static void sp_gradient_vector_color_changed (SPColorSelector *csel, GtkObject *object);

static gboolean blocked = FALSE;

static GtkWidget *
sp_gradient_vector_widget_new (SPGradient *gradient)
{
	GtkWidget *vb, *w, *f, *csel;

	g_return_val_if_fail (!gradient || SP_IS_GRADIENT (gradient), NULL);

	vb = gtk_vbox_new (FALSE, PAD);
	g_signal_connect (G_OBJECT (vb), "destroy", G_CALLBACK (sp_gradient_vector_widget_destroy), NULL);

	w = sp_gradient_image_new (gradient);
	g_object_set_data (G_OBJECT (vb), "preview", w);
	gtk_widget_show (w);
	gtk_box_pack_start (GTK_BOX (vb), w, TRUE, TRUE, PAD);

	f = gtk_frame_new (_("Start color"));
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (vb), f, TRUE, TRUE, PAD);
	csel = sp_color_selector_new ();
	g_object_set_data (G_OBJECT (vb), "start", csel);
	gtk_widget_show (csel);
	gtk_container_add (GTK_CONTAINER (f), csel);
	g_signal_connect (G_OBJECT (csel), "dragged", G_CALLBACK (sp_gradient_vector_color_dragged), vb);
	g_signal_connect (G_OBJECT (csel), "changed", G_CALLBACK (sp_gradient_vector_color_changed), vb);

	f = gtk_frame_new (_("End color"));
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (vb), f, TRUE, TRUE, PAD);
	csel = sp_color_selector_new ();
	g_object_set_data (G_OBJECT (vb), "end", csel);
	gtk_widget_show (csel);
	gtk_container_add (GTK_CONTAINER (f), csel);
	g_signal_connect (G_OBJECT (csel), "dragged", G_CALLBACK (sp_gradient_vector_color_dragged), vb);
	g_signal_connect (G_OBJECT (csel), "changed", G_CALLBACK (sp_gradient_vector_color_changed), vb);

	gtk_widget_show (vb);

	sp_gradient_vector_widget_load_gradient (vb, gradient);

	return vb;
}

GtkWidget *
sp_gradient_vector_editor_new (SPGradient *gradient)
{
	static GtkWidget *dialog = NULL;

	if (dialog == NULL) {
		GtkWidget *w;
		dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (dialog), _("Gradient vector"));
		gtk_container_set_border_width (GTK_CONTAINER (dialog), PAD);
		g_signal_connect (G_OBJECT (dialog), "delete_event", G_CALLBACK (sp_gradient_vector_dialog_delete), dialog);
		w = sp_gradient_vector_widget_new (gradient);
		g_object_set_data (G_OBJECT (dialog), "gradient-vector-widget", w);
		/* Connect signals */
		gtk_widget_show (w);
		gtk_container_add (GTK_CONTAINER (dialog), w);
	} else {
		GtkWidget *w;
		w = g_object_get_data (G_OBJECT (dialog), "gradient-vector-widget");
		sp_gradient_vector_widget_load_gradient (w, gradient);
	}

	return dialog;
}

static void
sp_gradient_vector_widget_load_gradient (GtkWidget *widget, SPGradient *gradient)
{
	SPGradient *old;
	GtkWidget *w;
	guint32 cs, ce;

	old = g_object_get_data (G_OBJECT (widget), "gradient");
	if (old != gradient) {
		if (old) {
  			sp_signal_disconnect_by_data (old, widget);
		}
		if (gradient) {
			g_signal_connect (G_OBJECT (gradient), "release", G_CALLBACK (sp_gradient_vector_gradient_release), widget);
			g_signal_connect (G_OBJECT (gradient), "modified", G_CALLBACK (sp_gradient_vector_gradient_modified), widget);
		}
	}

	g_object_set_data (G_OBJECT (widget), "gradient", gradient);

	if (gradient) {
		sp_gradient_ensure_vector (gradient);
		cs = sp_color_get_rgba32_falpha (&gradient->vector->stops[0].color, gradient->vector->stops[0].opacity);
		ce = sp_color_get_rgba32_falpha (&gradient->vector->stops[1].color, gradient->vector->stops[1].opacity);

		/* Set color selector values */
		w = g_object_get_data (G_OBJECT (widget), "start");
		sp_color_selector_set_any_rgba32 (SP_COLOR_SELECTOR (w), cs);
		w = g_object_get_data (G_OBJECT (widget), "end");
		sp_color_selector_set_any_rgba32 (SP_COLOR_SELECTOR (w), ce);

		/* Fixme: Sensitivity */
	}

	/* Fill preview */
	w = g_object_get_data (G_OBJECT (widget), "preview");
	sp_gradient_image_set_gradient (SP_GRADIENT_IMAGE (w), gradient);
}

static void
sp_gradient_vector_dialog_close (GtkWidget *widget, GtkWidget *dialog)
{
	gtk_widget_hide (dialog);
}

static gint
sp_gradient_vector_dialog_delete (GtkWidget *widget, GdkEvent *event, GtkWidget *dialog)
{
	sp_gradient_vector_dialog_close (widget, dialog);

	return TRUE;
}

/* Widget destroy handler */

static void
sp_gradient_vector_widget_destroy (GtkObject *object, gpointer data)
{
	GObject *gradient;

	gradient = g_object_get_data (G_OBJECT (object), "gradient");

	if (gradient) {
		/* Remove signals connected to us */
		/* fixme: may use _connect_while_alive as well */
  		sp_signal_disconnect_by_data (gradient, object);
	}
}

static void
sp_gradient_vector_gradient_release (SPGradient *gradient, GtkWidget *widget)
{
	sp_gradient_vector_widget_load_gradient (widget, NULL);
}

static void
sp_gradient_vector_gradient_modified (SPGradient *gradient, guint flags, GtkWidget *widget)
{
	if (!blocked) {
		blocked = TRUE;
		sp_gradient_vector_widget_load_gradient (widget, gradient);
		blocked = FALSE;
	}
}

static void
sp_gradient_vector_color_dragged (SPColorSelector *csel, GtkObject *object)
{
	SPGradient *gradient, *ngr;
	SPGradientVector *vector;
	gfloat c[4];

	if (blocked) return;

	gradient = g_object_get_data (G_OBJECT (object), "gradient");
	if (!gradient) return;

	blocked = TRUE;

	ngr = sp_gradient_ensure_vector_normalized (gradient);
	if (ngr != gradient) {
		/* Our master gradient has changed */
		sp_gradient_vector_widget_load_gradient (GTK_WIDGET (object), ngr);
	}

	sp_gradient_ensure_vector (ngr);

	vector = alloca (sizeof (SPGradientVector) + sizeof (SPGradientStop));
	vector->nstops = 2;
	vector->start = ngr->vector->start;
	vector->end = ngr->vector->end;

	csel = g_object_get_data (G_OBJECT (object), "start");
	sp_color_selector_get_rgba_floatv (csel, c);
	vector->stops[0].offset = 0.0;
	sp_color_set_rgb_float (&vector->stops[0].color, c[0], c[1], c[2]);
	vector->stops[0].opacity = c[3];
	csel = g_object_get_data (G_OBJECT (object), "end");
	sp_color_selector_get_rgba_floatv (csel, c);
	vector->stops[1].offset = 1.0;
	sp_color_set_rgb_float (&vector->stops[1].color, c[0], c[1], c[2]);
	vector->stops[1].opacity = c[3];

	sp_gradient_set_vector (ngr, vector);

	blocked = FALSE;
}

static void
sp_gradient_vector_color_changed (SPColorSelector *csel, GtkObject *object)
{
	SPGradient *gradient, *ngr;
	gdouble start, end;
	SPObject *child;
	guint32 color;
	gchar c[256];

	if (blocked) return;

	gradient = g_object_get_data (G_OBJECT (object), "gradient");
	if (!gradient) return;

	blocked = TRUE;

	ngr = sp_gradient_ensure_vector_normalized (gradient);
	if (ngr != gradient) {
		/* Our master gradient has changed */
		sp_gradient_vector_widget_load_gradient (GTK_WIDGET (object), ngr);
	}

	sp_gradient_ensure_vector (ngr);

	start = ngr->vector->start;
	end = ngr->vector->end;

	/* Set start parameters */
	/* We rely on normalized vector, i.e. stops HAVE to exist */
	for (child = ngr->stops; child != NULL; child = child->next) {
		if (SP_IS_STOP (child)) break;
	}
	g_return_if_fail (child != NULL);

	csel = g_object_get_data (G_OBJECT (object), "start");
	color = sp_color_selector_get_rgba32 (csel);

	sp_repr_set_double_attribute (SP_OBJECT_REPR (child), "offset", start);
	g_snprintf (c, 256, "stop-color:#%06x;stop-opacity:%g;", color >> 8, (gdouble) (color & 0xff) / 255.0);
	sp_repr_set_attr (SP_OBJECT_REPR (child), "style", c);

	for (child = child->next; child != NULL; child = child->next) {
		if (SP_IS_STOP (child)) break;
	}
	g_return_if_fail (child != NULL);

	csel = g_object_get_data (G_OBJECT (object), "end");
	color = sp_color_selector_get_rgba32 (csel);

	sp_repr_set_double_attribute (SP_OBJECT_REPR (child), "offset", end);
	g_snprintf (c, 256, "stop-color:#%06x;stop-opacity:%g;", color >> 8, (gdouble) (color & 0xff) / 255.0);
	sp_repr_set_attr (SP_OBJECT_REPR (child), "style", c);

	/* Remove other stops */
	while (child->next) {
		if (SP_IS_STOP (child->next)) {
			sp_repr_remove_child (SP_OBJECT_REPR (ngr), SP_OBJECT_REPR (child->next));
		} else {
			child = child->next;
		}
	}

	sp_document_done (SP_OBJECT_DOCUMENT (ngr));

	blocked = FALSE;
}

