#define __SP_GRADIENT_VECTOR_C__

/*
 * Gradient vector selection widget
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 * Copyright (C) 2004 Monash University
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <config.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <gtk/gtk.h>
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
#include "../inkscape.h"
#include "../document-private.h"
#include "../sp-gradient.h"
#include "../gradient-chemistry.h"
#include "gradient-vector.h"
#include "../helper/window.h"

#include "../dialogs/dialog-events.h"
#include "../prefs-utils.h"
#include "../verbs.h"
#include "../interface.h"
#include "../svg/stringstream.h"

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

static GtkWidget *dlg = NULL;
static win_data wd;
static gint x = -1000, y = -1000, w = 0, h = 0; // impossible original values to make sure they are read from prefs
static gchar const *prefs_path = "dialogs.gradienteditor";

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

	parent_class = (GtkVBoxClass*)gtk_type_class (GTK_TYPE_VBOX);

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

	gvs = (GtkWidget*)gtk_type_new (SP_TYPE_GRADIENT_VECTOR_SELECTOR);

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

	gr = (SPGradient*)g_object_get_data (G_OBJECT (mi), "gradient");
	/* Hmmm... bad things may happen here, if actual gradient is something new */
	/* Namely - menuitems etc. will be fucked up */
	/* Hmmm - probably we can just re-set it as menuitem data (Lauris) */

	//g_print ("SPGradientVectorSelector: gradient %s activated\n", SP_OBJECT_ID (gr));

	norm = sp_gradient_ensure_vector_normalized (gr);
	if (norm != gr) {
		//g_print ("SPGradientVectorSelector: become %s after normalization\n", SP_OBJECT_ID (norm));
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

/*##################################################################
###                 Vector Editing Widget
##################################################################*/

#include "../widgets/sp-color-selector.h"
#include "../widgets/sp-color-notebook.h"
#include "../widgets/sp-color-preview.h"
#include "../widgets/widget-sizes.h"
#include "../desktop-handles.h"
#include "../selection.h"
#include "../xml/repr-private.h"
#include "../svg/svg.h"


#define PAD 4

static GtkWidget *sp_gradient_vector_widget_new (SPGradient *gradient);

static void sp_gradient_vector_widget_load_gradient (GtkWidget *widget, SPGradient *gradient);
static gint sp_gradient_vector_dialog_delete (GtkWidget *widget, GdkEvent *event, GtkWidget *dialog);
static void sp_gradient_vector_dialog_destroy (GtkObject *object, gpointer data);

static void sp_gradient_vector_widget_destroy (GtkObject *object, gpointer data);
static void sp_gradient_vector_gradient_release (SPGradient *gradient, GtkWidget *widget);
static void sp_gradient_vector_gradient_modified (SPGradient *gradient, guint flags, GtkWidget *widget);
static void sp_gradient_vector_color_dragged (SPColorSelector *csel, GtkObject *object);
static void sp_gradient_vector_color_changed (SPColorSelector *csel, GtkObject *object);
static void update_stop_list( GtkWidget *mnu, SPGradient *gradient, SPStop *new_stop);

static gboolean blocked = FALSE;

static void grad_edit_dia_stopremoved (SPRepr *repr, SPRepr *child, SPRepr *ref, gpointer data)
{
   GtkWidget *vb = GTK_WIDGET(data);
   GtkWidget *mnu = (GtkWidget *)g_object_get_data (G_OBJECT(vb), "stopmenu");
   SPGradient *gradient = (SPGradient *)g_object_get_data (G_OBJECT(vb), "gradient");
   update_stop_list (mnu, gradient, NULL);
}

static SPReprEventVector grad_edit_dia_repr_events =
{
    NULL, /* destroy */
    NULL, /* add_child */
    NULL, /* child_added */
    NULL, /* remove_child */
    grad_edit_dia_stopremoved, /* child_removed */
    NULL, /* change_attr */
    NULL, /* attr_changed*/
    NULL, /* change_list */
    NULL, /* content_changed */
    NULL, /* change_order */
    NULL  /* order_changed */
};

static void
verify_grad(SPGradient *gradient)
{
	int i = 0;
	SPStop *stop = NULL;
	for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
		if (SP_IS_STOP (ochild)) {
			i++;
			stop = SP_STOP(ochild);
		}
	}
	if (i < 1) {
		gchar c[64];
		sp_svg_write_color (c, 64, 0x00000000);

		Inkscape::SVGOStringStream os;
		os << "stop-color:" << c << ";stop-opacity:" << 1.0 << ";";

		SPRepr *child;

		child = sp_repr_new ("stop");
		sp_repr_set_double (child, "offset", 0.0);
		sp_repr_set_attr (child, "style", os.str().c_str());
		sp_repr_add_child (SP_OBJECT_REPR (gradient), child, NULL);

		child = sp_repr_new ("stop");
		sp_repr_set_double (child, "offset", 1.0);
		sp_repr_set_attr (child, "style", os.str().c_str());
		sp_repr_add_child (SP_OBJECT_REPR (gradient), child, NULL);
	}
	if (i < 2) {
		sp_repr_set_double (SP_OBJECT_REPR(stop), "offset", 0.0);
		SPRepr *child = sp_repr_duplicate(SP_OBJECT_REPR(stop));
		sp_repr_set_double (child, "offset", 1.0);
		sp_repr_add_child (SP_OBJECT_REPR(gradient), child, SP_OBJECT_REPR (stop));
	}
}

static SPStop*
sp_prev_stop(SPStop *stop, SPGradient *gradient)
{
	if (sp_repr_attr(SP_OBJECT_REPR(sp_object_first_child(SP_OBJECT(gradient))),"id") == sp_repr_attr(SP_OBJECT_REPR(SP_OBJECT(stop)),"id")) 
		return NULL;
	for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
		if (SP_IS_STOP (ochild)) {
			if (sp_repr_attr(SP_OBJECT_REPR(SP_OBJECT_NEXT(ochild)),"id") == sp_repr_attr(SP_OBJECT_REPR(SP_OBJECT(stop)),"id")) 
				return SP_STOP(ochild);
		}
	}
	return NULL;
}

static SPStop*
sp_next_stop(SPStop *stop)
{
  SPObject *ochild = SP_OBJECT_NEXT(stop);
  if (ochild != NULL && SP_IS_STOP (ochild)) 
		return SP_STOP(ochild);
  else 
		return NULL;
}


static void
update_stop_list( GtkWidget *mnu, SPGradient *gradient, SPStop *new_stop)
{

	if (!SP_IS_GRADIENT (gradient))
		return;

	/* Clear old menu, if there is any */
	if (gtk_option_menu_get_menu (GTK_OPTION_MENU (mnu))) {
		gtk_option_menu_remove_menu (GTK_OPTION_MENU (mnu));
	}

	/* Create new menu widget */
	GtkWidget *m = gtk_menu_new ();
	gtk_widget_show (m);
	GSList *sl = NULL;
	if (gradient->has_stops) {
		for ( SPObject *ochild = sp_object_first_child (SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
			if (SP_IS_STOP (ochild)) {
				sl = g_slist_append (sl, ochild);
			}
		}
	}
	if (!sl) {
		GtkWidget *i = gtk_menu_item_new_with_label (_("No stops in gradient"));
		gtk_widget_show (i);
		gtk_menu_append (GTK_MENU (m), i);
		gtk_widget_set_sensitive (mnu, FALSE);
	} else {
     
		for (; sl != NULL; sl = sl->next){
			SPStop *stop;
			GtkWidget *i;
			if (SP_IS_STOP(sl->data)){
				stop = SP_STOP (sl->data);
				i = gtk_menu_item_new ();
				gtk_widget_show (i);
				g_object_set_data (G_OBJECT (i), "stop", stop);
				GtkWidget *hb = gtk_hbox_new (FALSE, 4);
				GtkWidget *cpv = sp_color_preview_new (sp_color_get_rgba32_falpha (&stop->color, stop->opacity));
				gtk_widget_show (cpv);
				gtk_container_add ( GTK_CONTAINER (hb), cpv );
				g_object_set_data ( G_OBJECT (i), "preview", cpv );
				SPRepr *repr = SP_OBJECT_REPR((SPItem *) sl->data);
				GtkWidget *l = gtk_label_new (sp_repr_attr(repr,"id"));
				gtk_widget_show (l);
				gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
				gtk_box_pack_start (GTK_BOX (hb), l, TRUE, TRUE, 0);
				gtk_widget_show (hb);
				gtk_container_add (GTK_CONTAINER (i), hb);
				gtk_menu_append (GTK_MENU (m), i);
			}
		}

		gtk_widget_set_sensitive (mnu, TRUE);
	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU (mnu), m);
	/* Set history */
	if (new_stop == NULL) gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), 0);
	else {
		int i = 0;
		for ( SPObject *ochild = sp_object_first_child(SP_OBJECT(gradient)) ; ochild != NULL ; ochild = SP_OBJECT_NEXT(ochild) ) {
			if (SP_IS_STOP (ochild)) {
				if (sp_repr_attr(SP_OBJECT_REPR(ochild),"id") == sp_repr_attr(SP_OBJECT_REPR(SP_OBJECT(new_stop)),"id")) gtk_option_menu_set_history (GTK_OPTION_MENU (mnu), i);
			}
			i++;
		}
	}
}


/*user selected existing stop from list*/
static void
sp_grad_edit_select (GtkOptionMenu *mnu,  GtkWidget *tbl)
{
	SPGradient *gradient = (SPGradient *)g_object_get_data (G_OBJECT(tbl), "gradient");

	if (!g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop")) return;
	SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));
	SPColorSelector *csel = (SPColorSelector*)g_object_get_data (G_OBJECT (tbl), "stop");
	guint32 c = sp_color_get_rgba32_falpha (&stop->color, stop->opacity);
	csel->base->setAlpha(SP_RGBA32_A_F (c));
	SPColor color;
	sp_color_set_rgb_float (&color, SP_RGBA32_R_F (c), SP_RGBA32_G_F (c), SP_RGBA32_B_F (c));
	// set its color, from the stored array
	csel->base->setColor( color );
	GtkWidget *offspin = GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "offspn"));
	GtkWidget *offslide =GTK_WIDGET (g_object_get_data (G_OBJECT (tbl), "offslide"));
	if (stop->offset>0 && stop->offset<1) {
		gtk_widget_set_sensitive (offslide, TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (offspin), TRUE);
	} else {
		gtk_widget_set_sensitive (offslide, FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (offspin), FALSE);
	}
	GtkAdjustment *adj = (GtkAdjustment*)gtk_object_get_data (GTK_OBJECT (tbl), "offset");
	SPStop *prev = NULL;
	prev = sp_prev_stop(stop, gradient);
	if (prev != NULL )  adj->lower = prev->offset+.01;
	else adj->lower = 0;

	SPStop *next = NULL;
	next = sp_next_stop(stop);
	if (next != NULL )  adj->upper = next->offset-.01;
	else adj->upper = 1.0;

	sp_repr_set_double (SP_OBJECT_REPR (stop), "offset", stop->offset);
	gtk_adjustment_set_value (adj, stop->offset);

	gtk_adjustment_changed (adj);
}




static void
offadjustmentChanged( GtkAdjustment *adjustment, GtkWidget *vb)
{
    GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(vb), "stopmenu");
    if (!g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop")) return;
    SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));

    stop->offset = adjustment->value;
    sp_repr_set_double (SP_OBJECT_REPR (stop), "offset", stop->offset);

    sp_document_done (SP_OBJECT_DOCUMENT (stop));
}

guint32
sp_average_color (guint32 c1, guint32 c2, gdouble p = 0.5)
{
	guint32 r = (guint32) (SP_RGBA32_R_U (c1) * p + SP_RGBA32_R_U (c2) * (1 - p));
	guint32 g = (guint32) (SP_RGBA32_G_U (c1) * p + SP_RGBA32_G_U (c2) * (1 - p));
	guint32 b = (guint32) (SP_RGBA32_B_U (c1) * p + SP_RGBA32_B_U (c2) * (1 - p));
	guint32 a = (guint32) (SP_RGBA32_A_U (c1) * p + SP_RGBA32_A_U (c2) * (1 - p));

	return SP_RGBA32_U_COMPOSE (r, g, b, a);
}


static void
sp_grd_ed_add_stop (GtkWidget *widget,  GtkWidget *vb)
{
	SPGradient *gradient = (SPGradient *) g_object_get_data (G_OBJECT(vb), "gradient");
	verify_grad (gradient);
	GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(vb), "stopmenu");

	SPStop *stop = (SPStop *) g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop");

	if (stop == NULL) 
		return;

	SPRepr *new_stop_repr = NULL;

	SPStop *next = sp_next_stop (stop);

	if (next == NULL) {
		SPStop *prev = sp_prev_stop (stop, gradient);
		if (prev != NULL) {
			next = stop;
			stop = prev;
		}
	}

	if (next != NULL) {
		new_stop_repr = sp_repr_duplicate(SP_OBJECT_REPR(stop));
		sp_repr_add_child (SP_OBJECT_REPR(gradient), new_stop_repr, SP_OBJECT_REPR(stop));
	} else {
		next = stop;
		new_stop_repr = sp_repr_duplicate(SP_OBJECT_REPR(sp_prev_stop(stop, gradient)));
		sp_repr_add_child (SP_OBJECT_REPR(gradient), new_stop_repr, SP_OBJECT_REPR(sp_prev_stop(stop, gradient)));
	}

	SPStop *newstop = (SPStop *) SP_OBJECT_DOCUMENT(gradient)->getObjectByRepr(new_stop_repr);
   
	newstop->offset = (stop->offset + next->offset) * 0.5 ;

	guint32 c1 = sp_color_get_rgba32_falpha (&stop->color, stop->opacity);
	guint32 c2 = sp_color_get_rgba32_falpha (&next->color, next->opacity);
	guint32 cnew = sp_average_color (c1, c2);

	Inkscape::SVGOStringStream os;
	gchar c[64];
	sp_svg_write_color (c, 64, cnew);
	gdouble opacity = (gdouble) SP_RGBA32_A_F (cnew);
	os << "stop-color:" << c << ";stop-opacity:" << opacity <<";";
	sp_repr_set_attr (SP_OBJECT_REPR (newstop), "style", os.str().c_str());

	sp_gradient_vector_widget_load_gradient (vb, gradient);
	sp_repr_unref (new_stop_repr);
	update_stop_list(GTK_WIDGET(mnu), gradient, newstop);
	GtkWidget *offspin = GTK_WIDGET (g_object_get_data (G_OBJECT (vb), "offspn"));
	GtkWidget *offslide =GTK_WIDGET (g_object_get_data (G_OBJECT (vb), "offslide"));
	gtk_widget_set_sensitive (offslide, TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (offspin), TRUE);
	sp_document_done (SP_OBJECT_DOCUMENT (gradient));
}

static void
sp_grd_ed_del_stop (GtkWidget *widget,  GtkWidget *vb)
{
    SPGradient *gradient = (SPGradient *)g_object_get_data (G_OBJECT(vb), "gradient");

    GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(vb), "stopmenu");
    if (!g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop")) return;
    SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));
    if (stop->offset>0 && stop->offset<1) {
        sp_repr_remove_child (SP_OBJECT_REPR(gradient), SP_OBJECT_REPR(stop));
        sp_gradient_vector_widget_load_gradient (vb, gradient);
        update_stop_list(GTK_WIDGET(mnu), gradient, NULL);
        sp_document_done (SP_OBJECT_DOCUMENT (gradient));
    }

}

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

	gtk_object_set_data (GTK_OBJECT (vb), "gradient", gradient);
	sp_repr_add_listener (SP_OBJECT_REPR(gradient), &grad_edit_dia_repr_events, vb);
	GtkTooltips *tt = gtk_tooltips_new ();

	/* Stop list */
	GtkWidget *mnu = gtk_option_menu_new ();
	/* Create new menu widget */
	update_stop_list (GTK_WIDGET(mnu), gradient, NULL);
	gtk_signal_connect (GTK_OBJECT (mnu), "changed", GTK_SIGNAL_FUNC (sp_grad_edit_select), vb);
	gtk_widget_show (mnu);
	gtk_object_set_data (GTK_OBJECT (vb), "stopmenu", mnu);
	gtk_box_pack_start (GTK_BOX (vb), mnu, FALSE, FALSE, 0);

	/* Add and Remove buttons */
	GtkWidget *hb = gtk_hbox_new (FALSE, 1);
	GtkWidget *b = gtk_button_new_with_label (_("Add stop"));
	gtk_widget_show (b);
	gtk_container_add (GTK_CONTAINER (hb), b);
	gtk_tooltips_set_tip (tt, b, _("Add another control stop to gradient"), NULL);
	gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_grd_ed_add_stop), vb);
	b = gtk_button_new_with_label (_("Delete stop"));
	gtk_widget_show (b);
	gtk_container_add (GTK_CONTAINER (hb), b);
	gtk_tooltips_set_tip (tt, b, _("Delete current control stop from gradient"), NULL);
	gtk_signal_connect (GTK_OBJECT (b), "clicked", GTK_SIGNAL_FUNC (sp_grd_ed_del_stop), vb);

	gtk_widget_show (hb);
	gtk_box_pack_start (GTK_BOX (vb),hb, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);


	/*  Offset Slider and stuff   */
	hb = gtk_hbox_new (FALSE, 0);

	/* Label */
	GtkWidget *l = gtk_label_new (_("Offset:"));
	gtk_misc_set_alignment (GTK_MISC (l), 1.0, 0.5);
	gtk_box_pack_start (GTK_BOX (hb),l, FALSE, FALSE, AUX_BETWEEN_BUTTON_GROUPS);
	gtk_widget_show (l);

	/* Adjustment */
	GtkAdjustment *Offset_adj = NULL;
	Offset_adj= (GtkAdjustment *) gtk_adjustment_new (0.0, 0.0, 1.0, 0.01, 0.01, 0.0);
	gtk_object_set_data (GTK_OBJECT (vb), "offset", Offset_adj);
	GtkMenu *m = GTK_MENU(gtk_option_menu_get_menu (GTK_OPTION_MENU(mnu)));
	SPStop *stop = SP_STOP (g_object_get_data (G_OBJECT (gtk_menu_get_active (m)), "stop"));
	gtk_adjustment_set_value (Offset_adj, stop->offset);

	/* Slider */
	GtkWidget *slider = gtk_hscale_new(Offset_adj);
	gtk_scale_set_draw_value( GTK_SCALE(slider), FALSE );
	gtk_widget_show (slider);
	gtk_box_pack_start (GTK_BOX (hb),slider, TRUE, TRUE, AUX_BETWEEN_BUTTON_GROUPS);
	gtk_object_set_data (GTK_OBJECT (vb), "offslide", slider);

	/* Spinbutton */
	GtkWidget *sbtn = gtk_spin_button_new (GTK_ADJUSTMENT (Offset_adj), 0.01, 2);
	sp_dialog_defocus_on_enter (sbtn);
	gtk_widget_show (sbtn);
	gtk_box_pack_start (GTK_BOX (hb),sbtn, FALSE, TRUE, AUX_BETWEEN_BUTTON_GROUPS);
	gtk_object_set_data (GTK_OBJECT (vb), "offspn", sbtn);

	if (stop->offset>0 && stop->offset<1) {
		gtk_widget_set_sensitive (slider, TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (sbtn), TRUE);
	} else {
		gtk_widget_set_sensitive (slider, FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (sbtn), FALSE);
	}


	/* Signals */
	gtk_signal_connect (GTK_OBJECT (Offset_adj), "value_changed",
											GTK_SIGNAL_FUNC (offadjustmentChanged), vb);

	// gtk_signal_connect (GTK_OBJECT (slider), "changed",  GTK_SIGNAL_FUNC (offsliderChanged), vb);
	gtk_widget_show (hb);
	gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, PAD);

	f = gtk_frame_new (_("Stop Color"));
	gtk_widget_show (f);
	gtk_box_pack_start (GTK_BOX (vb), f, TRUE, TRUE, PAD);
	csel = (GtkWidget*)sp_color_selector_new (SP_TYPE_COLOR_NOTEBOOK, SP_COLORSPACE_TYPE_NONE);
	g_object_set_data (G_OBJECT (vb), "stop", csel);
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
	GtkWidget *wid;

	if (dlg == NULL) {

		dlg = sp_window_new (_("Gradient editor"), TRUE);
		if (x == -1000 || y == -1000) {
			x = prefs_get_int_attribute (prefs_path, "x", 0);
			y = prefs_get_int_attribute (prefs_path, "y", 0);
		}
		if (w ==0 || h == 0) {
			w = prefs_get_int_attribute (prefs_path, "w", 0);
			h = prefs_get_int_attribute (prefs_path, "h", 0);
		}
		if (x != 0 || y != 0) 
			gtk_window_move ((GtkWindow *) dlg, x, y);
		else
			gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
		if (w && h) gtk_window_resize ((GtkWindow *) dlg, w, h);
		sp_transientize (dlg);
		wd.win = dlg;
		wd.stop = 0;
		g_signal_connect (G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd);
		gtk_signal_connect (GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
		gtk_signal_connect (GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_gradient_vector_dialog_destroy), dlg);
		gtk_signal_connect (GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_gradient_vector_dialog_delete), dlg);
		g_signal_connect (G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_gradient_vector_dialog_delete), dlg);
		g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg );
		g_signal_connect ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg );

		gtk_container_set_border_width (GTK_CONTAINER (dlg), PAD);

		wid = (GtkWidget*)sp_gradient_vector_widget_new (gradient);
		g_object_set_data (G_OBJECT (dlg), "gradient-vector-widget", wid);
		/* Connect signals */
		gtk_widget_show (wid);
		gtk_container_add (GTK_CONTAINER (dlg), wid);
	} else {
		// FIXME: temp fix for 0.38
		// Simply load_gradient into the editor does not work for multi-stop gradients, 
		// as the stop list and other widgets are in a wrong state and crash readily. 
		// Instead we just delete the window (by sending the delete signal)
		// and call sp_gradient_vector_editor_new again, so it creates the window anew.

		GdkEventAny event;
		GtkWidget *widget = (GtkWidget *) dlg;
		event.type = GDK_DELETE;
		event.window = widget->window;
		event.send_event = TRUE;
		g_object_ref (G_OBJECT (event.window));
		gtk_main_do_event ((GdkEvent*)&event);
		g_object_unref (G_OBJECT (event.window));

		g_assert (dlg == NULL);
		sp_gradient_vector_editor_new (gradient);

		// The old code which crashes when you "add" (i.e. copy) a three-stop gradient, "edit" it, and add a stop
		//		gtk_window_present ((GtkWindow *) dlg);
		//		wid = (GtkWidget*)g_object_get_data (G_OBJECT (dlg), "gradient-vector-widget");
		//		sp_gradient_vector_widget_load_gradient (wid, gradient);
	}

	return dlg;
}

static void
sp_gradient_vector_widget_load_gradient (GtkWidget *widget, SPGradient *gradient)
{
	SPGradient *old;

	old = (SPGradient*)g_object_get_data (G_OBJECT (widget), "gradient");
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

		// So far we can only handle 2 stops, but eventually we need to support arbitrary number.
		// Remember _now_ all stop colors of the given gradient in an array, so that they're not botched
		// by the colorselectors during setting (fixes bug 902319)
		GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(widget), "stopmenu");
		SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));
		guint32 c = sp_color_get_rgba32_falpha (&stop->color, stop->opacity);


		/// get the color selector by its id
			SPColorSelector *csel = SP_COLOR_SELECTOR(g_object_get_data (G_OBJECT (widget), "stop"));

			// set its alpha, from the stored array
			csel->base->setAlpha(SP_RGBA32_A_F (c));
			SPColor color;
			sp_color_set_rgb_float (&color, SP_RGBA32_R_F (c), SP_RGBA32_G_F (c), SP_RGBA32_B_F (c));
			// set its color, from the stored array
			csel->base->setColor( color );

			/* Fixme: Sensitivity */
	}

	/* Fill preview */
	GtkWidget *w = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(widget), "preview"));
	sp_gradient_image_set_gradient (SP_GRADIENT_IMAGE (w), gradient);

	GtkWidget *mnu = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(widget), "stopmenu"));
	update_stop_list (GTK_WIDGET(mnu), gradient, NULL);
}

static void
sp_gradient_vector_dialog_destroy (GtkObject *object, gpointer data)
{
	sp_signal_disconnect_by_data (INKSCAPE, dlg);
	wd.win = dlg = NULL;
	wd.stop = 0;
}

static gboolean
sp_gradient_vector_dialog_delete (GtkWidget *widget, GdkEvent *event, GtkWidget *dialog)
{
	gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
	gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

	prefs_set_int_attribute (prefs_path, "x", x);
	prefs_set_int_attribute (prefs_path, "y", y);
	prefs_set_int_attribute (prefs_path, "w", w);
	prefs_set_int_attribute (prefs_path, "h", h);

	return FALSE; // which means, go ahead and destroy it
}

/* Widget destroy handler */

static void
sp_gradient_vector_widget_destroy (GtkObject *object, gpointer data)
{
	GObject *gradient;

	gradient = (GObject*)g_object_get_data (G_OBJECT (object), "gradient");

	if (gradient) {
		/* Remove signals connected to us */
		/* fixme: may use _connect_while_alive as well */
		sp_signal_disconnect_by_data (gradient, object);
		sp_repr_remove_listener_by_data (SP_OBJECT_REPR(gradient), object);
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

static void sp_gradient_vector_color_dragged(SPColorSelector *csel, GtkObject *object)
{
	SPGradient *gradient, *ngr;

	if (blocked) return;

	gradient = (SPGradient*)g_object_get_data (G_OBJECT (object), "gradient");
	if (!gradient) return;

	blocked = TRUE;

	ngr = sp_gradient_ensure_vector_normalized (gradient);
	if (ngr != gradient) {
		/* Our master gradient has changed */
		sp_gradient_vector_widget_load_gradient (GTK_WIDGET (object), ngr);
	}

	sp_gradient_ensure_vector (ngr);

    GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(object), "stopmenu");
    SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));


    csel->base->getColorAlpha(stop->color, &stop->opacity);

	blocked = FALSE;
    SPColorPreview *cpv = (SPColorPreview *)g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "preview");
    sp_color_preview_set_rgba32 (cpv, sp_color_get_rgba32_falpha (&stop->color, stop->opacity));

}

static void
sp_gradient_vector_color_changed (SPColorSelector *csel, GtkObject *object)
{
	SPGradient *gradient, *ngr;
	gdouble start, end;
	SPObject *child;
	SPColor color;
	float alpha;
	guint32 rgb;

	if (blocked) return;

	gradient = (SPGradient*)g_object_get_data (G_OBJECT (object), "gradient");
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
	for ( child = sp_object_first_child(SP_OBJECT(ngr)) ; child != NULL ; child = SP_OBJECT_NEXT(child) ) {
		if (SP_IS_STOP (child)) break;
	}
	g_return_if_fail (child != NULL);

	GtkOptionMenu *mnu = (GtkOptionMenu *)g_object_get_data (G_OBJECT(object), "stopmenu");
	SPStop *stop = SP_STOP(g_object_get_data (G_OBJECT(gtk_menu_get_active (GTK_MENU(gtk_option_menu_get_menu (mnu)))), "stop"));

	csel = (SPColorSelector*)g_object_get_data (G_OBJECT (object), "stop");
	csel->base->getColorAlpha( color, &alpha );
	rgb = sp_color_get_rgba32_ualpha (&color, 0x00);

	sp_repr_set_double (SP_OBJECT_REPR (stop), "offset", stop->offset);
	Inkscape::SVGOStringStream os;
	gchar c[64];
	sp_svg_write_color (c, 64, rgb);
	os << "stop-color:" << c << ";stop-opacity:" << (gdouble) alpha <<";";
	sp_repr_set_attr (SP_OBJECT_REPR (stop), "style", os.str().c_str());
		//	g_snprintf (c, 256, "stop-color:#%06x;stop-opacity:%g;", rgb >> 8, (gdouble) alpha);
		//sp_repr_set_attr (SP_OBJECT_REPR (stop), "style", c);

	sp_document_done (SP_OBJECT_DOCUMENT (ngr));

	blocked = FALSE;
}

